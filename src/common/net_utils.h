#include "boost/asio.hpp"

namespace net_utils
{
    template <typename T>
    void send_data(boost::asio::ip::tcp::socket &socket, const T &data)
    {
        static_assert(std::is_trivially_copyable_v<T>, "send_data requires trivially copyable type");
        boost::asio::write(socket, boost::asio::buffer(&data, sizeof(data)));
    }

    template <typename T>
    T receive_data(boost::asio::ip::tcp::socket &socket)
    {
        static_assert(std::is_trivially_copyable_v<T>, "receive_data requires trivially copyable type");
        T data;
        boost::asio::read(socket, boost::asio::buffer(&data, sizeof(data)));
        return data;
    }
}
