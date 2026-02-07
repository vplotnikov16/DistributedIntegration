#include "task_distributor.h"
#include "logger.h"
#include <stdexcept>
#include <cmath>

std::map<uint64_t, TaskBatch> TaskDistributor::distribute_tasks(
    const std::vector<ClientConnection *> &clients,
    double lower,
    double upper,
    double step)
{
    // Валидация параметров
    if (clients.empty())
    {
        throw std::runtime_error("No clients connected");
    }

    LOG_INFO("Distributing task: range=[{}, {}], step={}, clients={}",
             lower, upper, step, clients.size());

    // Подсчёт общего количества ядер
    uint32_t total_cores = 0;
    for (const auto *client : clients)
    {
        total_cores += client->get_cpu_cores();
    }

    LOG_INFO("Total CPU cores available: {}", total_cores);

    // Вычисляем количество задач для каждого клиента
    auto tasks_per_client = calculate_tasks_per_client(clients, total_cores);

    // Распределяем диапазон
    double total_range = upper - lower;
    double current_position = lower;

    std::map<uint64_t, TaskBatch> result;
    total_tasks_ = 0;

    for (size_t i = 0; i < clients.size(); ++i)
    {
        const auto *client = clients[i];
        uint32_t num_tasks = tasks_per_client[i];

        TaskBatch batch;
        batch.tasks.reserve(num_tasks);

        double range_for_client = total_range *
                                  static_cast<double>(client->get_cpu_cores()) /
                                  static_cast<double>(total_cores);

        double task_range = range_for_client / num_tasks;

        for (uint32_t j = 0; j < num_tasks; ++j)
        {
            Task task;
            task.id = next_task_id_++;
            task.begin = current_position;
            task.end = (j == num_tasks - 1 && i == clients.size() - 1)
                           ? upper // Последняя задача - точно до конца
                           : current_position + task_range;
            task.step = step;

            batch.tasks.push_back(task);
            current_position = task.end;
            total_tasks_++;
        }

        double first_begin = batch.tasks.front().begin;
        double last_end = batch.tasks.back().end;
        
        LOG_INFO("Client ID={}: assigned {} tasks, range=[{}, {}]",
                 client->get_client_id(),
                 num_tasks,
                 first_begin,
                 last_end);

        result[client->get_client_id()] = std::move(batch);
    }

    LOG_INFO("Total tasks created: {}", total_tasks_);

    return result;
}

std::vector<uint32_t> TaskDistributor::calculate_tasks_per_client(
    const std::vector<ClientConnection *> &clients,
    uint32_t total_cores) const
{
    std::vector<uint32_t> result;
    result.reserve(clients.size());

    // Каждому клиенту выделяем количество задач = количество ядер
    // (чтобы максимально утилизировать каждое ядро)
    for (const auto *client : clients)
    {
        result.push_back(client->get_cpu_cores());
    }

    return result;
}
