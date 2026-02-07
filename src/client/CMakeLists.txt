# Исполняемый файл клиента
add_executable(client
    integration_methods/integration_strategy.h
    integration_methods/trapezoidal_rule.h
    about.h
    client.cpp
    client.h
    integrator.cpp
    integrator.h
    main.cpp
    network_manager.cpp
    network_manager.h
    task_executor.cpp
    task_executor.h
    worker_pool.cpp
    worker_pool.h
)

# Указываем, где искать заголовочные файлы
target_include_directories(client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Линкуем с библиотекой common
target_link_libraries(client PRIVATE common)

# Установка RPATH для Linux и macOS
if(UNIX)
    set_target_properties(client PROPERTIES
        INSTALL_RPATH "$ORIGIN/../lib"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()
