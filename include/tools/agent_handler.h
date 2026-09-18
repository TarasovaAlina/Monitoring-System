#ifndef SYSTEM_MONITORING_AGENT_LOADER_H
#define SYSTEM_MONITORING_AGENT_LOADER_H

#include <chrono>
#include <vector>
#include <string>

#include "agent/agent.h"

namespace core {
    /**
     * @class AgentHandler
     * @brief Надстройка над IAgent* классами.
     * Позволяет настраивать и управлять агентами как динамически загружаемыми библиотеками
     */
    class AgentHandler {
    public:
        /**
         * @brief Создается после того, как был прочитан соответствующий конфиг.
         * @param path Путь до динамической библиотеки
         * @param name Имя агента, передаваемое от ConfigService
         * @param milliseconds Таймаут обновления метрик, передаваемый от ConfigService
         */
        AgentHandler(const std::string& path, const std::string& name, int milliseconds) noexcept;
        ~AgentHandler() noexcept;

        void work() noexcept; ///< Через каждые _timeout секунд обновляет метрики
        void unload() noexcept; ///< Выгружает библиотеку

        std::string& name() noexcept; ///< Доступ к приватному полю _name
        std::chrono::milliseconds& timeout() noexcept; ///< Доступ к приватному полю _timeout

    private:
        std::unique_ptr<agent::IAgent> _agent; ///< Агент, выполняющий работу по сборке метрик
        std::vector<agent::Metric> _metrics; ///< Текущие значения метрик
        std::string _name; ///< Имя агента
        std::chrono::milliseconds _timeout; ///< Таймаут обновления
    };
}

#endif