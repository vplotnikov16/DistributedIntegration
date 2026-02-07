# Используем уже имеющийся Boost из src/third_party/boost
set(BOOST_ROOT ${CMAKE_SOURCE_DIR}/src/third_party/boost)
set(BOOST_TEST_HEADER ${BOOST_ROOT}/boost/test/included/unit_test.hpp)
set(Boost_NO_SYSTEM_PATHS ON)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)

if(NOT EXISTS ${BOOST_TEST_HEADER})
    message(STATUS "Boost.Test header not found at: ${BOOST_TEST_HEADER}")
    message(STATUS "Checking if Boost needs header generation...")
    
    # Проверяем, существует ли сам Boost
    if(NOT EXISTS ${BOOST_ROOT}/boost.css)
        message(FATAL_ERROR 
            "Boost submodule not found at ${BOOST_ROOT}\n"
            "Please run:\n"
            "  git submodule update --init --recursive\n"
        )
    endif()
    
    message(FATAL_ERROR 
        "Boost headers not generated. Please run:\n"
        "  cd src/third_party/boost\n"
        "  bootstrap.bat    (Windows) or ./bootstrap.sh (Linux/macOS)\n"
        "  b2 headers       (Windows) or ./b2 headers (Linux/macOS)\n"
    )
endif()

# Добавляем поддиректории с тестами
add_subdirectory(integration_tests)
