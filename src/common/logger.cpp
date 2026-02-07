#include "logger.h"
#include <spdlog/async.h>
#include <filesystem>
#include <stdexcept>

namespace logging
{

    // Глобальный логгер
    static std::shared_ptr<spdlog::logger> g_logger = nullptr;

    void init(const std::string &app_name, spdlog::level::level_enum log_level)
    {
        try
        {
            // Создаём директорию для логов, если её нет
            std::filesystem::create_directories("logs");

            // Создаём sinks (куда писать логи)
            std::vector<spdlog::sink_ptr> sinks;

            // 1. Console sink (цветной вывод в консоль)
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(log_level);
            console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");
            sinks.push_back(console_sink);

            // 2. File sink с ротацией (максимум 5 МБ, 3 файла)
            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                "logs/" + app_name + ".log",
                1024 * 1024 * 5, // 5 МБ
                3                // 3 файла
            );

            // В файл пишем всё
            file_sink->set_level(spdlog::level::trace);
            file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] [thread %t] %v");
            sinks.push_back(file_sink);

            // Создаём логгер
            g_logger = std::make_shared<spdlog::logger>(app_name, sinks.begin(), sinks.end());
            g_logger->set_level(log_level);
             // Автоматический flush на warn и выше
            g_logger->flush_on(spdlog::level::warn);

            // Регистрируем как глобальный логгер
            spdlog::register_logger(g_logger);
            spdlog::set_default_logger(g_logger);

            LOG_INFO("Logger initialized for '{}'", app_name);
            LOG_INFO("Log level: {}", spdlog::level::to_string_view(log_level));
            LOG_DEBUG("Log file: logs/{}.log", app_name);
        }
        catch (const std::exception &e)
        {
            throw std::runtime_error(std::string("Failed to initialize logger: ") + e.what());
        }
    }

    std::shared_ptr<spdlog::logger> get()
    {
        if (!g_logger)
        {
            throw std::runtime_error("Logger not initialized. Call logging::init() first.");
        }
        return g_logger;
    }

    void shutdown()
    {
        if (g_logger)
        {
            LOG_INFO("Shutting down logger");
            g_logger->flush();
            spdlog::shutdown();
            g_logger = nullptr;
        }
    }

} // namespace logging
