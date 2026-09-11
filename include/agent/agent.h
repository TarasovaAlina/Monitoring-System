#ifndef SYSTEM_MONITORING_AGENT_INTERFACE_H
#define SYSTEM_MONITORING_AGENT_INTERFACE_H

/**
 * @file agent.h
 * @brief В этом файле описан интерфейс класса агента,
 * который будет использоваться для построения конкретных реализаций агентов
 * @author Georgiy Kovalev
 * @date 04.09.2026
 */

/**
 * @namespace agent
 * @brief В этом пространстве имен описаны классы агентов, которые отслеживают системные метрики
 */
namespace agent {
    /**
     * @class IAgent
     * @brief Базовый класс, описывающий организацию агентов для сбора метрик
     */
    class IAgent {
    public:
        virtual ~IAgent() noexcept = default;

        /**
         * @brief Каждый класс агента должен иметь метод,
         * позволяющий подгружать актуальные значения метрик, собираемые этим агентом-библиотекой
         */
        virtual void updateMetrics() noexcept = 0;
    };
}

#endif