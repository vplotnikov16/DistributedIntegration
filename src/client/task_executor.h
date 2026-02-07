#pragma once

#include "messages.h"
#include "integrator.h"
#include <vector>
#include <memory>

/**
 * @file task_executor.h
 * @brief Модуль управления выполнением задач интегрирования
 */

/**
 * @class TaskExecutor
 * @brief Выполняет задачи интегрирования с использованием интегратора
 *
 * Координирует процесс вычислений, использует интегратор с выбранной стратегией.
 */
class TaskExecutor
{
public:
    /**
     * @brief Конструктор с интегратором
     * @param integrator Указатель на интегратор с настроенной стратегией
     * @throws std::invalid_argument если integrator == nullptr
     */
    explicit TaskExecutor(std::shared_ptr<Integrator> integrator);

    /**
     * @brief Деструктор
     */
    ~TaskExecutor() = default;

    /**
     * @brief Устанавливает новый интегратор
     * @param integrator Указатель на интегратор
     * @throws std::invalid_argument если integrator == nullptr
     */
    void set_integrator(std::shared_ptr<Integrator> integrator);

    /**
     * @brief Выполняет одну задачу интегрирования
     * @param task Задача для выполнения
     * @return Результат выполнения задачи
     */
    Result execute_single_task(const Task &task);

    /**
     * @brief Выполняет пакет задач последовательно
     * @param tasks Вектор задач
     * @return Вектор результатов
     */
    std::vector<Result> execute_tasks_sequential(const std::vector<Task> &tasks);

    /**
     * @brief Возвращает название текущего метода интегрирования
     * @return Строка с названием метода
     */
    std::string get_current_method() const;

private:
    // Интегратор для выполнения вычислений
    std::shared_ptr<Integrator> integrator_;
};
