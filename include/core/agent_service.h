#ifndef SYSTEM_MONITORING_AGENT_SERVICE_H
#define SYSTEM_MONITORING_AGENT_SERVICE_H

/**
 * @file agent_service.h
 * @brief В этом заголовочном файле описан класс AgentService
 * @date 18.09.2026
 * @authors Tarasova Alina, Kovalev Georgiy
 */

#include "tools/config_service.h"
#include "agent/agent.h"
#include "core/agent_handler.h"
#include <unordered_map>
#include <thread>

namespace core {

    /**
     * @struct AgentInfo
     * @brief Содержит подробную информацию об агенте
     */
    struct AgentInfo {
        agent::AgentType type; ///< Тип агента
        std::vector<std::string> metrics; ///< Список поддерживаемых метрик
        std::chrono::milliseconds elapsed_time; ///< Время, прошедшее с момента создания агента
        std::chrono::milliseconds refresh_time; ///< Таймаут изменения метрик
    };

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
         * @brief Включает конкретный агент (делает его активным)
         * @param name Имя этого агента
         */
        void enable(const std::string& name) noexcept;

        /**
         * @brief Выключает конкретный агент (делает его неактивным)
         * @param name Имя этого агента
         */
        void disable(const std::string& name) noexcept;
    private:
        std::unordered_map<std::string, AgentCore> _agents_list; ///< Агенты, которые работают в данный момент
    };
}

#endif