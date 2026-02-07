#include "input_handler.h"
#include "logger.h"
#include <iostream>
#include <algorithm>
#include <cctype>

InputHandler::~InputHandler()
{
    stop();
}

void InputHandler::start(StartCallback on_start)
{
    if (running_.load())
    {
        LOG_WARN("InputHandler already running");
        return;
    }

    on_start_callback_ = std::move(on_start);
    stop_requested_.store(false);
    running_.store(true);

    input_thread_ = std::thread(&InputHandler::input_thread_func, this);

    LOG_INFO("InputHandler started, waiting for 'START' command...");
}

void InputHandler::stop()
{
    if (!running_.load())
    {
        return;
    }

    LOG_INFO("Stopping InputHandler...");

    stop_requested_.store(true);
    running_.store(false);

    LOG_INFO("InputHandler stopped");
}

void InputHandler::input_thread_func()
{
    LOG_DEBUG("Input thread started");

    std::cout << "\n========================================\n";
    std::cout << "Waiting for clients to connect...\n";
    std::cout << "Type 'START' and press Enter to begin integration\n";
    std::cout << "========================================\n\n";

    while (!stop_requested_.load())
    {
        std::string input;
        std::cout << "> ";
        std::getline(std::cin, input);

        if (stop_requested_.load())
        {
            break;
        }

        // Убираем пробелы и приводим к верхнему регистру
        input.erase(std::remove_if(input.begin(), input.end(), ::isspace), input.end());
        std::transform(input.begin(), input.end(), input.begin(), ::toupper);

        if (input == "START")
        {
            LOG_INFO("START command received");

            if (on_start_callback_)
            {
                on_start_callback_();
            }

            // После команды START прекращаем ожидание
            break;
        }
        else if (!input.empty())
        {
            std::cout << "Unknown command: '" << input << "'. Type 'START' to begin.\n";
        }
    }

    running_.store(false);
    LOG_DEBUG("Input thread finished");
}
