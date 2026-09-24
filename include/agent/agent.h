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
        virtual const std::vector<Metric> updateMetrics(const std::vector<std::string>&) const noexcept = 0;
    };

    /**
     * @class Agent
     * @brief Класс, описывающий агента для сборки метрик
     */
    class Agent final : public IAgent {
    public:
        /**
         * @brief Создание специального сборщика метрик, который будет заниматься получением актуальных данных
         * @param type Тип требуемого агента
         */
        explicit Agent(AgentType type) noexcept;

        /**
         * @brief Получение текущих значений конкретных метрик, собираемых этим агентом-библиотекой.
         * @param metric_names_list Список метрик, которые необходимо собрать коллектору
         * @return Список актуальных значений метрик
         */
        const std::vector<Metric> updateMetrics(const std::vector<std::string>& metric_names_list) const noexcept;

    private:
        std::unique_ptr<IMetricsCollector> _metrics_collector; ///< Сборщик метрик, который имплементирует в себе логику получения данных
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