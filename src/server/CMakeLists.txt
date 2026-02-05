# Исполняемый файл сервера
add_executable(server
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

# Указываем, где искать заголовочные файлы
target_include_directories(server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Линкуем с библиотекой common
target_link_libraries(server PRIVATE common)