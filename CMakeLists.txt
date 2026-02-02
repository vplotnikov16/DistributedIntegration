cmake_minimum_required(VERSION 3.30)
project("DistributedIntegration")

# Настройка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_CLIENT "Build client application" ON)
option(BUILD_SERVER "Build server application" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src)
