#include "net_utils.h"
using tcp = boost::asio::ip::tcp;

namespace net_utils
{
    std::string get_remote_address(tcp::socket &socket)
    {
        try
        {
            if (socket.is_open())
            {
                return socket.remote_endpoint().address().to_string();
            }
        }
        catch (const boost::system::system_error &)
        {
            // Игнорируем ошибку
        }
        return "unknown";
    }

    uint16_t get_port(tcp::socket &socket)
    {
        try
        {
            if (socket.is_open())
            {
                return socket.remote_endpoint().port();
            }
        }
        catch (const boost::system::system_error &)
        {
            // Игнорируем ошибку
        }
        return 0;
    }
}
