#include <iostream>
#include <thread>
#include <utils.h>
#include <net_utils.h>
#include <about.h>

using boost::asio::ip::tcp;

void printWelcomeMessage()
{
    std::cout << "Client for distributed integration of 1/ln(x) (v" << CLIENT_VERSION << ")\n";
}

int main()
{
    try
    {
        // Контекст ввода/вывода
        boost::asio::io_context io;
        // Создаем сокет
        tcp::socket socket(io);

        socket.connect(
            tcp::endpoint(
                boost::asio::ip::make_address("127.0.0.1"), 
                5555
            )
        );

        SystemInfo info = sys_utils::collect_system_info();

        net_utils::send_data(socket, info);
        
        std::cout << "System info sent to server\n";
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << "\n";
    }
}