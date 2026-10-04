#ifndef SYSTEM_MONITORING_AGENT_SERVICE_H
#define SYSTEM_MONITORING_AGENT_SERVICE_H

/**
 * @file agent_service.h
 * @brief В этом заголовочном файле описан класс AgentService
 * @date 18.09.2026
 * @authors Tarasova Alina, Kovalev Georgiy
 */

#include "config_service.h"
#include "agent/agent.h"
#include "agent_handler.h"
#include <unordered_map>
#include <thread>

namespace core {

    /**
     * @class AgentService
     * @brief Управляет всеми агентами в программе
     */
    class AgentService {
        /**
         * @struct AgentCore
         * @brief Описывает рабочее состояние агента
         */
        struct AgentCore {
            std::unique_ptr<AgentHandler> agent; ///< Управление работой агента
            std::thread agent_work_thread; ///< Поток, в котором происходит считывание метрик
            std::vector<MetricConfig> critical_metrics_values; ///< Критические значения метрик
        };

    public:
        ~AgentService() noexcept;
        /**
         * @brief Обновляет список текущих агентов, работающих в программе.
         * Если какая-то библиотека была удалена, то такой объект выгружается из памяти.
         * Если, наоборот, найдена новая библиотека, то создается новый объект для ее управления
         * @param agent_data_list Информация об агентах, считанных из конфигов
         */
        void update(const std::vector<ConfigInfo>& agent_data_list) noexcept;

        /**
         * @brief Передает управление конкретным агентом выше по цепочке вызовов
         * @param name Имя данного агента
         * @return Обработчик агента с именем name
         */
        std::unique_ptr<AgentHandler>& getAgent(const std::string& name);

        /**
         * @brief Передает собранные метрики от всех активных агентов
         * @return Список данных в виде имя_метрики : значение
         */
        std::vector<agent::Metric> collectMetrics() noexcept;

        /**
         * @brief Сообщает, какие агенты загружены в данный момент.
         * Необходимо для того, чтобы потом получать информацию по этим агентам
         * @return Список имен агентов
         */
        std::vector<std::string> agentsNamesList() const noexcept;

        /**
         * @brief Изменяет имя загруженного в систему агента
         * @param name Текущее имя агента
         * @param new_name Новое имя агента
         */
        void changeName(const std::string& name, const std::string& new_name) noexcept;

        /**
         * @brief Передает информацию о критических значениях метрик, которые собирает агент
         * @param name Имя агента, у которого берется информация
         */
        std::vector<MetricConfig>& criticalMetricValues(const std::string& name) noexcept;
    private:
        std::unordered_map<std::string, AgentCore> _agents_list; ///< Агенты, которые работают в данный момент
    };
}

#endif