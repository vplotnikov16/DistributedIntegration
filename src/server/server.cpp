#include "server.h"
#include "logger.h"
#include "net_utils.h"
#include <iomanip>
#include <chrono>

Server::Server(uint16_t port)
    : port_(port)
{
    LOG_INFO("Server initialized on port {}", port_);
}

Server::~Server()
{
    stop();
}

void Server::run(const IntegrationParameters &params)
{
    // Валидация параметров
    if (!params.is_valid())
    {
        LOG_ERROR("Invalid integration parameters");
        return;
    }

    LOG_INFO("Starting server with parameters: lower={}, upper={}, step={}",
             params.lower_limit, params.upper_limit, params.step);

    LOG_INFO("=== Distributed Integration Server ===");
    LOG_INFO("Integration parameters:");
    LOG_INFO("  Lower limit: {}", params.lower_limit);
    LOG_INFO("  Upper limit: {}", params.upper_limit);
    LOG_INFO("  Step: {}", params.step);
    LOG_INFO("======================================");

    running_.store(true);

    // Запускаем приём клиентов
    start_accepting_clients();

    // Запускаем обработчик команды START
    input_handler_.start([this]()
                         {
        start_received_.store(true);
        LOG_INFO("START command triggered"); });

    // Ожидаем команды START
    LOG_INFO("Waiting for clients to connect...");
    LOG_INFO("Type 'START' and press Enter to begin integration");

    while (!start_received_.load() && running_.load())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (!running_.load())
    {
        LOG_INFO("Server stopped before START command");
        return;
    }

    // Останавливаем приём новых клиентов
    stop_accepting_clients();

    // Проверяем наличие клиентов
    if (client_manager_.get_client_count() == 0)
    {
        LOG_ERROR("No clients connected. Cannot start integration.");
        stop();
        return;
    }

    client_manager_.log_clients_info();

    LOG_INFO("=== Starting Integration ===");

    // Распределяем и отправляем задачи
    if (!distribute_and_send_tasks(params))
    {
        LOG_ERROR("Failed to distribute tasks to clients");
        stop();
        return;
    }

    // Создаём агрегатор результатов
    size_t total_tasks = task_distributor_.get_total_tasks_count();
    ResultAggregator aggregator(total_tasks);

    // Собираем результаты
    if (!collect_results(aggregator))
    {
        LOG_ERROR("Failed to collect results from all clients");
        stop();
        return;
    }

    // Получаем итоговый результат
    double final_result = aggregator.get_final_result();
    aggregator.log_results_info();

    // Выводим результат
    print_final_result(final_result, params);

    // Отправляем команду завершения работы клиентам
    send_stop_command_to_all_clients();

    // Завершаем работу
    stop();
}

void Server::stop()
{
    if (!running_.load())
    {
        return;
    }

    LOG_INFO("Stopping server...");
    running_.store(false);

    input_handler_.stop();
    stop_accepting_clients();
    client_manager_.clear();

    LOG_INFO("Server stopped");
}

void Server::start_accepting_clients()
{
    try
    {
        acceptor_ = std::make_unique<tcp::acceptor>(
            io_context_,
            tcp::endpoint(tcp::v4(), port_));

        LOG_INFO("Server listening on port {}", port_);
        LOG_INFO("Waiting for clients...");

        accept_thread_ = std::thread(&Server::accept_thread_func, this);
    }
    catch (const boost::system::system_error &e)
    {
        LOG_ERROR("Failed to start acceptor: {}", e.what());
        throw;
    }
}

void Server::accept_thread_func()
{
    LOG_DEBUG("Accept thread started");

    while (running_.load() && client_manager_.is_accepting())
    {
        try
        {
            auto socket_ptr = std::make_shared<tcp::socket>(io_context_);

            // Синхронный accept
            acceptor_->accept(*socket_ptr);

            if (running_.load() && client_manager_.is_accepting())
            {
                // Получаем endpoint ЗДЕСЬ, пока сокет гарантированно валиден
                std::string client_ip = "unknown";
                uint16_t client_port = 0;

                try
                {
                    if (socket_ptr->is_open())
                    {
                        auto endpoint = socket_ptr->remote_endpoint();
                        client_ip = endpoint.address().to_string();
                        client_port = endpoint.port();
                    }
                }
                catch (const std::exception &e)
                {
                    LOG_WARN("Failed to get remote endpoint: {}", e.what());
                }

                // Запускаем обработку в отдельном потоке
                std::thread([this, socket_ptr, client_ip, client_port]() {
                    handle_client_connection(std::move(*socket_ptr), client_ip, client_port);
                }).detach();
            }
        }
        catch (const boost::system::system_error &e)
        {
            if (e.code() == boost::asio::error::operation_aborted ||
                e.code().value() == 10004) // WSA_INTERRUPTED
            {
                // Acceptor был закрыт - это нормально при остановке
                LOG_DEBUG("Accept operation aborted (normal shutdown)");
                break;
            }
            LOG_ERROR("Error in accept: {}", e.what());
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("Error in accept thread: {}", e.what());
        }
    }

    LOG_DEBUG("Accept thread finished");
}

