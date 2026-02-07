# Тесты методов численного интегрирования

# Создаём библиотеку с методами интегрирования для тестирования
add_library(integration_methods INTERFACE)

# Указываем пути к заголовкам
target_include_directories(integration_methods INTERFACE
    ${CMAKE_SOURCE_DIR}/src/client
    ${CMAKE_SOURCE_DIR}/src/client/integration_methods
)

# Линкуем с common
target_link_libraries(integration_methods INTERFACE common)

# Общие настройки для всех тестов

# Функция для создания теста с общими настройками
function(add_integration_test TEST_NAME TEST_SOURCE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    
    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/client/integration_methods
        ${CMAKE_SOURCE_DIR}/src/third_party/boost 
    )
    
    target_link_libraries(${TEST_NAME} PRIVATE
        integration_methods
        Threads::Threads
    )
    
    # Определяем header-only режим Boost.Test
    target_compile_definitions(${TEST_NAME} PRIVATE
        BOOST_TEST_NO_LIB  # Не линковать библиотеку
    )
    
    # Регистрируем в CTest
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    
    # RPATH для Unix
    if(UNIX)
        set_target_properties(${TEST_NAME} PROPERTIES
            INSTALL_RPATH "$ORIGIN/../../lib"
            BUILD_WITH_INSTALL_RPATH TRUE
        )
    endif()
endfunction()

# Создание тестов

add_integration_test(test_trapezoidal_rule test_trapezoidal_rule.cpp)
add_integration_test(test_simpsons_rule test_simpsons_rule.cpp)
add_integration_test(test_integration_common test_integration_common.cpp)
