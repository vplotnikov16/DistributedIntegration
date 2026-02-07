#pragma once

#include "integration_strategy.h"
#include <cmath>
#include <stdexcept>
#include <sstream>

/**
 * @file trapezoid_rule.h
 * @brief Реализация метода трапеций для численного интегрирования
 */

/**
 * @class TrapezoidRule
 * @brief Метод трапеций
 */
class TrapezoidalRule : public IIntegrationStrategy
{
public:
    /**
     * @brief Вычисляет определённый интеграл функции 1/ln(x) методом трапеций
     *
     * @param lower Нижний предел интегрирования
     * @param upper Верхний предел интегрирования
     * @param step Шаг интегрирования
     * @return Значение определенного интеграла
     *
     * @throws std::invalid_argument если параметры некорректны
     * @throws std::runtime_error если возникла ошибка при вычислении
     */
    double integrate(double lower, double upper, double step) const override
    {
        // Валидация входных параметров
        validate_parameters(lower, upper, step);

        double sum = 0.0;
        double x = lower;

        // Значение функции в начальной точке
        double f_prev = function(x);

        // Основной цикл интегрирования
        while (x < upper)
        {
            // Следующая точка
            double x_next = x + step;

            // Корректируем последний шаг, если выходим за границу
            if (x_next > upper)
            {
                x_next = upper;
            }

            // Значение функции в следующей точке
            double f_next = function(x_next);

            // Площадь трапеции
            double trapezoid_area = (f_prev + f_next) * (x_next - x) / 2.0;
            sum += trapezoid_area;

            // Переход к следующему шагу
            x = x_next;
            f_prev = f_next;
        }

        return sum;
    }

    /**
     * @brief Возвращает название метода интегрирования
     * @return "Trapezoidal rule"
     */
    std::string get_method_name() const override
    {
        return "Trapezoidal rule";
    }

private:
    /**
     * @brief Вычисляет значение функции 1/ln(x)
     *
     * @param x Аргумент функции (должен быть > 1)
     * @return Значение 1/ln(x)
     *
     * @throws std::runtime_error если x <= 1 (ln(x) <= 0)
     */
    static double function(double x)
    {
        if (x <= 0.0)
        {
            std::ostringstream oss;
            oss << "Function 1/ln(x) is undefined for x <= 0.0";
            throw std::runtime_error(oss.str());
        }

        double ln_x = std::log(x);

        return 1.0 / ln_x;
    }

    /**
     * @brief Валидирует параметры интегрирования
     *
     * @param lower Нижний предел
     * @param upper Верхний предел
     * @param step Шаг
     *
     * @throws std::invalid_argument если параметры некорректны
     */
    static void validate_parameters(double lower, double upper, double step)
    {
        std::ostringstream error_msg;

        // Проверка 1: Нижний предел должен быть положительным
        if (lower <= 0.0)
        {
            error_msg << "Lower limit must be positive for function 1/ln(x) (got "
                      << lower << ")";
            throw std::invalid_argument(error_msg.str());
        }

        // Проверка 2: Интервал не должен содержать x = 1 (особая точка)
        if ((lower < 1.0 && upper > 1.0) ||
            std::abs(lower - 1.0) < 1e-10 ||
            std::abs(upper - 1.0) < 1e-10)
        {
            error_msg << "Integration interval cannot contain x = 1 ";
            throw std::invalid_argument(error_msg.str());
        }

        // Проверка 3: Верхний предел больше нижнего
        if (upper <= lower)
        {
            error_msg << "Upper limit must be greater than lower limit";
            throw std::invalid_argument(error_msg.str());
        }

        // Проверка 4: Шаг положительный
        if (step <= 0.0)
        {
            error_msg << "Step must be positive";
            throw std::invalid_argument(error_msg.str());
        }

        // Проверка 5: Шаг меньше диапазона
        if (step >= (upper - lower))
        {
            error_msg << "Step must be smaller than integration range";
            throw std::invalid_argument(error_msg.str());
        }
    }
};
