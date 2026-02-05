#include <iostream>
#include "utils.h"
#include "net_utils.h"
#include "about.h"

using boost::asio::ip::tcp;

void printWelcomeMessage()
{
    std::cout << "Server for distributed integration of 1/ln(x) (v" << SERVER_VERSION << ")\n";
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
    printWelcomeMessage();

    double lower_limit = askFor(askForVariant::lower_limit);
    double upper_limit = askFor(askForVariant::upper_limit);
    double step = askFor(askForVariant::step);

    try
    {
        boost::asio::io_context io;
        tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 5555));

        std::cout << "Server listening on port 5555\n";

        while (true)
        {
            tcp::socket socket(io);
            acceptor.accept(socket);

            auto remote_ep = socket.remote_endpoint();
            std::cout << "Client connected from "
                      << remote_ep.address().to_string()
                      << ":" << remote_ep.port() << "\n";

            SystemInfo info =
                net_utils::receive_data<SystemInfo>(socket);

            std::cout << info.to_string() << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Server error: " << e.what() << std::endl;
    }
}
