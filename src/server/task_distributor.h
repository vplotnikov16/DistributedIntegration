#pragma once

#include <vector>
#include <map>
#include "messages.h"
#include "client_connection.h"

/**
 * @file task_distributor.h
 * @brief Модуль распределения задач интегрирования между клиентами
 */

/**
 * @class TaskDistributor
 * @brief Разделяет общую задачу интегрирования на подзадачи для клиентов
 *
 * Распределяет диапазон интегрирования пропорционально количеству ядер каждого клиента
 */
class TaskDistributor
{
public:
    TaskDistributor() = default;

    /**
     * @brief Распределяет задачу между клиентами
     *
     * @param clients Список всех клиентов
     * @param lower Нижний предел интегрирования
     * @param upper Верхний предел интегрирования
     * @param step Шаг интегрирования
     * @return Карта: client_id -> TaskBatch (задачи для каждого клиента)
     *
     * @throws std::invalid_argument если параметры некорректны
     * @throws std::runtime_error если нет клиентов
     */
    std::map<uint64_t, TaskBatch> distribute_tasks(
        const std::vector<ClientConnection *> &clients,
        double lower,
        double upper,
        double step);

    /**
     * @brief Геттер суммарного числа созданных задач
     * @return Количество задач
     */
    size_t get_total_tasks_count() const { return total_tasks_; }

private:
    /**
     * @brief Вычисляет количество задач для клиента пропорционально его ядрам
     */
    std::vector<uint32_t> calculate_tasks_per_client(
        const std::vector<ClientConnection *> &clients,
        uint32_t total_cores) const;
    
    // Общее количество задач
    size_t total_tasks_{0};
    // Счетчик ID задач
    uint64_t next_task_id_{1};
};
