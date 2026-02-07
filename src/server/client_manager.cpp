#include "client_manager.h"
#include "logger.h"
#include <algorithm>

ClientManager::ClientManager()
{
    LOG_INFO("ClientManager initialized");
}

uint64_t ClientManager::add_client(std::unique_ptr<ClientConnection> connection)
{
    if (!accepting_.load())
    {
        uint64_t client_id = next_client_id_++;
        LOG_WARN("Attempt to add client while not accepting new connections");
        return 0;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    uint64_t client_id = connection->get_client_id();

    LOG_INFO("Adding client: ID={}, IP={}, Cores={}",
             client_id,
             connection->get_ip_address(),
             connection->get_cpu_cores());

    clients_.push_back(std::move(connection));

    LOG_INFO("Total clients: {}, Total CPU cores: {}",
             clients_.size(),
             get_total_cpu_cores());

    return client_id;
}

size_t ClientManager::get_client_count() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return clients_.size();
}

uint32_t ClientManager::get_total_cpu_cores() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    uint32_t total = 0;
    for (const auto &client : clients_)
    {
        total += client->get_cpu_cores();
    }
    return total;
}

ClientConnection *ClientManager::get_client(uint64_t client_id)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::find_if(clients_.begin(), clients_.end(),
                           [client_id](const std::unique_ptr<ClientConnection> &client)
                           {
                               return client->get_client_id() == client_id;
                           });

    if (it != clients_.end())
    {
        return it->get();
    }
    return nullptr;
}

std::vector<ClientConnection *> ClientManager::get_all_clients()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ClientConnection *> result;
    result.reserve(clients_.size());

    for (auto &client : clients_)
    {
        result.push_back(client.get());
    }

    return result;
}

bool ClientManager::remove_client(uint64_t client_id)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::find_if(clients_.begin(), clients_.end(),
                           [client_id](const std::unique_ptr<ClientConnection> &client)
                           {
                               return client->get_client_id() == client_id;
                           });

    if (it != clients_.end())
    {
        LOG_INFO("Removing client: ID={}", client_id);
        clients_.erase(it);
        return true;
    }

    LOG_WARN("Client ID={} not found for removal", client_id);
    return false;
}

void ClientManager::clear()
{
    std::lock_guard<std::mutex> lock(mutex_);
    LOG_INFO("Clearing all clients (count: {})", clients_.size());
    clients_.clear();
}

void ClientManager::stop_accepting()
{
    accepting_.store(false);
    LOG_INFO("Stopped accepting new clients");
}

void ClientManager::log_clients_info() const
{
    std::lock_guard<std::mutex> lock(mutex_);

    LOG_INFO("=== Connected Clients ===");
    LOG_INFO("Total clients: {}", clients_.size());
    LOG_INFO("Total CPU cores: {}", get_total_cpu_cores());

    for (size_t i = 0; i < clients_.size(); ++i)
    {
        const auto &client = clients_[i];
        LOG_INFO("  [{}] ID={}, IP={}:{}, OS={}, Cores={}",
                 i + 1,
                 client->get_client_id(),
                 client->get_ip_address(),
                 client->get_port(),
                 to_string(client->get_system_info().os_type),
                 client->get_cpu_cores());
    }
    LOG_INFO("========================");
}