void Server::handle_client_connection(tcp::socket socket, 
                                      const std::string &client_ip, 
                                      uint16_t client_port)
{
    try
    {
        LOG_INFO("New connection from {}:{}", client_ip, client_port);

        // Получаем HandshakeRequest от клиента
        auto handshake = net_utils::receive_data<HandshakeRequest>(socket);

        LOG_INFO("Handshake received: version={}, OS={}, cores={}",
                 handshake.client_version,
                 to_string(handshake.system_info.os_type),
                 handshake.system_info.cpu_cores);

        // Генерируем ID для клиента
        static std::atomic<uint64_t> next_id{1};
        uint64_t client_id = next_id++;

        // Отправляем HandshakeResponse
        HandshakeResponse response;
        response.assigned_client_id = client_id;
        response.server_version = "1.0.0";
        response.accepted = true;
        response.message = "Connection accepted";

        net_utils::send_data(socket, response);

        LOG_INFO("Handshake completed for client {}", client_id);

        // Создаём ClientConnection и добавляем в менеджер
        auto connection = std::make_unique<ClientConnection>(
            std::move(socket),
            client_id,
            handshake.system_info);

        client_manager_.add_client(std::move(connection));

        LOG_INFO("Client registered: ID={}, Cores={}", 
                 client_id, handshake.system_info.cpu_cores);
        LOG_INFO("Total clients: {}, Total cores: {}", 
                 client_manager_.get_client_count(),
                 client_manager_.get_total_cpu_cores());
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Error handling client connection from {}:{}: {}", 
                  client_ip, client_port, e.what());
    }
}

void Server::stop_accepting_clients()
{
    LOG_INFO("Stopping acceptance of new clients");
    client_manager_.stop_accepting();

    if (acceptor_ && acceptor_->is_open())
    {
        boost::system::error_code ec;
        acceptor_->close(ec);
        if (ec)
        {
            LOG_WARN("Error closing acceptor: {}", ec.message());
        }
    }

    if (accept_thread_.joinable())
    {
        accept_thread_.join();
    }

    LOG_INFO("Client acceptance stopped");
}

bool Server::distribute_and_send_tasks(const IntegrationParameters &params)
{
    try
    {
        auto clients = client_manager_.get_all_clients();

        // Распределяем задачи
        auto task_map = task_distributor_.distribute_tasks(
            clients,
            params.lower_limit,
            params.upper_limit,
            params.step);

        LOG_INFO("Distributing tasks to {} client(s)...", clients.size());

        // Отправляем задачи каждому клиенту
        for (auto *client : clients)
        {
            auto it = task_map.find(client->get_client_id());
            if (it != task_map.end())
            {
                if (!send_tasks_to_client(client, it->second))
                {
                    LOG_ERROR("Failed to send tasks to client {}", client->get_client_id());
                    return false;
                }
            }
        }

        LOG_INFO("All tasks sent successfully");
        return true;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Error distributing tasks: {}", e.what());
        return false;
    }
}

bool Server::send_tasks_to_client(ClientConnection *client, const TaskBatch &batch)
{
    try
    {
        LOG_INFO("Sending {} tasks to client {}", batch.tasks.size(), client->get_client_id());

        net_utils::send_data(client->get_socket(), batch);
        client->mark_task_sent();

        LOG_INFO("Client {}: {} tasks sent", client->get_client_id(), batch.tasks.size());

        return true;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Failed to send tasks to client {}: {}",
                  client->get_client_id(), e.what());
        return false;
    }
}

bool Server::collect_results(ResultAggregator &aggregator)
{
    LOG_INFO("Waiting for results from clients...");

    auto clients = client_manager_.get_all_clients();

    // Запускаем получение результатов от каждого клиента в отдельных потоках
    std::vector<std::thread> receive_threads;
    receive_threads.reserve(clients.size());

    for (auto *client : clients)
    {
        receive_threads.emplace_back([this, client, &aggregator]()
                                     { receive_results_from_client(client, aggregator); });
    }

    // Ожидаем завершения всех потоков
    for (auto &thread : receive_threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

    // Ожидаем получения всех результатов с таймаутом 300 секунд
    bool all_received = aggregator.wait_for_all_results(300);

    if (!all_received)
    {
        LOG_WARN("Not all results received within timeout");
    }

    return all_received;
}

bool Server::receive_results_from_client(ClientConnection *client, ResultAggregator &aggregator)
{
    try
    {
        LOG_INFO("Waiting for results from client {}", client->get_client_id());

        auto result_batch = net_utils::receive_data<ResultBatch>(client->get_socket());
        client->mark_result_received();

        LOG_INFO("Received {} results from client {} (time: {:.3f}s)",
                 result_batch.results.size(),
                 client->get_client_id(),
                 result_batch.total_time_seconds);

        aggregator.add_result(result_batch);

        LOG_INFO("Client {}: results received ({:.3f}s)", 
                 client->get_client_id(), 
                 result_batch.total_time_seconds);

        return true;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Failed to receive results from client {}: {}",
                  client->get_client_id(), e.what());
        return false;
    }
}

void Server::send_stop_command_to_all_clients()
{
    LOG_INFO("Sending STOP command to all clients");

    auto clients = client_manager_.get_all_clients();

    Command stop_cmd;
    stop_cmd.type = CommandType::STOP_WORK;
    stop_cmd.message = "Integration completed";

    for (auto *client : clients)
    {
        try
        {
            net_utils::send_data(client->get_socket(), stop_cmd);
            LOG_DEBUG("STOP command sent to client {}", client->get_client_id());
        }
        catch (const std::exception &e)
        {
            LOG_WARN("Failed to send STOP command to client {}: {}",
                     client->get_client_id(), e.what());
        }
    }

    LOG_INFO("Stop commands sent to all clients");
}

void Server::print_final_result(double final_result, const IntegrationParameters &params)
{
    LOG_INFO("========================================");
    LOG_INFO("       INTEGRATION COMPLETED");
    LOG_INFO("========================================");
    LOG_INFO("Integral of 1/ln(x) from {} to {}", params.lower_limit, params.upper_limit);
    LOG_INFO("Result = {:.15f}", final_result);
    LOG_INFO("========================================");
}
