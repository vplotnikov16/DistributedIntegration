#include "client_connection.h"
#include "logger.h"
#include <net_utils.h>

ClientConnection::ClientConnection(
    tcp::socket socket,
    uint64_t client_id,
    const SystemInfo &system_info)
    : socket_(std::move(socket)), client_id_(client_id), system_info_(system_info)
{
    LOG_DEBUG("ClientConnection created: ID={}, IP={}, Cores={}",
              client_id_, get_ip_address(), system_info_.cpu_cores);
}

ClientConnection::~ClientConnection()
{
    close();
    LOG_DEBUG("ClientConnection destroyed: ID={}", client_id_);
}

std::string ClientConnection::get_ip_address() const
{
    std::string result = net_utils::get_remote_address(socket_);
    if (result == "unknown")
        LOG_WARN("Failed to get IP address for client ID={}", client_id_);
    return result;
}

uint16_t ClientConnection::get_port() const
{
    uint16_t result = net_utils::get_port(socket_);
    if (result == 0)
        LOG_WARN("Failed to get port for client ID={}", client_id_);
    return result;
}

bool ClientConnection::is_connected() const
{
    return socket_.is_open();
}

void ClientConnection::close()
{
    if (socket_.is_open())
    {
        try
        {
            LOG_DEBUG("Closing connection for client ID={}", client_id_);
            socket_.close();
        }
        catch (const boost::system::system_error &e)
        {
            LOG_ERROR("Error closing socket for client {}: {}",
                      client_id_, e.what());
        }
    }
}
