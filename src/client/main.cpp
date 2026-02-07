#include <iostream>
#include <thread>
#include "utils.h"
#include "net_utils.h"
#include "about.h"
#include "logger.h"
#include "client.h"

using boost::asio::ip::tcp;

void printWelcomeMessage()
{
    LOG_INFO("Client for distributed integration of 1/ln(x)");
    LOG_INFO("Version: {}", CLIENT_VERSION);
}

int main(int argc, char *argv[])
{
    // Инициализация логгера
    try
    {
        logging::init("client", spdlog::level::debug);
    }
    catch (const std::exception &e)
    {
        // Если логгер не инициализирован, то пишем об этом и завершаем работу
        std::cerr << "Failed to initialize logger: " << e.what() << std::endl;
        return 1;
    }

    printWelcomeMessage();

    if (argc != 3)
    {
        LOG_ERROR("Usage: {} <ip> <port>", argv[0]);
        return 1;
    }

    std::string server_address = argv[1];
    uint16_t server_port = static_cast<uint16_t>(std::atoi(argv[2]));;

    try
    {
        LOG_DEBUG("Collecting system information...");
        SystemInfo info = sys_utils::collect_system_info();

        LOG_INFO("System information:");
        LOG_INFO("  OS: {}", to_string(info.os_type));
        LOG_INFO("  Architecture: {}", to_string(info.architecture));
        LOG_INFO("  CPU cores: {}", info.cpu_cores);
        LOG_INFO("  RAM: {} MB", info.total_ram_mb);

        Client client(server_address, server_port);
        client.run();

        logging::shutdown();
        
        LOG_INFO("Client finished");
        return 0;
    }
    catch (std::exception &e)
    {
        LOG_ERROR("Client error: {}", e.what());
        std::cerr << e.what() << "\n";
        logging::shutdown();
        return 1;
    }
    logging::shutdown();
    return 0;
}