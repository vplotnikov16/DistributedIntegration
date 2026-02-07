#define BOOST_TEST_MODULE TrapezoidalRuleTests
#include <boost/test/included/unit_test.hpp>
#include <cmath>
#include <memory>

#include "integration_strategy.h"
#include "trapezoidal_rule.h"

/**
 * @file test_trapezoidal_rule.cpp
 * @brief Модульные тесты для метода трапеций
 */

// Допустимая погрешность для сравнения double
const double EPSILON = 1e-6;

// Тестовый набор: Базовые проверки
BOOST_AUTO_TEST_SUITE(BasicTests)

/**
 * @brief Тест создания объекта TrapezoidalRule
 */
BOOST_AUTO_TEST_CASE(CreateTrapezoidalRule)
{
    BOOST_TEST_MESSAGE("Testing TrapezoidalRule creation...");
    
    std::unique_ptr<IIntegrationStrategy> strategy = 
        std::make_unique<TrapezoidalRule>();
    
    BOOST_CHECK(strategy != nullptr);
    BOOST_CHECK_EQUAL(strategy->get_method_name(), "Trapezoidal rule");
}

/**
 * @brief Тест интегрирования на простом диапазоне [2, 3]
 */
BOOST_AUTO_TEST_CASE(IntegrateSimpleRange)
{
    BOOST_TEST_MESSAGE("Testing integration on [2, 3] with step=0.01...");
    
    TrapezoidalRule rule;
    
    double lower = 2.0;
    double upper = 3.0;
    double step = 0.01;
    
    // Вычисляем интеграл
    double result = rule.integrate(lower, upper, step);
    // Из WolframAlpha
    double expected = 1.11842;
    
    BOOST_TEST_MESSAGE("Result: " << result);
    BOOST_TEST_MESSAGE("Expected: " << expected);
    
    // Метод трапеций имеет погрешность ~O(h^2)
    BOOST_CHECK_CLOSE(result, expected, 1.0);
}

/**
 * @brief Тест с очень маленьким шагом
 */
BOOST_AUTO_TEST_CASE(IntegrateSmallStep)
{
    BOOST_TEST_MESSAGE("Testing integration with small step=0.001...");
    
    TrapezoidalRule rule;
    
    double lower = 2.0;
    double upper = 3.0;
    double step = 0.00001;  // Маленький шаг
    
    double result = rule.integrate(lower, upper, step);
    double expected = 1.11842;
    
    BOOST_TEST_MESSAGE("Result: " << result);
    BOOST_TEST_MESSAGE("Expected: " << expected);
    
    // С маленьким шагом точность должна быть выше
    BOOST_CHECK_CLOSE(result, expected, 0.01);
}

/**
 * @brief Тест с большим диапазоном [2, 10000]
 */
BOOST_AUTO_TEST_CASE(IntegrateLargeRange)
{
    BOOST_TEST_MESSAGE("Testing integration on [2, 10]...");
    
    TrapezoidalRule rule;
    
    double lower = 2.0;
    double upper = 10000.0;
    double step = 0.01;
    
    double result = rule.integrate(lower, upper, step);
    
    // Эталонное значение
    double expected = 1245.09;
    
    BOOST_TEST_MESSAGE("Result: " << result);
    BOOST_TEST_MESSAGE("Expected: " << expected);

    BOOST_CHECK_CLOSE(result, expected, 1.0);
}

BOOST_AUTO_TEST_SUITE_END()

// Граничные случаи
BOOST_AUTO_TEST_SUITE(EdgeCaseTests)

/**
 * @brief Тест с диапазоном близко к 1 справа [1.1, 2]
 */
BOOST_AUTO_TEST_CASE(IntegrateNearOne)
{
    BOOST_TEST_MESSAGE("Testing integration near x=1 on [1.1, 2]...");
    
    TrapezoidalRule rule;
    
    double lower = 1.1;
    double upper = 2.0;
    double step = 0.01;

    // WolframAlpha
    double expected = 2.72094;
    
    // Не должно выбрасывать исключение
    BOOST_CHECK_NO_THROW({
        double result = rule.integrate(lower, upper, step);
        BOOST_TEST_MESSAGE("Result: " << result);
        BOOST_TEST_MESSAGE("Expected: " << expected);
        BOOST_CHECK_CLOSE(result, expected, 1.0);
    });
}

/**
 * @brief Тест с очень узким диапазоном [2, 2.1]
 */
