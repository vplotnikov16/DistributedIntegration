#include "network_manager.h"
#include "net_utils.h"
#include "logger.h"
#include <stdexcept>

NetworkManager::NetworkManager(const std::string &server_address, uint16_t server_port)
    : server_address_(server_address),
      server_port_(server_port),
      io_context_(),
      socket_(nullptr),
      connected_(false)
{
    LOG_DEBUG("NetworkManager created for server {}:{}", server_address_, server_port_);
}

NetworkManager::~NetworkManager()
{
    disconnect();
}

void NetworkManager::connect()
{
    if (connected_)
    {
        LOG_WARN("Already connected to server");
        return;
    }

    try
    {
        LOG_INFO("Connecting to server {}:{}...", server_address_, server_port_);

        // Создаём новый сокет
        socket_ = std::make_unique<tcp::socket>(io_context_);

        // Резолвим адрес
        tcp::resolver resolver(io_context_);
        auto endpoints = resolver.resolve(server_address_, std::to_string(server_port_));

        // Подключаемся
        boost::asio::connect(*socket_, endpoints);

        connected_ = true;
        LOG_INFO("Successfully connected to server {}:{}", server_address_, server_port_);
    }
    catch (const boost::system::system_error &e)
    {
        LOG_ERROR("Failed to connect to server: {}", e.what());
        connected_ = false;
        socket_.reset();
        throw std::runtime_error(
            std::string("Connection failed: ") + e.what());
    }
}

bool NetworkManager::is_connected() const
{
    return connected_ && socket_ && socket_->is_open();
}

HandshakeResponse NetworkManager::perform_handshake(
    const std::string &client_version,
    const SystemInfo &system_info)
{
    if (!is_connected())
    {
        throw std::runtime_error("Not connected to server");
    }

    LOG_INFO("Performing handshake with server...");

    try
    {
        // Создаём запрос
        HandshakeRequest request;
        request.client_version = client_version;
        request.system_info = system_info;

        // Отправляем запрос
        net_utils::send_data(*socket_, request);
        LOG_DEBUG("Handshake request sent");

        // Получаем ответ
        HandshakeResponse response = net_utils::receive_data<HandshakeResponse>(*socket_);

        if (!response.accepted)
        {
            LOG_ERROR("Handshake rejected by server: {}", response.message);
            throw std::runtime_error("Handshake rejected: " + response.message);
        }

        LOG_INFO("Handshake successful. Assigned client_id: {}, server version: {}",
                 response.assigned_client_id,
                 response.server_version);

        return response;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Handshake failed: {}", e.what());
        throw std::runtime_error(std::string("Handshake failed: ") + e.what());
    }
}

TaskBatch NetworkManager::receive_tasks()
{
    if (!is_connected())
    {
        throw std::runtime_error("Not connected to server");
    }

    LOG_INFO("Waiting for tasks from server...");

    try
    {
        TaskBatch batch = net_utils::receive_data<TaskBatch>(*socket_);
        LOG_INFO("Received {} tasks from server", batch.tasks.size());
        return batch;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Failed to receive tasks: {}", e.what());
        throw std::runtime_error(std::string("Failed to receive tasks: ") + e.what());
    }
}

void NetworkManager::send_results(const ResultBatch &results)
{
    if (!is_connected())
    {
        throw std::runtime_error("Not connected to server");
    }

    LOG_INFO("Sending {} results to server...", results.results.size());

    try
    {
        net_utils::send_data(*socket_, results);
        LOG_INFO("Results sent successfully");
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Failed to send results: {}", e.what());
        throw std::runtime_error(std::string("Failed to send results: ") + e.what());
    }
}

Command NetworkManager::receive_command()
{
    if (!is_connected())
    {
        throw std::runtime_error("Not connected to server");
    }

    LOG_DEBUG("Waiting for command from server...");

    try
    {
        Command cmd = net_utils::receive_data<Command>(*socket_);
        LOG_DEBUG("Received command: type={}", static_cast<int>(cmd.type));
        return cmd;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Failed to receive command: {}", e.what());
        throw std::runtime_error(std::string("Failed to receive command: ") + e.what());
    }
}

void NetworkManager::send_command(const Command &command)
{
    if (!is_connected())
    {
        throw std::runtime_error("Not connected to server");
    }

    LOG_DEBUG("Sending command to server: type={}", static_cast<int>(command.type));

    try
    {
        net_utils::send_data(*socket_, command);
        LOG_DEBUG("Command sent successfully");
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Failed to send command: {}", e.what());
        throw std::runtime_error(std::string("Failed to send command: ") + e.what());
    }
}

void NetworkManager::disconnect()
{
    if (socket_ && socket_->is_open())
    {
        try
        {
            LOG_INFO("Disconnecting from server...");
            socket_->close();
            LOG_INFO("Disconnected from server");
        }
        catch (const std::exception &e)
        {
            LOG_WARN("Error during disconnect: {}", e.what());
        }
    }

    connected_ = false;
    socket_.reset();
}

std::string NetworkManager::get_server_address() const
{
    return server_address_ + ":" + std::to_string(server_port_);
}
