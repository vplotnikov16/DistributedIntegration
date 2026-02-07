#pragma once

#include "integration_strategy.h"

/**
 * @file trapezoidal_rule.h
 * @brief Реализация метода трапеций для численного интегрирования
 */

/**
 * @class TrapezoidRule
 * @brief Метод трапеций
 */
class TrapezoidalRule : public IntegrationStrategyBase
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
};
