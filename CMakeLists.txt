cmake_minimum_required(VERSION 3.20)
project("DistributedIntegration")

# Настройка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Опции сборки
option(BUILD_CLIENT "Build client application" ON)
option(BUILD_SERVER "Build server application" ON)
option(BUILD_TESTS "Build unit tests" OFF)
option(BUILD_DOCS "Build documentation" OFF)
option(ENABLE_LOGGING "Enable logging" ON)

# Настройка путей вывода
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Поиск потоков
find_package(Threads REQUIRED)

# Определение целевой версии Windows
if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0601) # Windows 7
    add_compile_definitions(_WIN32_WINDOWS=0x0601)
endif()

# Флаги компиляции для MSVC
if(MSVC)
    # Поддержка UTF-8 (требуется для spdlog)
    add_compile_options(/utf-8)

    # Дополнительные полезные флаги
    add_compile_options(/W4) # Уровень предупреждений
    add_compile_options(/permissive-) # Стандартное соответствие
endif()

# Флаги для GCC/Clang
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Каталог с исходным кодом
add_subdirectory(src)

# Опционально: тесты
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
