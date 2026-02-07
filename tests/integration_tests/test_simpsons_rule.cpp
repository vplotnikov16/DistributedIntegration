#define BOOST_TEST_MODULE SimpsonsRuleTests
#include <boost/test/included/unit_test.hpp>
#include <cmath>
#include <memory>

#include "integration_strategy.h"
#include "simpsons_rule.h"
#include "trapezoidal_rule.h"

/**
 * @file test_simpsons_rule.cpp
 * @brief Модульные тесты для метода Симпсона
 */

// Допустимая погрешность для сравнения double
const double EPSILON = 1e-6;

// Тестовый набор: Базовые проверки
BOOST_AUTO_TEST_SUITE(BasicTests)

/**
 * @brief Тест создания объекта SimpsonsRule
 */
BOOST_AUTO_TEST_CASE(CreateSimpsonsRule)
{
    BOOST_TEST_MESSAGE("Testing SimpsonsRule creation...");
    
    std::unique_ptr<IIntegrationStrategy> strategy = 
        std::make_unique<SimpsonsRule>();
    
    BOOST_CHECK(strategy != nullptr);
    BOOST_CHECK_EQUAL(strategy->get_method_name(), "Simpson's rule");
}

/**
 * @brief Тест интегрирования на простом диапазоне [2, 3]
 */
BOOST_AUTO_TEST_CASE(IntegrateSimpleRange)
{
    BOOST_TEST_MESSAGE("Testing integration on [2, 3] with step=0.01...");
    
    SimpsonsRule rule;
    
    double lower = 2.0;
    double upper = 3.0;
    double step = 0.01;
    
    // Вычисляем интеграл
    double result = rule.integrate(lower, upper, step);
    // Из WolframAlpha
    double expected = 1.11842;
    
    BOOST_TEST_MESSAGE("Result: " << result);
    BOOST_TEST_MESSAGE("Expected: " << expected);
    
    // Метод Симпсона имеет погрешность ~O(h^4)
    BOOST_CHECK_CLOSE(result, expected, 0.5);
}

/**
 * @brief Сравнение точности Симпсона и трапеций
 */
BOOST_AUTO_TEST_CASE(CompareWithTrapezoidal)
{
    BOOST_TEST_MESSAGE("Comparing Simpson's vs Trapezoidal accuracy...");
    
    SimpsonsRule simpson;
    TrapezoidalRule trapezoid;
    
    double lower = 2.0;
    double upper = 3.0;
    double step = 0.01;
    double expected = 1.11842;
    
    double simpson_result = simpson.integrate(lower, upper, step);
    double trapezoid_result = trapezoid.integrate(lower, upper, step);
    
    double simpson_error = std::abs(simpson_result - expected);
    double trapezoid_error = std::abs(trapezoid_result - expected);
    
    BOOST_TEST_MESSAGE("Simpson error: " << simpson_error);
    BOOST_TEST_MESSAGE("Trapezoid error: " << trapezoid_error);
    
    // Метод Симпсона должен быть точнее
    BOOST_CHECK(simpson_error < trapezoid_error);
}

/**
 * @brief Тест с очень маленьким шагом
 */
BOOST_AUTO_TEST_CASE(IntegrateSmallStep)
{
    BOOST_TEST_MESSAGE("Testing integration with small step=0.001...");
    
    SimpsonsRule rule;
    
    double lower = 2.0;
    double upper = 3.0;
    double step = 0.00001;  // Маленький шаг
    
    double result = rule.integrate(lower, upper, step);
    double expected = 1.11842;
    
    BOOST_TEST_MESSAGE("Result: " << result);
    BOOST_TEST_MESSAGE("Expected: " << expected);
    
    // С маленьким шагом точность должна быть выше
    BOOST_CHECK_CLOSE(result, expected, 0.0005);
}

/**
 * @brief Тест с большим диапазоном [2, 10000]
 */
BOOST_AUTO_TEST_CASE(IntegrateLargeRange)
{
    BOOST_TEST_MESSAGE("Testing integration on [2, 10]...");
    
    SimpsonsRule rule;
    
    double lower = 2.0;
    double upper = 10000.0;
    double step = 0.01;
    
    double result = rule.integrate(lower, upper, step);
    
    // Эталонное значение
    double expected = 1245.09;
    
    BOOST_TEST_MESSAGE("Result: " << result);
    BOOST_TEST_MESSAGE("Expected: " << expected);

    BOOST_CHECK_CLOSE(result, expected, 0.5);
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
    
    SimpsonsRule rule;
    
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
        BOOST_CHECK_CLOSE(result, expected, 0.5);
    });
}

/**
 * @brief Тест с очень узким диапазоном [2, 2.1]
 */
BOOST_AUTO_TEST_CASE(IntegrateNarrowRange)
{
    BOOST_TEST_MESSAGE("Testing narrow range [2, 2.1]...");
    
    SimpsonsRule rule;
    
    double lower = 2.0;
    double upper = 2.1;
    double step = 0.001;

    // WolframAlpha
    double expected = 0.13938;

    double result = rule.integrate(lower, upper, step);
    
    BOOST_TEST_MESSAGE("Result: " << result);
    BOOST_TEST_MESSAGE("Expected: " << expected);

    BOOST_CHECK_CLOSE(result, expected, 0.5);
}

/**
 * @brief Тест с большими значениями [0.001, 0.999]
 */
BOOST_AUTO_TEST_CASE(IntegrateLargeValues)
{
    BOOST_TEST_MESSAGE("Testing large values [0.001, 0.999]...");
    
    SimpsonsRule rule;
    
    double lower = 0.001;
    double upper = 0.999;
    double step = 0.001;
    
    // WolframAlpha
    double expected = -6.33091;

    double result = rule.integrate(lower, upper, step);
    
    BOOST_TEST_MESSAGE("Result: " << result);
    BOOST_TEST_MESSAGE("Expected: " << expected);

    BOOST_CHECK_CLOSE(result, expected, 0.5);
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
    
    SimpsonsRule rule;
    
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
    
    SimpsonsRule rule;
    
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
    
    SimpsonsRule rule;
    
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
    
    SimpsonsRule rule;
    
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
    
    SimpsonsRule rule;
    
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
    
    SimpsonsRule rule;
    
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
    
    SimpsonsRule rule;
    
    double lower = 3.0;
    double upper = 2.0;
    double step = 0.01;
    
    BOOST_CHECK_THROW(
        rule.integrate(lower, upper, step),
        std::invalid_argument
    );
}

BOOST_AUTO_TEST_SUITE_END()
