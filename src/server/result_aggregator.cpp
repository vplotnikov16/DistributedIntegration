#include "result_aggregator.h"
#include "logger.h"
#include <chrono>

ResultAggregator::ResultAggregator(size_t expected_results_count)
    : expected_count_(expected_results_count)
{
    all_results_.reserve(expected_results_count);
    LOG_INFO("ResultAggregator initialized, expecting {} results", expected_count_);
}

void ResultAggregator::add_result(const ResultBatch &batch)
{
    std::lock_guard<std::mutex> lock(mutex_);

    LOG_DEBUG("Received result batch from client ID={}, {} results, time: {:.3f}s",
              batch.client_id,
              batch.results.size(),
              batch.total_time_seconds);

    for (const auto &result : batch.results)
    {
        if (result.success)
        {
            total_sum_ += result.value;
            successful_count_++;
            LOG_TRACE("Task {}: value={}", result.task_id, result.value);
        }
        else
        {
            error_count_++;
            LOG_ERROR("Task {} failed: {}", result.task_id, result.error_message);
        }

        all_results_.push_back(result);
    }

    received_count_ += batch.results.size();

    LOG_INFO("Progress: {}/{} results received ({:.1f}%)",
             received_count_.load(),
             expected_count_,
             100.0 * received_count_.load() / expected_count_);

    // Уведомляем ожидающие потоки
    cv_.notify_all();
}

bool ResultAggregator::wait_for_all_results(uint32_t timeout_seconds)
{
    std::unique_lock<std::mutex> lock(mutex_);

    LOG_INFO("Waiting for all results...");

    if (timeout_seconds == 0)
    {
        // Бесконечное ожидание
        cv_.wait(lock, [this]
                 { return received_count_.load() >= expected_count_; });
        return true;
    }
    else
    {
        // Ожидание с таймаутом
        auto timeout = std::chrono::seconds(timeout_seconds);
        bool result = cv_.wait_for(lock, timeout, [this]
                                   { return received_count_.load() >= expected_count_; });

        if (!result)
        {
            LOG_WARN("Timeout waiting for results: received {}/{}",
                     received_count_.load(), expected_count_);
        }

        return result;
    }
}

double ResultAggregator::get_final_result() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return total_sum_;
}

void ResultAggregator::log_results_info() const
{
    std::lock_guard<std::mutex> lock(mutex_);

    LOG_INFO("=== Integration Results ===");
    LOG_INFO("Total tasks: {}", expected_count_);
    LOG_INFO("Received: {}", received_count_.load());
    LOG_INFO("Successful: {}", successful_count_.load());
    LOG_INFO("Errors: {}", error_count_.load());
    LOG_INFO("Final result: {:.15f}", total_sum_);
    LOG_INFO("===========================");
}
