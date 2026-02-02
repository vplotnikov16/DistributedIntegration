# Исполняемый файл клиента
add_executable(client
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

# Линкуем с библиотекой common
target_link_libraries(client PRIVATE common)
