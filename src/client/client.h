#pragma once

#include "network_manager.h"
#include "task_executor.h"
#include "worker_pool.h"
#include "integrator.h"
#include "messages.h"
#include "systeminfo.h"
#include <memory>
#include <string>

/**
 * @file client.h
 * @brief Основной модуль
 */

/**
 * @class Client
 * @brief Главный контроллер клиентского приложения
 * 
 * Координирует работу всех компонентов: сеть, вычисления, многопоточность.
 */
class Client
{
public:
    /**
     * @brief Конструктор с параметрами подключения
     * @param server_address IP-адрес сервера
     * @param server_port Порт сервера
     */
    Client(const std::string &server_address,
           uint16_t server_port);

    /**
     * @brief Деструктор
     */
    ~Client();

    // Запрет копирования и перемещения
    Client(const Client &) = delete;
    Client &operator=(const Client &) = delete;
    Client(Client &&) = delete;
    Client &operator=(Client &&) = delete;

    /**
     * @brief Запускает клиента
     * 
     * Выполняет полный цикл работы:
     * 1. Подключение к серверу
     * 2. Handshake
     * 3. Получение задач
     * 4. Выполнение задач
     * 5. Отправка результатов
     * 6. Ожидание команды STOP_WORK
     * 7. Завершение
     * 
     * @throws std::runtime_error при критических ошибках
     */
    void run();

    /**
     * @brief Устанавливает стратегию интегрирования
     * @param strategy Указатель на стратегию
     */
    void set_integration_strategy(std::unique_ptr<IIntegrationStrategy> strategy);

private:
    /**
     * @brief Собирает информацию о системе
     * @return Структура SystemInfo
     */
    SystemInfo collect_system_info();

    /**
     * @brief Выполняет задачи параллельно
     * @param tasks Вектор задач
     * @return Вектор результатов
     */
    std::vector<Result> execute_tasks(const std::vector<Task> &tasks);

    // Версия клиента
    std::string client_version_;
    // ID клиента, присвоенный сервером
    uint64_t client_id_;
    // Информация о системе
    SystemInfo system_info_;
    
    // Менеджер сетевого взаимодействия
    std::unique_ptr<NetworkManager> network_manager_;
    // Интегратор с выбранной стратегией
    std::shared_ptr<Integrator> integrator_;
    // Executor для выполнения задач
    std::unique_ptr<TaskExecutor> task_executor_;
    // Пул потоков для параллельных вычислений
    std::unique_ptr<WorkerPool> worker_pool_;
};
