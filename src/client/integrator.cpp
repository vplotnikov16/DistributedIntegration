#include "integrator.h"
#include <stdexcept>
#include <logger.h>

Integrator::Integrator(std::unique_ptr<IIntegrationStrategy> strategy)
{
    if (!strategy)
    {
        throw std::invalid_argument("Integration strategy cannot be nullptr");
    }

    strategy_ = std::move(strategy);

    LOG_INFO("Integrator initialized with strategy: {}", strategy_->get_method_name());
}

void Integrator::set_strategy(std::unique_ptr<IIntegrationStrategy> strategy)
{
    if (!strategy)
    {
        throw std::invalid_argument("Integration strategy cannot be nullptr");
    }

    std::string old_method = strategy_ ? strategy_->get_method_name() : "none";
    strategy_ = std::move(strategy);

    LOG_INFO("Integration strategy changed from '{}' to '{}'",
             old_method, strategy_->get_method_name());
}

std::string Integrator::get_current_method() const
{
    if (!strategy_)
    {
        throw std::runtime_error("Integration strategy is not set");
    }
    return strategy_->get_method_name();
}

Result Integrator::execute_task(const Task &task)
{
    Result result;
    result.task_id = task.id;
    result.success = true;

    if (!strategy_)
    {
        result.success = false;
        result.error_message = "Integration strategy is not set";
        result.value = 0.0;
        LOG_ERROR("Cannot execute task {}: strategy is not set", task.id);
        return result;
    }

    try
    {
        // Валидация задачи
        if (!task.is_valid())
        {
            result.success = false;
            result.error_message = "Invalid task parameters";
            result.value = 0.0;

            LOG_ERROR("Task {} validation failed", task.id);
            return result;
        }

        LOG_DEBUG("Executing task {} with method '{}' (range: [{}, {}], step: {})",
                  task.id, strategy_->get_method_name(),
                  task.begin, task.end, task.step);

        // Выполнение интегрирования
        result.value = strategy_->integrate(task.begin, task.end, task.step);

        LOG_DEBUG("Task {} completed successfully, result: {}", task.id, result.value);
    }
    catch (const std::invalid_argument &e)
    {
        result.success = false;
        result.error_message = std::string("Invalid argument: ") + e.what();
        result.value = 0.0;

        LOG_ERROR("Task {} failed with invalid argument: {}", task.id, e.what());
    }
    catch (const std::runtime_error &e)
    {
        result.success = false;
        result.error_message = std::string("Runtime error: ") + e.what();
        result.value = 0.0;

        LOG_ERROR("Task {} failed with runtime error: {}", task.id, e.what());
    }
    catch (const std::exception &e)
    {
        result.success = false;
        result.error_message = std::string("Unexpected error: ") + e.what();
        result.value = 0.0;

        LOG_ERROR("Task {} failed with unexpected error: {}", task.id, e.what());
    }

    return result;
}

std::vector<Result> Integrator::execute_tasks(const std::vector<Task> &tasks)
{
    std::vector<Result> results;
    results.reserve(tasks.size());

    if (!strategy_)
    {
        LOG_ERROR("Cannot execute tasks: strategy is not set");
        throw std::runtime_error("Integration strategy is not set");
    }

    LOG_INFO("Starting execution of {} tasks using '{}'",
             tasks.size(), strategy_->get_method_name());

    for (const auto &task : tasks)
    {
        results.push_back(execute_task(task));
    }

    // Подсчет успешных и неуспешных задач
    size_t successful = 0;
    size_t failed = 0;

    for (const auto &result : results)
    {
        if (result.success)
            ++successful;
        else
            ++failed;
    }

    LOG_INFO("Completed execution: {} successful, {} failed", successful, failed);

    return results;
}
