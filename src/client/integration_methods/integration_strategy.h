#pragma once

#include <string>

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
