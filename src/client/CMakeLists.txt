# Исполняемый файл клиента
add_executable(client
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

# Указываем, где искать заголовочные файлы
target_include_directories(client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Линкуем с библиотекой common
target_link_libraries(client PRIVATE common)
