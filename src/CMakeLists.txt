# Общая библиотека
add_subdirectory(common)

# Клиентское приложение
if(BUILD_CLIENT)
    add_subdirectory(client)
endif()

# Серверное приложение
if(BUILD_SERVER)
    add_subdirectory(server)
endif()

