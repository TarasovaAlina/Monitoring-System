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
#include "core/agent_handler.h"

namespace core {
    /**
     * @class AgentService
     * @brief Управляет всеми агентами в программе
     */
    class AgentService {
    public:
        /**
         * @brief Обновляет список текущих агентов, работающих в программе.
         * Если какая-то библиотека была удалена, то такой объект выгружается из памяти.
         * Если, наоборот, найдена новая библиотека, то создается новый объект для ее управления
         * @param agent_data_list Информация об агентах, считанных из конфигов
         */
        void update(const std::vector<ConfigInfo>& agent_data_list) noexcept;

        /**
         * @brief Передает информацию о конкретном агенте
         * @param name Имя данного агента
         * @return Подробная информация об агенте, которая будет отображена в UI
         */
        AgentInfo getAgentInfo(const std::string& name);

        /**
         * @brief Передает собранные метрики от всех активных агентов
         * @return Список данных в виде имя_метрики : значение
         */
        std::vector<agent::Metric> collectMetrics() noexcept;

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
        /**
         * @brief Множество агентов, сохраненных под своими именами
         */
        std::unordered_map<std::string, std::unique_ptr<AgentHandler>> _agents_list;
    };
}

#endif