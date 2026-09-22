#ifndef SYSTEM_MONITORING_AGENT_H
#define SYSTEM_MONITORING_AGENT_H

/**
 * @file agent.h
 * @brief В этом файле описан класс Agent
 * @author Georgiy Kovalev
 * @date 04.09.2026
 */

#include "metrics_collector.h"
#include <vector>
#include <memory>

/**
 * @namespace agent
 * @brief В этом пространстве имен описаны классы агентов, которые отслеживают системные метрики
 */
namespace agent {
    /**
     * @enum AgentType
     * @brief Перечисление, хранящее возможные типы агентов, которые могут быть загружены в программу
     */
    enum AgentType {
        CPU_AGENT, ///< Агент, отслеживающий загрузку CPU
        MEMORY_AGENT, ///< Агент, отслеживающий использование RAM и HDD
        NETWORK_AGENT ///< Агент, отслеживающий использование сети
    };

    /**
     * @class IAgent
     * @brief Абстрактный класс, который необходим для корректной загрузки из динамической библиотеки
     */
    class IAgent {
    public:
        virtual ~IAgent() noexcept = default;
        virtual void updateMetrics() noexcept = 0;
        virtual void switchType(const AgentType& new_type) noexcept = 0;

        virtual const std::vector<Metric>& getMetrics() const noexcept = 0;
        virtual AgentType type() const noexcept = 0;
    };

    /**
     * @class Agent
     * @brief Класс, описывающий агента для сборки метрик
     */
    class Agent final : public IAgent {
    public:
        explicit Agent(AgentType type) noexcept;

        /**
         * @brief Каждый класс агента должен иметь метод,
         * позволяющий подгружать актуальные значения метрик, собираемые этим агентом-библиотекой
         */
        void updateMetrics() noexcept override;
        void switchType(const AgentType& new_type) noexcept override;

        /**
         * @brief Каждый класс агента должен иметь метод,
         * позволяющий выдавать актуальные данные для их отображения в GUI
         * @return Список метрик, которые собирает агент
         */
        const std::vector<Metric>& getMetrics() const noexcept override;
        AgentType type() const noexcept override;

    private:
        AgentType _type; ///< Тип агента
        std::unique_ptr<IMetricsCollector> _metrics_collector; ///< Сборщик метрик, который имплементирует в себе логику получения данных
        std::vector<Metric> _metrics;
    };

    // Экспорт C-функций для возможной загрузки объекта Agent из динамической библиотеки
    extern "C" {
        __attribute__((visibility("default"))) inline IAgent* CreateAgent(AgentType type) {
            return new Agent(type);
        }

        __attribute__((visibility("default"))) inline void DestroyAgent(IAgent* agent) {
            delete agent;
        }
    }
}

#endif