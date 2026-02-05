set(COMMON_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils.h
)
# Создаем библиотеку common
add_library(common STATIC ${COMMON_SOURCES})

if (WIN32)
    target_link_libraries(common PRIVATE ws2_32)
endif()

# Публичные заголовки
target_include_directories(common
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/third_party/boost
)
