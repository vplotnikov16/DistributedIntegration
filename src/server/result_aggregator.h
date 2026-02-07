#pragma once

#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "messages.h"

/**
 * @file result_aggregator.h
 * @brief Модуль сбора и агрегации результатов от клиентов
 */

/**
 * @class ResultAggregator
 * @brief Собирает результаты от всех клиентов и вычисляет итоговый результат
 *
 * Обеспечивает потокобезопасный сбор результатов и ожидание получения всех данных
 */
class ResultAggregator
{
public:
    /**
     * @brief Конструктор
     * @param expected_results_count Ожидаемое количество результатов
     */
    explicit ResultAggregator(size_t expected_results_count);

    /**
     * @brief Добавляет результат от клиента
     * @param batch Пакет результатов от клиента
     */
    void add_result(const ResultBatch &batch);

    /**
     * @brief Ожидает получения всех результатов
     * @param timeout_seconds Таймаут в секундах (0 = бесконечно)
     * @return true, если все результаты получены
     */
    bool wait_for_all_results(uint32_t timeout_seconds = 0);

    /**
     * @brief Получает итоговый результат интегрирования
     * @return Суммарное значение интеграла
     */
    double get_final_result() const;

    /**
     * @brief Получает количество полученных результатов
     * @return Количество результатов
     */
    size_t get_received_count() const { return received_count_.load(); }

    /**
     * @brief Получает количество успешных результатов
     * @return Количество успешных результатов
     */
    size_t get_successful_count() const { return successful_count_.load(); }

    /**
     * @brief Получает количество ошибок
     * @return Количество ошибок
     */
    size_t get_error_count() const { return error_count_.load(); }

    /**
     * @brief Выводит детальную информацию о результатах в лог
     */
    void log_results_info() const;

private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;

    // Ожидаемое количество результатов
    size_t expected_count_;
    // Получено результатов
    std::atomic<size_t> received_count_{0};
    // Успешных результатов
    std::atomic<size_t> successful_count_{0};
    // Количество ошибок
    std::atomic<size_t> error_count_{0};

    // Сумма всех результатов
    double total_sum_{0.0};
    // Все полученные результаты
    std::vector<Result> all_results_;
};
