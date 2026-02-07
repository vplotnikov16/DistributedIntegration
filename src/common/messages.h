#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include "systeminfo.h"

/**
 * @file messages.h
 * @brief Определение структур для обмена сообщениями между клиентом и сервером
 */

/**
 * @struct Task
 * @brief Задача численного интегрирования для клиента
 */
struct Task
{
    // Уникальный ID задачи
    uint64_t id = 0;
    // Нижний предел интегрирования
    double begin = 0.0;
    // Верхний предел интегрирования
    double end = 0.0;
    // Шаг интегрирования
    double step = 0.0;

    /**
     * @brief Проверяет корректность параметров задачи
     * @return true, если задача валидна, false иначе
     */
    bool is_valid() const
    {
        // ограничения для интегрирования 1/ln(x)
        bool result = true;
        
        // Начало не может быть больше конца, шаг должен быть положительным
        // и быть меньше длины интегрируемого интервала
        result &= !(begin >= end || step <= 0.0 || step >= (end - begin));

        // Нижний предел должен быть положительным
        result &= !(begin <= 0.0);

        // Интервал не должен содержать x = 1
        result &= !(begin < 1.0 && end > 1.0);
        result &= !(std::abs(begin - 1.0) < 1e-10 || std::abs(end - 1.0) < 1e-10);

        return result;
    }

    /**
     * @brief Метод сериализации для Cereal
     */
    template <class Archive>
    void serialize(Archive &archive)
    {
        archive(
            CEREAL_NVP(id),
            CEREAL_NVP(begin),
            CEREAL_NVP(end),
            CEREAL_NVP(step));
    }
};

/**
 * @struct Result
 * @brief Результат выполнения задачи интегрирования
 */
struct Result
{
    // ID задачи, к которой относится результат
    uint64_t task_id = 0;
    // Вычисленное значение интеграла
    double value = 0.0;
    // Флаг успешного выполнения
    bool success = true;
    // Сообщение об ошибке, если !success
    std::string error_message;

    /**
     * @brief Метод сериализации для Cereal
     */
    template <class Archive>
    void serialize(Archive &archive)
    {
        archive(
            CEREAL_NVP(task_id),
            CEREAL_NVP(value),
            CEREAL_NVP(success),
            CEREAL_NVP(error_message));
    }
};

/**
 * @struct TaskBatch
 * @brief Пакет задач для одного клиента
 */
struct TaskBatch
{
    // Массив задач для выполнения
    std::vector<Task> tasks;

    /**
     * @brief Метод сериализации для Cereal
     */
    template <class Archive>
    void serialize(Archive &archive)
    {
        archive(CEREAL_NVP(tasks));
    }
};

/**
 * @struct ResultBatch
 * @brief Пакет результатов от одного клиента
 */
struct ResultBatch
{
    // ID клиента, который отправил результаты
    uint64_t client_id = 0;
    // Массив результатов вычислений
    std::vector<Result> results;
    // Общее время выполнения всех задач
    double total_time_seconds = 0.0;

    /**
     * @brief Метод сериализации для Cereal
     */
    template <class Archive>
    void serialize(Archive &archive)
    {
        archive(
            CEREAL_NVP(client_id),
            CEREAL_NVP(results),
            CEREAL_NVP(total_time_seconds));
    }
};

/**
 * @enum CommandType
 * @brief Типы управляющих команд
 */
enum class CommandType : uint8_t
{
    // Команда начать вычисления
    START_WORK = 1,
    // Команда завершить работу
    STOP_WORK = 2,
    // Проверка связи
    PING = 3,
    // Команда подтверждения
    ACK = 4
};

/**
 * @struct Command
 * @brief Управляющая команда
 */
struct Command
{
    CommandType type = CommandType::PING;
    std::string message;

    /**
     * @brief Метод сериализации для Cereal
     */
    template <class Archive>
    void serialize(Archive &archive)
    {
        archive(
            CEREAL_NVP(type),
            CEREAL_NVP(message));
    }
};

/**
 * @struct ClientInfo
 * @brief Информация о клиенте с уникальным идентификатором
 */
struct ClientInfo
{
    // Уникальный ID клиента
    uint64_t client_id = 0;
    // Информация о системе клиента
    SystemInfo system_info;

    /**
     * @brief Метод сериализации для Cereal
     */
    template <class Archive>
    void serialize(Archive &archive)
    {
        archive(
            CEREAL_NVP(client_id),
            CEREAL_NVP(system_info));
    }
};

/**
 * @struct HandshakeRequest
 * @brief Запрос на подключение от клиента
 */
struct HandshakeRequest
{
    // Версия клиента
    std::string client_version;
    SystemInfo system_info;

    /**
     * @brief Метод сериализации для Cereal
     */
    template <class Archive>
    void serialize(Archive &archive)
    {
        archive(
            CEREAL_NVP(client_version),
            CEREAL_NVP(system_info));
    }
};

/**
 * @struct HandshakeResponse
 * @brief Ответ сервера на запрос подключения
 */
struct HandshakeResponse
{
    // Присвоенный ID клиента
    uint64_t assigned_client_id = 0;
    // Версия сервера
    std::string server_version = "1.0.0";
    // Принято ли подключение
    bool accepted = true;
    // Сообщение
    std::string message;

    /**
     * @brief Метод сериализации для Cereal
     */
    template <class Archive>
    void serialize(Archive &archive)
    {
        archive(
            CEREAL_NVP(assigned_client_id),
            CEREAL_NVP(server_version),
            CEREAL_NVP(accepted),
            CEREAL_NVP(message));
    }
};

/**
 * @enum MessageType
 * @brief Типы сообщений
 */
enum class MessageType : uint8_t
{
    // Запрос на подключение
    HANDSHAKE_REQUEST = 1,
    // Ответ на запрос подключения
    HANDSHAKE_RESPONSE = 2,
    // Пакет задач
    TASK_BATCH = 3,
    // Пакет результатов
    RESULT_BATCH = 4,
    // Управляющая команда
    COMMAND = 5
};