BOOST_AUTO_TEST_CASE(IntegrateNarrowRange)
{
    BOOST_TEST_MESSAGE("Testing narrow range [2, 2.1]...");
    
    TrapezoidalRule rule;
    
    double lower = 2.0;
    double upper = 2.1;
    double step = 0.001;

    // WolframAlpha
    double expected = 0.13938;

    double result = rule.integrate(lower, upper, step);
    
    BOOST_TEST_MESSAGE("Result: " << result);
    BOOST_TEST_MESSAGE("Expected: " << expected);

    BOOST_CHECK_CLOSE(result, expected, 1.0);
}

/**
 * @brief Тест с большими значениями [0.001, 0.999]
 */
BOOST_AUTO_TEST_CASE(IntegrateLargeValues)
{
    BOOST_TEST_MESSAGE("Testing large values [0.001, 0.999]...");
    
    TrapezoidalRule rule;
    
    double lower = 0.001;
    double upper = 0.999;
    double step = 0.0001;
    
    // WolframAlpha
    double expected = -6.33091;

    double result = rule.integrate(lower, upper, step);
    
    BOOST_TEST_MESSAGE("Result: " << result);
    BOOST_TEST_MESSAGE("Expected: " << expected);

    BOOST_CHECK_CLOSE(result, expected, 1.0);
}

BOOST_AUTO_TEST_SUITE_END()

// Ошибочные параметры
BOOST_AUTO_TEST_SUITE(ErrorTests)

/**
 * @brief Тест с отрицательным нижним пределом (должно выбросить исключение)
 */
BOOST_AUTO_TEST_CASE(IntegrateNegativeLower)
{
    BOOST_TEST_MESSAGE("Testing negative lower bound (should throw)...");
    
    TrapezoidalRule rule;
    
    double lower = -1.0;
    double upper = 2.0;
    double step = 0.01;
    
    BOOST_CHECK_THROW(
        rule.integrate(lower, upper, step),
        std::invalid_argument
    );
}

/**
 * @brief Тест с нулевым нижним пределом
 */
BOOST_AUTO_TEST_CASE(IntegrateZeroLower)
{
    BOOST_TEST_MESSAGE("Testing zero lower bound (should throw)...");
    
    TrapezoidalRule rule;
    
    double lower = 0.0;
    double upper = 2.0;
    double step = 0.01;
    
    BOOST_CHECK_THROW(
        rule.integrate(lower, upper, step),
        std::invalid_argument
    );
}

/**
 * @brief Тест с диапазоном, содержащим x=1
 */
BOOST_AUTO_TEST_CASE(IntegrateContainsOne)
{
    BOOST_TEST_MESSAGE("Testing range containing x=1 (should throw)...");
    
    TrapezoidalRule rule;
    
    double lower = 0.5;
    double upper = 1.5;
    double step = 0.01;
    
    BOOST_CHECK_THROW(
        rule.integrate(lower, upper, step),
        std::invalid_argument
    );
}

/**
 * @brief Тест с нижним пределом равным 1
 */
BOOST_AUTO_TEST_CASE(IntegrateLowerEqualsOne)
{
    BOOST_TEST_MESSAGE("Testing lower=1 (should throw)...");
    
    TrapezoidalRule rule;
    
    double lower = 1.0;
    double upper = 2.0;
    double step = 0.01;
    
    BOOST_CHECK_THROW(
        rule.integrate(lower, upper, step),
        std::invalid_argument
    );
}

/**
 * @brief Тест с отрицательным шагом
 */
BOOST_AUTO_TEST_CASE(IntegrateNegativeStep)
{
    BOOST_TEST_MESSAGE("Testing negative step (should throw)...");
    
    TrapezoidalRule rule;
    
    double lower = 2.0;
    double upper = 3.0;
    double step = -0.01;
    
    BOOST_CHECK_THROW(
        rule.integrate(lower, upper, step),
        std::invalid_argument
    );
}

/**
 * @brief Тест с нулевым шагом
 */
BOOST_AUTO_TEST_CASE(IntegrateZeroStep)
{
    BOOST_TEST_MESSAGE("Testing zero step (should throw)...");
    
    TrapezoidalRule rule;
    
    double lower = 2.0;
    double upper = 3.0;
    double step = 0.0;
    
    BOOST_CHECK_THROW(
        rule.integrate(lower, upper, step),
        std::invalid_argument
    );
}

/**
 * @brief Тест с перевернутым диапазоном (lower > upper)
 */
BOOST_AUTO_TEST_CASE(IntegrateInvertedRange)
{
    BOOST_TEST_MESSAGE("Testing inverted range (should throw)...");
    
    TrapezoidalRule rule;
    
    double lower = 3.0;
    double upper = 2.0;
    double step = 0.01;
    
    BOOST_CHECK_THROW(
        rule.integrate(lower, upper, step),
        std::invalid_argument
    );
}

BOOST_AUTO_TEST_SUITE_END()
