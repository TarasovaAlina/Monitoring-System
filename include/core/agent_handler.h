#ifndef SYSTEM_MONITORING_AGENT_LOADER_H
#define SYSTEM_MONITORING_AGENT_LOADER_H

#include <atomic>
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
        // Определяем типы для функций фабрики
        typedef agent::IAgent* (*CreateAgentFunc)(agent::AgentType);
        typedef void (*DestroyAgentFunc)(agent::IAgent*);
    public:
        /**
         * @brief Создается после того, как был прочитан соответствующий конфиг.
         * @param path Путь до динамической библиотеки
         * @param milliseconds Таймаут обновления метрик, передаваемый от ConfigService
         * @throw std::runtime_error Если не удалось загрузить динамическую библиотеку
         */
        AgentHandler(const std::string& path, agent::AgentType type, int milliseconds);
        ~AgentHandler() noexcept;

        void work() noexcept; ///< Через каждые _timeout секунд обновляет метрики
        void setSleepMode(bool is_sleep) noexcept; ///< Устанавливает режим работы (сон или активная работа)
        void switchType(const agent::AgentType& type) noexcept; ///< Устанавливает новый тип агента
        void unload() noexcept; ///< Выгружает библиотеку

        agent::AgentType type() const noexcept; ///< Доступ к приватному полю _type
        long& timeout() noexcept; ///< Доступ к приватному полю _timeout
        std::vector<agent::Metric>& metrics() noexcept; ///< Доступ к текущим параметрам метрик
        long timeElapsedSinceStart() const noexcept; ///< Время в мс, прошедшее с создания этого агента
        std::vector<std::string>& metricNamesList() noexcept; ///< Доступ к списку метрик, значения которых агент должен обновлять
        bool isActive() const noexcept; ///< Доступ к приватному атомарному флагу _sleeping

    private:
        void* _shared_lib; ///< Загруженная динамическая библиотека агента
        std::pair<CreateAgentFunc, DestroyAgentFunc> _agent_func; ///< Функции фабрики, которые используются для создания агентов
        std::unique_ptr<agent::Agent, DestroyAgentFunc> _agent; ///< Агент, загруженный из динамической библиотеки

        std::atomic<bool> _running; ///< Атомарны флаг, показывающий, находится ли агент в процессе выполнения
        std::atomic<bool> _sleeping; ///< Атомарный флаг, показывающий, находится ли агент в состоянии сна

        std::vector<agent::Metric> _metrics; ///< Текущие значения метрик
        std::vector<std::string> _metric_names_list;
        agent::AgentType _type; ///< Тип агента
        long _timeout; ///< Таймаут обновления
        std::chrono::steady_clock::time_point _start_time; ///< Время создания агента
    };
}

#endif