# Исполняемый файл клиента
add_executable(client
    integration_methods/integration_strategy.h
    integration_methods/trapezoidal_rule.h
    about.h
    integrator.cpp
    integrator.h
    main.cpp
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
