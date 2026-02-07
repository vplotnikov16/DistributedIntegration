#include <iostream>
#include "utils.h"
#include "net_utils.h"
#include "about.h"
#include "logger.h"
#include "server.h"

using boost::asio::ip::tcp;

void printWelcomeMessage()
{
    LOG_INFO("Server for distributed integration of 1/ln(x)");
    LOG_INFO("Version: {}", SERVER_VERSION);
}

/**
 * @brief Запрашивает у пользователя параметр интегрирования
 * @param prompt Текст запроса
 * @return Введенное значение
 */
double askFor(const std::string &prompt)
{
    double result;

    while (true)
    {
        std::cout << prompt;

        if (std::cin >> result)
        {
            // Очищаем буфер после успешного ввода
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return result;
        }
        else
        {
            std::cout << "Invalid input. Please enter a number.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

int main()
{
    // Инициализация логгера
    try
    {
        logging::init("server", spdlog::level::debug);
    }
    catch (const std::exception &e)
    {
        // Если логгер не инициализирован, то пишем об этом и завершаем работу
        std::cerr << "Failed to initialize logger: " << e.what() << std::endl;
        return 1;
    }

    printWelcomeMessage();

    IntegrationParameters params;
    params.lower_limit = askFor("  Lower limit (x > 0, x != 1): ");
    params.upper_limit = askFor("  Upper limit (x > lower): ");
    params.step = askFor("  Integration step: ");

    if (!params.is_valid())
    {
        LOG_ERROR("Invalid integration parameters provided");
        logging::shutdown();
        return 1;
    }

    try
    {
        const uint16_t PORT = 5555;
        Server server(PORT);
        server.run(params);

        logging::shutdown();
        return 0;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Server error: {}", e.what());
    }

    logging::shutdown();
    return 0;
}
