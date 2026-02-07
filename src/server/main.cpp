#include <iostream>
#include "utils.h"
#include "net_utils.h"
#include "about.h"
#include "logger.h"

using boost::asio::ip::tcp;

void printWelcomeMessage()
{
    LOG_INFO("Server for distributed integration of 1/ln(x)");
    LOG_INFO("Version: {}", SERVER_VERSION);
}

enum askForVariant
{
    lower_limit,
    upper_limit,
    step
};

double askFor(askForVariant variant)
{
    double result;

    switch (variant)
    {
    case lower_limit:
        std::cout << "Enter a lower integration limit: ";
        break;
    case upper_limit:
        std::cout << "Enter an upper integration limit: ";
        break;
    case step:
        std::cout << "Enter integration step size: ";
        break;

    default:
        break;
    }

    while (!(std::cin >> result))
    {
        std::cout << "Invalid input. Please enter a number: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::cout << "\n";
    return result;
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

    double lower_limit = askFor(askForVariant::lower_limit);
    double upper_limit = askFor(askForVariant::upper_limit);
    double step = askFor(askForVariant::step);

    LOG_INFO("Integration parameters:");
    LOG_INFO("  Lower limit: {}", lower_limit);
    LOG_INFO("  Upper limit: {}", upper_limit);
    LOG_INFO("  Step: {}", step);

    try
    {
        boost::asio::io_context io;
        tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 5555));

        LOG_INFO("Server listening on port 5555");

        while (true)
        {
            tcp::socket socket(io);
            acceptor.accept(socket);

            auto remote_ep = socket.remote_endpoint();
            LOG_INFO("Client connected from {}:{}",
                     remote_ep.address().to_string(),
                     remote_ep.port());

            SystemInfo info = net_utils::receive_data<SystemInfo>(socket);

            LOG_DEBUG("Received SystemInfo:");
            LOG_DEBUG("  OS: {}", to_string(info.os_type));
            LOG_DEBUG("  Architecture: {}", to_string(info.architecture));
            LOG_DEBUG("  CPU cores: {}", info.cpu_cores);
            LOG_DEBUG("  RAM: {} MB", info.total_ram_mb);
        }
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Server error: {}", e.what());
    }

    logging::shutdown();
    return 0;
}
