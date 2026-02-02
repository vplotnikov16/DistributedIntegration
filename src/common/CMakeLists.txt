set(COMMON_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils.h
)
# Создаем библиотеку common
add_library(common STATIC ${COMMON_SOURCES})

# Публичные заголовки
target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
