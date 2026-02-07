#include "worker_pool.h"
#include "logger.h"
#include <stdexcept>
#include <algorithm>

WorkerPool::WorkerPool(uint32_t num_threads, std::shared_ptr<Integrator> integrator)
    : num_threads_(num_threads),
      integrator_(integrator)
{
    if (num_threads_ == 0)
    {
        throw std::invalid_argument("Number of threads must be > 0");
    }

    if (!integrator_)
    {
        throw std::invalid_argument("Integrator cannot be nullptr");
    }

    LOG_INFO("WorkerPool created with {} threads, method: {}",
             num_threads_, integrator_->get_current_method());
}

std::vector<Result> WorkerPool::execute_tasks_parallel(const std::vector<Task> &tasks)
{
    if (tasks.empty())
    {
        LOG_WARN("No tasks to execute");
        return {};
    }

    LOG_INFO("Starting parallel execution of {} tasks on {} threads...",
             tasks.size(), num_threads_);

    // Подготовка результатов
    std::vector<Result> results(tasks.size());

    // Индекс следующей задачи для обработки
    size_t task_index = 0;
    std::mutex index_mutex;

    // Создание и запуск потоков
    std::vector<std::thread> workers;
    workers.reserve(num_threads_);

    for (uint32_t i = 0; i < num_threads_; ++i)
    {
        workers.emplace_back(
            &WorkerPool::worker_function,
            this,
            std::cref(tasks),
            std::ref(results),
            std::ref(task_index),
            std::ref(index_mutex));
    }

    // Ожидание завершения всех потоков
    for (auto &worker : workers)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }

    // Подсчёт статистики
    size_t successful = 0;
    for (const auto &result : results)
    {
        if (result.success)
        {
            successful++;
        }
    }

    LOG_INFO("Parallel execution completed: {} successful, {} failed",
             successful, tasks.size() - successful);

    return results;
}

uint32_t WorkerPool::get_num_threads() const
{
    return num_threads_;
}

void WorkerPool::worker_function(
    const std::vector<Task> &tasks,
    std::vector<Result> &results,
    size_t &task_index,
    std::mutex &mutex)
{
    std::thread::id thread_id = std::this_thread::get_id();
    LOG_DEBUG("Worker thread {} started", 
        std::hash<std::thread::id>{}(thread_id));

    while (true)
    {
        size_t current_index;

        // Получаем следующую задачу (критическая секция)
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (task_index >= tasks.size())
            {
                // Все задачи обработаны
                break;
            }
            current_index = task_index++;
        }

        // Выполняем задачу (вне критической секции)
        const Task &task = tasks[current_index];
        LOG_TRACE("Thread {} executing task {}", 
            std::hash<std::thread::id>{}(thread_id), 
            task.id);

        try
        {
            // Валидация задачи
            if (!task.is_valid())
            {
                Result result;
                result.task_id = task.id;
                result.value = 0.0;
                result.success = false;
                result.error_message = "Invalid task parameters";
                results[current_index] = result;
                continue;
            }

            // Выполнение интегрирования
            Result result = integrator_->execute_task(task);
            results[current_index] = result;

            if (result.success)
            {
                LOG_TRACE("Thread {} completed task {}: result = {}",
                          std::hash<std::thread::id>{}(thread_id), 
                          task.id, result.value);
            }
            else
            {
                LOG_WARN("Thread {} failed task {}: {}",
                         std::hash<std::thread::id>{}(thread_id), 
                         task.id, result.error_message);
            }
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("Thread {} exception on task {}: {}",
                      std::hash<std::thread::id>{}(thread_id), 
                      task.id, e.what());

            Result result;
            result.task_id = task.id;
            result.value = 0.0;
            result.success = false;
            result.error_message = std::string("Exception: ") + e.what();
            results[current_index] = result;
        }
    }

    LOG_DEBUG("Worker thread {} finished", 
        std::hash<std::thread::id>{}(thread_id));
}
