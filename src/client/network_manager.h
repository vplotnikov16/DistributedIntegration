#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <string>
#include "messages.h"
#include "systeminfo.h"

using tcp = boost::asio::ip::tcp;

/**
 * @file network_manager.h
 * @brief Модуль управления сетевым подключением клиента к серверу
 */

/**
 * @class NetworkManager
 * @brief Отвечает за сетевое взаимодействие с сервером
 *
 * Обеспечивает установку соединения, handshake, отправку и получение данных.
 */
class NetworkManager
{
public:
    /**
     * @brief Конструктор с параметрами подключения
     * @param server_address IP-адрес сервера
     * @param server_port Порт сервера
     */
    NetworkManager(const std::string &server_address, uint16_t server_port);

    /**
     * @brief Деструктор - закрывает соединение
     */
    ~NetworkManager();

    // Запрет копирования и перемещения
    NetworkManager(const NetworkManager &) = delete;
    NetworkManager &operator=(const NetworkManager &) = delete;
    NetworkManager(NetworkManager &&) = delete;
    NetworkManager &operator=(NetworkManager &&) = delete;

    /**
     * @brief Подключается к серверу
     * @throws std::runtime_error если не удалось подключиться
     */
    void connect();

    /**
     * @brief Проверяет, установлено ли соединение
     * @return true если соединение активно
     */
    bool is_connected() const;

    /**
     * @brief Выполняет handshake с сервером
     * @param client_version Версия клиента
     * @param system_info Информация о системе клиента
     * @return Ответ сервера с присвоенным client_id
     * @throws std::runtime_error если handshake не удался
     */
    HandshakeResponse perform_handshake(
        const std::string &client_version,
        const SystemInfo &system_info);

    /**
     * @brief Получает пакет задач от сервера
     * @return Пакет задач для выполнения
     * @throws std::runtime_error при ошибке получения данных
     */
    TaskBatch receive_tasks();

    /**
     * @brief Отправляет пакет результатов на сервер
     * @param results Пакет результатов выполнения задач
     * @throws std::runtime_error при ошибке отправки данных
     */
    void send_results(const ResultBatch &results);

    /**
     * @brief Получает команду от сервера
     * @return Управляющая команда
     * @throws std::runtime_error при ошибке получения данных
     */
    Command receive_command();

    /**
     * @brief Отправляет команду серверу
     * @param command Команда для отправки
     * @throws std::runtime_error при ошибке отправки данных
     */
    void send_command(const Command &command);

    /**
     * @brief Закрывает соединение с сервером
     */
    void disconnect();

    /**
     * @brief Возвращает адрес сервера
     * @return Строка "IP:PORT"
     */
    std::string get_server_address() const;

private:
    // Адрес сервера
    std::string server_address_;
    // Порт сервера
    uint16_t server_port_;
    // IO context для Boost.Asio
    boost::asio::io_context io_context_;
    // TCP сокет
    std::unique_ptr<tcp::socket> socket_;
    // Флаг подключения
    bool connected_;
};
