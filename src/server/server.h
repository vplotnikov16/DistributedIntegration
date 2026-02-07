#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <atomic>
#include <thread>
#include "client_manager.h"
#include "task_distributor.h"
#include "result_aggregator.h"
#include "input_handler.h"

using boost::asio::ip::tcp;

/**
 * @file server.h
 * @brief Основной модуль сервера
 */

/**
 * @struct IntegrationParameters
 * @brief Параметры задачи интегрирования
 */
struct IntegrationParameters
{
    // Нижний предел интегрирования
    double lower_limit;
    // Верхний предел интегрирования
    double upper_limit;
    // Шаг интегрирования
    double step;

    /**
     * @brief Проверка корректности параметров
     */
    bool is_valid() const
    {
        // ограничения для интегрирования 1/ln(x)
        bool result = true;

        // Начало не может быть больше конца, шаг должен быть положительным
        // и быть меньше длины интегрируемого интервала
        result &= !(lower_limit >= upper_limit || step <= 0.0 || step >= (upper_limit - lower_limit));

        // Нижний предел должен быть положительным
        result &= !(lower_limit <= 0.0);

        // Интервал не должен содержать x = 1
        result &= !(lower_limit < 1.0 && upper_limit > 1.0);
        result &= !(std::abs(lower_limit - 1.0) < 1e-10 || std::abs(upper_limit - 1.0) < 1e-10);

        return result;
    }
};

/**
 * @class Server
 * @brief Главный класс сервера
 *
 * Координирует работу всех компонентов: прием клиентов, распределение задач,
 * сбор результатов и управление жизненным циклом
 */
class Server
{
public:
    /**
     * @brief Конструктор
     * @param port Порт для прослушивания подключений
     */
    explicit Server(uint16_t port);

    ~Server();

    // Запрет копирования
    Server(const Server &) = delete;
    Server &operator=(const Server &) = delete;

    /**
     * @brief Запускает сервер
     *
     * @param params Параметры интегрирования
     *
     * Выполняет полный цикл работы сервера:
     * 1. Запрос параметров у пользователя
     * 2. Ожидание подключения клиентов
     * 3. Ожидание команды START
     * 4. Распределение задач
     * 5. Сбор результатов
     * 6. Вывод итогового результата
     * 7. Завершение работы
     */
    void run(const IntegrationParameters &params);

    /**
     * @brief Остановка сервера
     */
    void stop();

private:
    /**
     * @brief Запускает прием подключений клиентов в отдельном потоке
     */
    void start_accepting_clients();

    /**
     * @brief Функция потока приёма подключений
     */
    void accept_thread_func();

    /**
     * @brief Обработка подключения одного клиента
     * @param socket Сокет подключившегося клиента
     */
    void handle_client_connection(tcp::socket socket,
                                  const std::string &client_ip,
                                  uint16_t client_port);

    /**
     * @brief Остановка приема новых клиентов
     */
    void stop_accepting_clients();

    /**
     * @brief Распределение и отправка задачи всем клиентам
     * @param params Параметры интегрирования
     * @return true, если задачи успешно отправлены
     */
    bool distribute_and_send_tasks(const IntegrationParameters &params);

    /**
     * @brief Отправка задач одному клиенту
     * @param client Указатель на клиента
     * @param batch Пакет задач
     * @return true, если успешно отправлено
     */
    bool send_tasks_to_client(ClientConnection *client, const TaskBatch &batch);

    /**
     * @brief Сбор результатов от всех клиентов
     * @param aggregator Агрегатор результатов
     * @return true, если все результаты получены
     */
    bool collect_results(ResultAggregator &aggregator);

    /**
     * @brief Получение результатов от одного клиента
     * @param client Указатель на клиента
     * @param aggregator Агрегатор результатов
     * @return true, если успешно получено
     */
    bool receive_results_from_client(ClientConnection *client, ResultAggregator &aggregator);

    /**
     * @brief Отправитка команд завершения работы всем клиентам
     */
    void send_stop_command_to_all_clients();

    /**
     * @brief Вывести итоговый результат
     * @param final_result Значение интеграла
     * @param params Параметры интегрирования
     */
    void print_final_result(double final_result, const IntegrationParameters &params);

    // Контекст ввода/вывода
    boost::asio::io_context io_context_;
    // Аксептор входящих подключений
    std::unique_ptr<tcp::acceptor> acceptor_;
    // Порт сервера
    uint16_t port_;

    // Менеджер клиентов
    ClientManager client_manager_;
    // Распределитель задач
    TaskDistributor task_distributor_;
    // Обработчик пользовательского ввода
    InputHandler input_handler_;

    // Поток приема подключений
    std::thread accept_thread_;
    // Флаг работы сервера
    std::atomic<bool> running_{false};
    // Флаг получения команды START
    std::atomic<bool> start_received_{false};
};
