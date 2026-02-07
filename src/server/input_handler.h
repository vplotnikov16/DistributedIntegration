#pragma once

#include <string>
#include <functional>
#include <thread>
#include <atomic>

/**
 * @file input_handler.h
 * @brief Модуль обработки пользовательского ввода из консоли в отдельном потоке
 */

/**
 * @class InputHandler
 * @brief Читает команды пользователя из консоли в отдельном потоке
 */
class InputHandler
{
public:
    /**
     * @brief Тип callback-функции для обработки команды START
     */
    using StartCallback = std::function<void()>;

    InputHandler() = default;
    ~InputHandler();

    // Запрет копирования
    InputHandler(const InputHandler &) = delete;
    InputHandler &operator=(const InputHandler &) = delete;

    /**
     * @brief Запускает обработчик ввода в отдельном потоке
     *
     * @param on_start Callback, вызываемый при получении команды START
     *
     * Поток будет ожидать ввода "START" от пользователя и вызывать callback
     */
    void start(StartCallback on_start);

    /**
     * @brief Остановить обработчик ввода
     *
     * Останавливает поток чтения и дожидается его завершения
     */
    void stop();

    /**
     * @brief Проверить, запущен ли обработчик
     * @return true, если обработчик активен
     */
    bool is_running() const { return running_.load(); }

private:
    /**
     * @brief Функция потока обработки ввода
     */
    void input_thread_func();

    // Поток обработки ввода
    std::thread input_thread_;
    // Флаг работы потока
    std::atomic<bool> running_{false};
    // Флаг запроса остановки
    std::atomic<bool> stop_requested_{false};
    // Callback для команды START
    StartCallback on_start_callback_;
};
