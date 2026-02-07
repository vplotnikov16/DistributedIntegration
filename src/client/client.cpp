#include "client.h"
#include "about.h"
#include "utils.h"
#include "logger.h"
#include "integration_methods/trapezoidal_rule.h"
#include <chrono>
#include <stdexcept>

Client::Client(const std::string &server_address,
               uint16_t server_port)
    : client_id_(0),
      system_info_(),
      network_manager_(nullptr),
      integrator_(nullptr),
      task_executor_(nullptr),
      worker_pool_(nullptr)
{
    LOG_INFO("Client v{} initializing...", CLIENT_VERSION);

    // Создаём network manager
    network_manager_ = std::make_unique<NetworkManager>(server_address, server_port);

    // Собираем информацию о системе
    system_info_ = sys_utils::collect_system_info();
    LOG_INFO("System info collected: {} cores, {} MB RAM",
             system_info_.cpu_cores, system_info_.total_ram_mb);

    // Создаём интегратор с методом трапеций по умолчанию
    auto strategy = std::make_unique<TrapezoidalRule>();
    integrator_ = std::make_shared<Integrator>(std::move(strategy));
    LOG_INFO("Integration method: {}", integrator_->get_current_method());

    // Создаём task executor
    task_executor_ = std::make_unique<TaskExecutor>(integrator_);

    // Создаём worker pool
    worker_pool_ = std::make_unique<WorkerPool>(system_info_.cpu_cores, integrator_);

    LOG_INFO("Client initialized successfully");
}

Client::~Client()
{
    LOG_INFO("Client shutting down...");

    if (network_manager_)
    {
        network_manager_->disconnect();
    }

    LOG_INFO("Client shutdown complete");
}

void Client::run()
{
    try
    {
        // 1. Подключение к серверу
        LOG_INFO("=== STEP 1: Connecting to server ===");
        network_manager_->connect();

        // 2. Handshake
        LOG_INFO("=== STEP 2: Performing handshake ===");
        HandshakeResponse handshake = network_manager_->perform_handshake(
            client_version_,
            system_info_);

        client_id_ = handshake.assigned_client_id;
        LOG_INFO("Assigned client ID: {}", client_id_);

        // 3. Получение задач
        LOG_INFO("=== STEP 3: Waiting for tasks ===");
        TaskBatch task_batch = network_manager_->receive_tasks();
        LOG_INFO("Received {} tasks", task_batch.tasks.size());

        if (task_batch.tasks.empty())
        {
            LOG_WARN("No tasks received, exiting");
            return;
        }

        // 4. Выполнение задач
        LOG_INFO("=== STEP 4: Executing tasks ===");
        auto start_time = std::chrono::high_resolution_clock::now();

        std::vector<Result> results = execute_tasks(task_batch.tasks);

        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end_time - start_time;

        LOG_INFO("All tasks completed in {:.3f} seconds", elapsed.count());

        // 5. Отправка результатов
        LOG_INFO("=== STEP 5: Sending results ===");
        ResultBatch result_batch;
        result_batch.client_id = client_id_;
        result_batch.results = results;
        result_batch.total_time_seconds = elapsed.count();

        network_manager_->send_results(result_batch);
        LOG_INFO("Results sent successfully");

        // 6. Ожидание команды STOP_WORK
        LOG_INFO("=== STEP 6: Waiting for STOP_WORK command ===");
        Command cmd = network_manager_->receive_command();

        if (cmd.type == CommandType::STOP_WORK)
        {
            LOG_INFO("Received STOP_WORK command: {}", cmd.message);
        }
        else
        {
            LOG_WARN("Unexpected command received: {}", static_cast<int>(cmd.type));
        }

        // 7. Завершение
        LOG_INFO("=== STEP 7: Shutting down ===");
        network_manager_->disconnect();

        LOG_INFO("Client finished successfully");
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("Client error: {}", e.what());
        throw;
    }
}

void Client::set_integration_strategy(std::unique_ptr<IIntegrationStrategy> strategy)
{
    if (!strategy)
    {
        throw std::invalid_argument("Strategy cannot be nullptr");
    }

    integrator_->set_strategy(std::move(strategy));
    LOG_INFO("Integration strategy changed to: {}", integrator_->get_current_method());
}

SystemInfo Client::collect_system_info()
{
    return sys_utils::collect_system_info();
}

std::vector<Result> Client::execute_tasks(const std::vector<Task> &tasks)
{
    LOG_INFO("Executing {} tasks using {} threads...",
             tasks.size(), system_info_.cpu_cores);

    // Используем worker pool для параллельного выполнения
    return worker_pool_->execute_tasks_parallel(tasks);
}
