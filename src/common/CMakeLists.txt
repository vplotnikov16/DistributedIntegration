# Общие исходники
set(COMMON_SOURCES
    messages.h
    net_utils.h
    systeminfo.h
    utils.cpp
    utils.h
)

# Создаем библиотеку common
add_library(common STATIC ${COMMON_SOURCES})

# Линковка с winsocket2
if (WIN32)
    target_link_libraries(common PRIVATE ws2_32)
endif()

# Линковка с потоками
target_link_libraries(common PUBLIC Threads::Threads)

# Публичные заголовки
target_include_directories(common
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/third_party/boost
    ${CMAKE_SOURCE_DIR}/src/third_party/cereal/include
)

# Определения препроцессора
if(ENABLE_LOGGING)
    target_compile_definitions(common PUBLIC ENABLE_LOGGING)
endif()
