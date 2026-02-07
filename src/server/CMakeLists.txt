# Исполняемый файл сервера
add_executable(server
    main.cpp
    about.h
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
