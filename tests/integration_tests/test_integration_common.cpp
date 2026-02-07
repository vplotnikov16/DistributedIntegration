#define BOOST_TEST_MODULE IntegrationCommonTests
#include <boost/test/included/unit_test.hpp>
#include <memory>

#include "integration_strategy.h"
#include "trapezoidal_rule.h"
#include "simpsons_rule.h"

/**
 * @file test_integration_common.cpp
 * @brief Общие тесты для всех методов интегрирования
 */

// Полиморфизм и интерфейс
BOOST_AUTO_TEST_SUITE(PolymorphismTests)

/**
 * @brief Тест использования через интерфейс IIntegrationStrategy
 */
BOOST_AUTO_TEST_CASE(UseViaInterface)
{
    BOOST_TEST_MESSAGE("Testing polymorphic usage...");
    
    std::unique_ptr<IIntegrationStrategy> strategy1 = 
        std::make_unique<TrapezoidalRule>();
    std::unique_ptr<IIntegrationStrategy> strategy2 = 
        std::make_unique<SimpsonsRule>();
    
    double lower = 2.0;
    double upper = 3.0;
    double step = 0.01;

    double expected = 1.11842;
    
    double result1 = strategy1->integrate(lower, upper, step);
    double result2 = strategy2->integrate(lower, upper, step);
    
    BOOST_TEST_MESSAGE("Trapezoidal: " << result1);
    BOOST_TEST_MESSAGE("Simpson: " << result2);
    
    // Оба должны дать положительные результаты
    BOOST_CHECK(result1 > 0.0);
    BOOST_CHECK(result2 > 0.0);
    
    // Simpson должен быть точнее
    BOOST_CHECK(
        std::abs(result2 - expected) < std::abs(result1 - expected)
    );
}

/**
 * @brief Тест get_method_name()
 */
BOOST_AUTO_TEST_CASE(GetMethodNames)
{
    BOOST_TEST_MESSAGE("Testing method names...");
    
    TrapezoidalRule trap;
    SimpsonsRule simp;
    
    BOOST_CHECK_EQUAL(trap.get_method_name(), "Trapezoidal rule");
    BOOST_CHECK_EQUAL(simp.get_method_name(), "Simpson's rule");
}

BOOST_AUTO_TEST_SUITE_END()

// Тесты вокруг 1/ln(x)
BOOST_AUTO_TEST_SUITE(FunctionTests)

/**
 * @brief Тест вычисления функции в нормальных точках
 */
BOOST_AUTO_TEST_CASE(FunctionNormalPoints)
{
    BOOST_TEST_MESSAGE("Testing function(x) for normal points...");
    
    TrapezoidalRule rule1;
    TrapezoidalRule rule2;
    
    BOOST_CHECK_NO_THROW({
        rule1.integrate(2.0, 3.0, 0.01);
    });
    BOOST_CHECK_NO_THROW({
        rule2.integrate(2.0, 3.0, 0.01);
    });
}

BOOST_AUTO_TEST_SUITE_END()
