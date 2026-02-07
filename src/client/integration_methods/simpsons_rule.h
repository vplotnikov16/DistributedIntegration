#pragma once

#include "integration_strategy.h"
#include <cmath>
#include <vector>

/**
 * @file simpsons_rule.h
 * @brief Реализация метода Симпсона для численного интегрирования
 */

/**
 * @class SimpsonsRule
 * @brief Метод Симпсона (парабол)
 */
class SimpsonsRule : public IntegrationStrategyBase
{
public:
    /**
     * @brief Вычисляет определённый интеграл функции 1/ln(x) методом Симпсона
     *
     * @param lower Нижний предел интегрирования
     * @param upper Верхний предел интегрирования
     * @param step Шаг интегрирования (будет скорректирован для чётного числа интервалов)
     * @return Значение определенного интеграла
     *
     * @throws std::invalid_argument если параметры некорректны
     * @throws std::runtime_error если возникла ошибка при вычислении
     */
    double integrate(double lower, double upper, double step) const override
    {
        // Валидация входных параметров
        validate_parameters(lower, upper, step);

        // Вычисляем количество интервалов
        uint64_t n = static_cast<uint64_t>(std::ceil((upper - lower) / step));

        // Метод Симпсона требует чётное количество интервалов
        if (n % 2 == 1)
        {
            n++;  // Делаем чётным
        }

        // Корректируем шаг для точного покрытия интервала
        double h = (upper - lower) / n;

        // Начальное и конечное значения (коэффициент 1)
        double sum = function(lower) + function(upper);

        // Промежуточные точки
        for (uint64_t i = 1; i < n; i++)
        {
            double x = lower + i * h;

            if (i % 2 == 0)
            {
                // Для четных индексов коэффициент 2
                sum += 2.0 * function(x);
            }
            else
            {
                // Для нечетных индексов коэффициент 4
                sum += 4.0 * function(x);
            }
        }

        return sum * h / 3.0;
    }

    /**
     * @brief Возвращает название метода интегрирования
     * @return "Simpson's Rule"
     */
    std::string get_method_name() const override
    {
        return "Simpson's rule";
    }
};
