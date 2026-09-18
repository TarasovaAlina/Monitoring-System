#ifndef SYSTEM_MONITORING_AGENT_LOADER_H
#define SYSTEM_MONITORING_AGENT_LOADER_H

#include <atomic>
#include <chrono>
#include <functional>
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
         * @param milliseconds Таймаут обновления метрик, передаваемый от ConfigService
         * @throw std::runtime_error Если не удалось загрузить динамическую библиотеку
         */
        AgentHandler(const std::string& path, int milliseconds);
        ~AgentHandler() noexcept;

        void work() noexcept; ///< Через каждые _timeout секунд обновляет метрики
        void setSleepMode(bool is_sleep) noexcept; ///< Устанавливает режим работы (сон или активная работа)
        void unload() noexcept; ///< Выгружает библиотеку

        std::string& name() noexcept; ///< Доступ к приватному полю _name
        std::chrono::milliseconds& timeout() noexcept; ///< Доступ к приватному полю _timeout
        std::vector<agent::Metric>& metrics() noexcept; ///< Доступ к текущим параметрам метрик

    private:
        std::function<void ()> _updateMetricsCallback;
        std::function<std::vector<agent::Metric> ()> _gettingMetricsCallback;
        void* _lib_agent; ///< Загруженная динамическая библиотека агента
        std::atomic<bool> _running; ///< Атомарны флаг, показывающий, находится ли агент в процессе выполнения
        std::atomic<bool> _sleeping; ///< Атомарный флаг, показывающий, находится ли агент в состоянии сна
        std::vector<agent::Metric> _metrics; ///< Текущие значения метрик
        std::string _name; ///< Имя агента
        std::chrono::milliseconds _timeout; ///< Таймаут обновления
    };
}

#endif