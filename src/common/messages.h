#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>

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
    uint64_t id = 0;    ///< Уникальный идентификатор задачи
    double begin = 0.0; ///< Нижний предел интегрирования
    double end = 0.0;   ///< Верхний предел интегрирования
    double step = 0.0;  ///< Шаг интегрирования

    /**
     * @brief Проверяет корректность параметров задачи
     * @return true, если задача валидна, false иначе
     */
    bool is_valid() const
    {
        // ограничения для интегрирования 1/ln(x)
        return begin < end &&
               step > 0.0 &&
               step < (end - begin) &&
               begin > 1.0;
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
            CEREAL_NVP(message)
        );
    }
};
