#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <atomic>
#include "messages.h"
#include "systeminfo.h"

using boost::asio::ip::tcp;

/**
 * @file client_connection.h
 * @brief Модуль представления одного подключенного клиента
 */

/**
 * @class ClientConnection
 * @brief Хранит информацию и состояние одного подключенного клиента
 *
 * Каждый экземпляр представляет отдельное подключение к клиенту
 * и содержит сокет, системную информацию и статус
 */
class ClientConnection
{
public:
    /**
     * @brief Конструктор
     * @param socket TCP сокет клиента
     * @param client_id Уникальный ID клиента
     * @param system_info Информация о системе клиента
     */
    ClientConnection(
        tcp::socket socket,
        uint64_t client_id,
        const SystemInfo &system_info);

    /**
     * @brief Деструктор - закрывает сокет
     */
    ~ClientConnection();

    // Запрет копирования
    ClientConnection(const ClientConnection &) = delete;
    ClientConnection &operator=(const ClientConnection &) = delete;

    // Разрешаем перемещение
    ClientConnection(ClientConnection &&) noexcept = default;
    ClientConnection &operator=(ClientConnection &&) noexcept = default;

    /**
     * @brief Геттер ID клиента
     * @return ID клиента
     */
    uint64_t get_client_id() const { return client_id_; }

    /**
     * @brief Геттер информации о системе клиента
     * @return Структура SystemInfo
     */
    const SystemInfo &get_system_info() const { return system_info_; }

    /**
     * @brief Геттер числа ядер CPU клиента
     * @return Количество ядер
     */
    uint32_t get_cpu_cores() const { return system_info_.cpu_cores; }

    /**
     * @brief Геттер IP адреса клиента
     * @return Строка с IP адресом
     */
    std::string get_ip_address() const;

    /**
     * @brief Геттер порта клиента
     * @return Номер порта
     */
    uint16_t get_port() const;

    /**
     * @brief Геттер ссылки на сокет
     * @return Ссылка на TCP сокет
     */
    tcp::socket &get_socket() { return socket_; }

    /**
     * @brief Проверяет, открыто ли соединение
     * @return true, если сокет открыт
     */
    bool is_connected() const;

    /**
     * @brief Закрывает соединение
     */
    void close();

    /**
     * @brief Помечает у клиента статус "задача отправлена"
     */
    void mark_task_sent() { task_sent_.store(true); }

    /**
     * @brief Помечает у клиента статус "результат получен"
     */
    void mark_result_received() { result_received_.store(true); }

    /**
     * @brief Проверяет, отправлена ли задача клиенту
     * @return true, если задача отправлена
     */
    bool is_task_sent() const { return task_sent_.load(); }

    /**
     * @brief Проверяет, получен ли результат от клиента
     * @return true, если результат получен
     */
    bool is_result_received() const { return result_received_.load(); }

private:
    // TCP сокет клиента
    tcp::socket socket_;
    // Уникальный ID клиента
    uint64_t client_id_;
    // Информация о системе клиента
    SystemInfo system_info_;
    // Флаг отправки задачи
    std::atomic<bool> task_sent_{false};
    // Флаг получения результата
    std::atomic<bool> result_received_{false};
};
