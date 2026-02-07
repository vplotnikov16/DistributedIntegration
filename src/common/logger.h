#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <memory>
#include <string>

namespace logging
{

    /**
     * @brief Инициализирует систему логирования
     *
     * Создаёт логгер для вывода логов в консоль и файл.
     * Файл лога хранится по пути `logs/<app_name>.log` (относительно приложения)
     *
     * @param app_name Имя приложения (например, "server" или "client")
     * @param log_level Уровень логирования (trace, debug, info, warn, error, critical)
     *
     * @example
     * logging::init("server", spdlog::level::debug);
     */
    void init(const std::string &app_name, spdlog::level::level_enum log_level = spdlog::level::info);

    /**
     * @brief Возвращает глобальный логгер
     *
     * @return Указатель на логгер
     * @throws std::runtime_error если логгер не инициализирован
     *
     * @example
     * logging::get()->info("Server started on port {}", 5555);
     * logging::get()->error("Connection failed: {}", error_msg);
     */
    std::shared_ptr<spdlog::logger> get();

    /**
     * @brief Завершает работу логгера (flush и shutdown)
     *
     * Вызывать перед завершением программы для гарантии записи всех логов
     */
    void shutdown();

} // namespace logging

// Макросы для удобного логирования
#define LOG_TRACE(...) logging::get()->trace(__VA_ARGS__)
#define LOG_DEBUG(...) logging::get()->debug(__VA_ARGS__)
#define LOG_INFO(...) logging::get()->info(__VA_ARGS__)
#define LOG_WARN(...) logging::get()->warn(__VA_ARGS__)
#define LOG_ERROR(...) logging::get()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) logging::get()->critical(__VA_ARGS__)
