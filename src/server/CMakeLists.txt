# Исполняемый файл сервера
add_executable(server
    about.h
    client_connection.cpp
    client_connection.h
    client_manager.cpp
    client_manager.h
    input_handler.cpp
    input_handler.h
    main.cpp
    result_aggregator.cpp
    result_aggregator.h
    server.cpp
    server.h
    task_distributor.cpp
    task_distributor.h
)

# Указываем, где искать заголовочные файлы
target_include_directories(server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Линкуем с библиотекой common
target_link_libraries(server PRIVATE common)

# Установка RPATH для Linux/macOS
if(UNIX)
    set_target_properties(server PROPERTIES
        INSTALL_RPATH "$ORIGIN/../lib"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()
