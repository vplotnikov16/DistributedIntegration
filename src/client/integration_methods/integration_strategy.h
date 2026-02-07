#pragma once

#include <string>
#include <cmath>
#include <stdexcept>

/**
 * @file integration_strategy.h
 * @brief Интерфейс стратегии численного интегрирования
 */

/**
 * @class IIntegrationStrategy
 * @brief Интерфейс для стратегий численного интегрирования функции 1/ln(x)
 *
 * Данный интерфейс реализует паттерн Strategy для легкой подмены
 * различных методы численного интегрирования (прямоугольникой, трапеций, Симпсона и т.д.)
 */
class IIntegrationStrategy
{
public:
    virtual ~IIntegrationStrategy() = default;

    /**
     * @brief Вычисляет определённый интеграл функции 1/ln(x) на заданном отрезке
     *
     * @param lower Нижний предел интегрирования
     * @param upper Верхний предел интегрирования
     * @param step Шаг интегрирования
     * @return Значение определённого интеграла
     *
     * @throws std::invalid_argument если параметры некорректны
     * @throws std::runtime_error если возникла ошибка при вычислении
     */
    virtual double integrate(double lower, double upper, double step) const = 0;

    /**
     * @brief Возвращает название метода интегрирования
     * @return Строка с названием метода
     */
    virtual std::string get_method_name() const = 0;
};

/**
 * @class IntegrationStrategyBase
 * @brief Базовая реализация стратегии с общими методами валидации и вычисления функции
 *
 * Предоставляет общую логику для интегрирования функции 1/ln(x):
 * - Валидация параметров
 * - Вычисление функции с проверками
 * - Обработка особых случаев
 */
class IntegrationStrategyBase : public IIntegrationStrategy
{
public:
    virtual ~IntegrationStrategyBase() = default;

protected:
    /**
     * @brief Вычисляет значение функции 1/ln(x)
     *
     * @param x Аргумент функции (должен быть > 0 и != 1)
     * @return Значение 1/ln(x)
     *
     * @throws std::runtime_error если x <= 0 или x слишком близко к 1
     */
    static double function(double x)
    {
        // Проверка 1: x должен быть положительным
        if (x <= 0.0)
        {
            throw std::runtime_error("Function 1/ln(x) is undefined for x <= 0");
        }

        double ln_x = std::log(x);

        // Проверка 2: ln(x) не должен быть близок к нулю (x близко к 1)
        constexpr double EPSILON = 1e-10;
        if (std::abs(ln_x) < EPSILON)
        {
            throw std::runtime_error("Function 1/ln(x) is undefined for x too close to 1 ");
        }

        return 1.0 / ln_x;
    }

    /**
     * @brief Валидирует параметры интегрирования для функции 1/ln(x)
     *
     * @param lower Нижний предел интегрирования
     * @param upper Верхний предел интегрирования
     * @param step Шаг интегрирования
     *
     * @throws std::invalid_argument если параметры некорректны
     */
    static void validate_parameters(double lower, double upper, double step)
    {
        // ограничения для интегрирования 1/ln(x)
        // true - все хорошо
        bool result = true;
        
        // Начало не может быть больше конца, шаг должен быть положительным
        // и быть меньше длины интегрируемого интервала
        result &= !(lower >= upper || step <= 0.0 || step >= (upper - lower));

        // Нижний предел должен быть положительным
        result &= !(lower <= 0.0);

        // Интервал не должен содержать x = 1
        result &= !(lower < 1.0 && upper > 1.0);
        result &= !(std::abs(lower - 1.0) < 1e-10 || std::abs(upper - 1.0) < 1e-10);

        if (!result) throw std::invalid_argument("Incorrect parameters");
    }
};
