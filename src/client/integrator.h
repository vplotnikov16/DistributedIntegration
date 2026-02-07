#pragma once

#include "integration_methods/integration_strategy.h"
#include "messages.h"
#include <memory>
#include <vector>

/**
 * @file integrator.h
 * @brief Модуль интегратора, в котором устанавливается метод интегрирования
 */

/**
 * @class Integrator
 * @brief Выполняет численное интегрирование с использованием выбранной стратегии
 *
 * Данный класс реализует паттерн Strategy, для легкого выбора метода интегрирования
 */
class Integrator
{
public:
    /**
     * @brief Конструктор по умолчанию (без стратегии)
     */
    Integrator() = default;

    /**
     * @brief Конструктор с указанием стратегии
     *
     * @param strategy Указатель на стратегию интегрирования
     *
     * @throws std::invalid_argument если strategy == nullptr
     */
    explicit Integrator(std::unique_ptr<IIntegrationStrategy> strategy);

    /**
     * @brief Устанавливает новую стратегию интегрирования
     *
     * @param strategy Указатель на новую стратегию
     *
     * @throws std::invalid_argument если strategy == nullptr
     */
    void set_strategy(std::unique_ptr<IIntegrationStrategy> strategy);

    /**
     * @brief Возвращает название текущей стратегии
     * @return Строка с названием метода интегрирования
     * @throws std::runtime_error если стратегия не установлена
     */
    std::string get_current_method() const;

    /**
     * @brief Выполняет интегрирование одной задачи
     *
     * @param task Задача для выполнения
     * @return Результат выполнения задачи
     * @throws std::runtime_error если стратегия не установлена
     */
    Result execute_task(const Task &task);

    /**
     * @brief Выполняет интегрирование пакета задач
     *
     * @param tasks Вектор задач для выполнения
     * @return Вектор результатов
     * @throws std::runtime_error если стратегия не установлена
     */
    std::vector<Result> execute_tasks(const std::vector<Task> &tasks);

private:
    // Текущая стратегия интегрирования
    std::unique_ptr<IIntegrationStrategy> strategy_;
};
