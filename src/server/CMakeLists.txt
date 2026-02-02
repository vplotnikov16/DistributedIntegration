# Исполняемый файл сервера
add_executable(server
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

# Линкуем с библиотекой common
target_link_libraries(server PRIVATE common)