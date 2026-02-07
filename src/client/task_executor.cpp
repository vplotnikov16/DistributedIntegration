#include "task_executor.h"
#include "logger.h"
#include <stdexcept>

TaskExecutor::TaskExecutor(std::shared_ptr<Integrator> integrator)
    : integrator_(integrator)
{
    if (!integrator_)
    {
        throw std::invalid_argument("Integrator cannot be nullptr");
    }

    LOG_DEBUG("TaskExecutor created with method: {}", integrator_->get_current_method());
}

void TaskExecutor::set_integrator(std::shared_ptr<Integrator> integrator)
{
    if (!integrator)
    {
        throw std::invalid_argument("Integrator cannot be nullptr");
    }

    integrator_ = integrator;
    LOG_INFO("Integrator changed to: {}", integrator_->get_current_method());
}

Result TaskExecutor::execute_single_task(const Task &task)
{
    LOG_DEBUG("Executing task {}: [{}, {}] with step {}",
              task.id, task.begin, task.end, task.step);

    try
    {
        // Валидация задачи
        if (!task.is_valid())
        {
            LOG_ERROR("Task {} is invalid", task.id);
            Result result;
            result.task_id = task.id;
            result.value = 0.0;
            result.success = false;
            result.error_message = "Invalid task parameters";
            return result;
        }

        // Выполнение интегрирования
        Result result = integrator_->execute_task(task);

        if (result.success)
        {
            LOG_DEBUG("Task {} completed successfully: result = {}",
                      task.id, result.value);
        }
        else
        {
            LOG_WARN("Task {} failed: {}", task.id, result.error_message);
        }

        return result;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Exception during task {} execution: {}", task.id, e.what());

        Result result;
        result.task_id = task.id;
        result.value = 0.0;
        result.success = false;
        result.error_message = std::string("Exception: ") + e.what();
        return result;
    }
}

std::vector<Result> TaskExecutor::execute_tasks_sequential(const std::vector<Task> &tasks)
{
    LOG_INFO("Executing {} tasks sequentially...", tasks.size());

    std::vector<Result> results;
    results.reserve(tasks.size());

    for (const auto &task : tasks)
    {
        results.push_back(execute_single_task(task));
    }

    // Подсчёт успешных задач
    size_t successful = 0;
    for (const auto &result : results)
    {
        if (result.success)
        {
            successful++;
        }
    }

    LOG_INFO("Completed {} tasks: {} successful, {} failed",
             tasks.size(), successful, tasks.size() - successful);

    return results;
}

std::string TaskExecutor::get_current_method() const
{
    if (!integrator_)
    {
        return "None";
    }
    return integrator_->get_current_method();
}
