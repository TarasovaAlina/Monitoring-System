#ifndef SYSTEM_MONITORING_AGENT_INTERFACE_H
#define SYSTEM_MONITORING_AGENT_INTERFACE_H

/**
 * @file agent.h
 * @brief В этом файле описан интерфейс класса агента,
 * который будет использоваться для построения конкретных реализаций агентов
 * @author Georgiy Kovalev
 * @date 04.09.2026
 */

#include <string>
#include <vector>

/**
 * @namespace agent
 * @brief В этом пространстве имен описаны классы агентов, которые отслеживают системные метрики
 */
namespace agent {
    /**
     * @struct Metric
     * @brief Содержит информацию об отдельной метрике, которую собирает агент
     */
    struct Metric {
        std::string name; ///< Название метрики
        double value; ///< Значение метрики
    };

    enum AgentType {
        CPU_AGENT, ///< Агент, отслеживающий загрузку CPU
        MEMORY_AGENT, ///< Агент, отслеживающий использование RAM и HDD
        NETWORK_AGENT ///< Агент, отслеживающий использование сети
    };

    /**
     * @class IAgent
     * @brief Базовый класс, описывающий организацию агентов для сбора метрик
     */
    class IAgent {
    public:
        explicit IAgent(AgentType type) noexcept : _type(type) {}
        virtual ~IAgent() noexcept = default;

        /**
         * @brief Каждый класс агента должен иметь метод,
         * позволяющий подгружать актуальные значения метрик, собираемые этим агентом-библиотекой
         */
        virtual void updateMetrics() noexcept = 0;

        /**
         * @brief Каждый класс агента должен иметь метод,
         * позволяющий выдавать актуальные данные для их отображения в GUI
         * @return Список метрик, которые собирает агент
         */
        virtual std::vector<Metric> getMetrics() noexcept = 0;

        AgentType type() const noexcept {
            return _type;
        }
    protected:
        AgentType _type; ///< Тип агента
    };
}

#endif