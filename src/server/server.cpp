#include "server.h"
#include "logger.h"
#include "net_utils.h"
#include <iostream>
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
        std::cerr << "Error: Invalid integration parameters\n";
        return;
    }

    LOG_INFO("Starting server with parameters: lower={}, upper={}, step={}",
             params.lower_limit, params.upper_limit, params.step);

    std::cout << "\n=== Distributed Integration Server ===\n";
    std::cout << "Integration parameters:\n";
    std::cout << "  Lower limit: " << params.lower_limit << "\n";
    std::cout << "  Upper limit: " << params.upper_limit << "\n";
    std::cout << "  Step: " << params.step << "\n";
    std::cout << "======================================\n\n";

    running_.store(true);

    // Запускаем приём клиентов
    start_accepting_clients();

    // Запускаем обработчик команды START
    input_handler_.start([this]()
                         {
        start_received_.store(true);
        LOG_INFO("START command triggered"); });

    // Ожидаем команды START
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
        LOG_ERROR("No clients connected");
        std::cerr << "\nError: No clients connected. Cannot start integration.\n";
        stop();
        return;
    }

    client_manager_.log_clients_info();

    std::cout << "\n=== Starting Integration ===\n";

    // Распределяем и отправляем задачи
    if (!distribute_and_send_tasks(params))
    {
        LOG_ERROR("Failed to distribute tasks");
        std::cerr << "Error: Failed to distribute tasks to clients\n";
        stop();
        return;
    }

    // Создаём агрегатор результатов
    size_t total_tasks = task_distributor_.get_total_tasks_count();
    ResultAggregator aggregator(total_tasks);

    // Собираем результаты
    if (!collect_results(aggregator))
    {
        LOG_ERROR("Failed to collect all results");
        std::cerr << "Error: Failed to collect results from all clients\n";
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
        std::cout << "Server listening on port " << port_ << "\n";
        std::cout << "Waiting for clients...\n\n";

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
            tcp::socket socket(io_context_);

            // Устанавливаем таймаут для accept (не блокирующий бесконечно)
            acceptor_->async_accept(socket,
                                    [this, socket = std::move(socket)](const boost::system::error_code &ec) mutable
                                    {
                                        if (!ec && running_.load() && client_manager_.is_accepting())
                                        {
                                            handle_client_connection(std::move(socket));
                                        }
                                    });

            io_context_.run_one();
            io_context_.restart();
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("Error in accept thread: {}", e.what());
        }
    }

    LOG_DEBUG("Accept thread finished");
}

void Server::handle_client_connection(tcp::socket socket)
{
    try
    {
        std::string client_ip = socket.remote_endpoint().address().to_string();
        uint16_t client_port = socket.remote_endpoint().port();

        LOG_INFO("New connection from {}:{}", client_ip, client_port);
        std::cout << "Client connected from " << client_ip << ":" << client_port << "\n";

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

        // Создаём ClientConnection и добавляем в менеджер
        auto connection = std::make_unique<ClientConnection>(
            std::move(socket),
            client_id,
            handshake.system_info);

        client_manager_.add_client(std::move(connection));

        std::cout << "Client registered: ID=" << client_id
                  << ", Cores=" << handshake.system_info.cpu_cores << "\n";
        std::cout << "Total clients: " << client_manager_.get_client_count()
                  << ", Total cores: " << client_manager_.get_total_cpu_cores() << "\n\n";
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Error handling client connection: {}", e.what());
    }
}

void Server::stop_accepting_clients()
{
    LOG_INFO("Stopping acceptance of new clients");
    client_manager_.stop_accepting();

    if (acceptor_ && acceptor_->is_open())
    {
        acceptor_->close();
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

        std::cout << "\nDistributing tasks to " << clients.size() << " client(s)...\n";

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

        std::cout << "All tasks sent successfully\n";
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

        std::cout << "  Client " << client->get_client_id()
                  << ": " << batch.tasks.size() << " tasks sent\n";

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
    std::cout << "\nWaiting for results from clients...\n";

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
        std::cout << "\nWarning: Not all results received within timeout\n";
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

        std::cout << "  Client " << client->get_client_id()
                  << ": results received (" << result_batch.total_time_seconds << "s)\n";

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
    std::cout << "\nSending stop command to clients...\n";

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

    std::cout << "Stop commands sent\n";
}

void Server::print_final_result(double final_result, const IntegrationParameters &params)
{
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "       INTEGRATION COMPLETED\n";
    std::cout << "========================================\n";
    std::cout << std::fixed << std::setprecision(15);
    std::cout << "\nIntegral of 1/ln(x) from " << params.lower_limit
              << " to " << params.upper_limit << ":\n";
    std::cout << "\n  Result = " << final_result << "\n";
    std::cout << "\n========================================\n\n";

    LOG_INFO("Final integration result: {:.15f}", final_result);
}
