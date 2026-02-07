#pragma once

#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include "client_connection.h"

/**
 * @file client_manager.h
 * @brief Модуль управления списком подключенных клиентов
 */

/**
 * @class ClientManager
 * @brief Управляет всеми подключенными клиентами
 *
 * Обеспечивает потокобезопасное добавление, удаление и доступ к клиентам
 */
class ClientManager
{
public:
    ClientManager();

    /**
     * @brief Добавляет нового клиента
     * @param connection Уникальный указатель на ClientConnection
     * @return ID добавленного клиента
     */
    uint64_t add_client(std::unique_ptr<ClientConnection> connection);

    /**
     * @brief Геттер числа подключенных клиентов
     * @return Количество клиентов
     */
    size_t get_client_count() const;

    /**
     * @brief Геттер суммарного числа ядер CPU всех клиентов
     * @return Суммарное количество ядер
     */
    uint32_t get_total_cpu_cores() const;

    /**
     * @brief Геттер клиента по ID
     * @param client_id ID клиента
     * @return Указатель на ClientConnection или nullptr, если не найден
     */
    ClientConnection *get_client(uint64_t client_id);

    /**
     * @brief Геттер всех клиентов
     * @return Вектор указателей на всех клиентов
     */
    std::vector<ClientConnection *> get_all_clients();

    /**
     * @brief Удаляет клиента по ID
     * @param client_id ID клиента
     * @return true, если клиент удалён
     */
    bool remove_client(uint64_t client_id);

    /**
     * @brief Удаляет всех клиентов
     */
    void clear();

    /**
     * @brief Останавливает прием новых клиентов
     */
    void stop_accepting();

    /**
     * @brief Проверяет, можно ли принимать новых клиентов
     * @return true, если приём активен
     */
    bool is_accepting() const { return accepting_.load(); }

    /**
     * @brief Выводит информацию обо всех клиентах в лог
     */
    void log_clients_info() const;

private:
    // Мьютекс для потокобезопасности
    mutable std::mutex mutex_;       
    // Список клиентов
    std::vector<std::unique_ptr<ClientConnection>> clients_;
    // Счетчик ID клиентов
    std::atomic<uint64_t> next_client_id_{1};
    // Флаг, принимаем ли новых клиентов
    std::atomic<bool> accepting_{true};
};
