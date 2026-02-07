#pragma once

#include "messages.h"
#include "integrator.h"
#include <vector>
#include <thread>
#include <mutex>
#include <memory>
#include <functional>

/**
 * @file worker_pool.h
 * @brief Модуль пула потоков для параллельного выполнения задач
 */

/**
 * @class WorkerPool
 * @brief Управляет пулом рабочих потоков для параллельных вычислений
 *
 * Распределяет задачи между потоками для максимальной утилизации CPU.
 */
class WorkerPool
{
public:
    /**
     * @brief Конструктор с указанием количества потоков и интегратора
     * @param num_threads Количество рабочих потоков 
     * @param integrator Интегратор для выполнения вычислений
     * @throws std::invalid_argument если num_threads == 0 или integrator == nullptr
     */
    WorkerPool(uint32_t num_threads, std::shared_ptr<Integrator> integrator);

    /**
     * @brief Деструктор - ожидает завершения всех потоков
     */
    ~WorkerPool() = default;

    // Запрет копирования и перемещения
    WorkerPool(const WorkerPool &) = delete;
    WorkerPool &operator=(const WorkerPool &) = delete;
    WorkerPool(WorkerPool &&) = delete;
    WorkerPool &operator=(WorkerPool &&) = delete;

    /**
     * @brief Выполняет задачи параллельно на всех потоках
     * @param tasks Вектор задач для выполнения
     * @return Вектор результатов в том же порядке, что и задачи
     */
    std::vector<Result> execute_tasks_parallel(const std::vector<Task> &tasks);

    /**
     * @brief Возвращает количество потоков в пуле
     * @return Количество потоков
     */
    uint32_t get_num_threads() const;

private:
    /**
     * @brief Функция-работник для потока
     * @param tasks Ссылка на вектор задач
     * @param results Ссылка на вектор результатов
     * @param task_index Ссылка на текущий индекс задачи
     * @param mutex Мьютекс для синхронизации доступа
     */
    void worker_function(
        const std::vector<Task> &tasks,
        std::vector<Result> &results,
        size_t &task_index,
        std::mutex &mutex);

    // Количество рабочих потоков
    uint32_t num_threads_;
    // Интегратор для вычислений
    std::shared_ptr<Integrator> integrator_;
};
