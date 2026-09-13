#ifndef SYSTEM_MONITORING_KERNEL_H
#define SYSTEM_MONITORING_KERNEL_H

#include "tools/agent_service.h"
#include "logger.h"
#include <memory>
#include <chrono>
#include <atomic>
#include <thread>

/**
 * @file kernel.h
 * @brief В этом файле описан класс Kernel
 * @date 13.09.2026
 * @authors Tarasova Alina, Georgiy Kovalev
 */

/**
 * @namespace core
 * @brief В этом пространстве имен описаны основные классы, которые управляют программой
 */
namespace core {

    /**
     * @struct AgentInfo
     * @brief Содержит подробную информацию об агенте
     */
    struct AgentInfo {
        agent::AgentType type; ///< Тип агента
        std::vector<std::string> metrics; ///< Список поддерживаемых метрик
        std::chrono::time_point<std::chrono::system_clock> start_time; ///<
        std::chrono::duration<std::chrono::milliseconds> refresh_time;
    };

    /**
     * @class Kernel
     * @brief Главный класс программы.
     * Осуществляет с помощью вспомогательного класса AgentLoader подключение новых агентов к системе.
     * Также с заданной переодичностью осуществляет обновление актуальных метрик с помощью и запись их
     */
    class Kernel {
    public:
        /**
         * @brief Инициализация программы:
         * 1. Создается отдельный поток, в котором осуществляется поиск и загрузка новых агентов из ./agents
         * 2. В другом потоке происходит отслеживание конфигураций агентов, чтобы их можно было динамически изменять
         */
        Kernel() noexcept;
        ~Kernel() noexcept; ///< Завершает работу потоков

        /**
         * @brief Записывает лог в журнал после обновления текущих метрик
         */
        void log() noexcept;

        /**
         * @brief Передает сигнал обновления данных метрик ко всем агентам
         */
        void update() noexcept;

        /**
         * @brief Отключает выбранный в списке активный агент.
         * Это значит, что остается загруженным, но его данные отображаются в UI
         * @param name Имя агента, которого нужно отключить
         */
        void disconnectionAgent(const std::string& name) noexcept;

        /**
         * @brief Включает отключенный агент.
         * После этого данные этого агента снова будут отображаться в UI
         * @param name Имя неактивного агента
         */
        void connectionAgent(const std::string& name) noexcept;

        /**
         * @brief Отправляет сообщение на указанный email или в телеграме
         * при достижении какой-то метрики критической отметки
         * @param metric Название метрики
         * @param value Значение этой метрики
         */
        void notifyUser(const std::string& metric, double value);

        /**
         * @brief Возможность включать/выключать дублирование оповещений на указанный email адрес
         * @param is_enable Включено, если true, иначе выключено
         */
        void enableEmailDuplication(bool is_enable) noexcept;

        /**
         * @brief Передает в интерфейс подробную информацию об агенте
         * @param name Имя агента, о котором нужно получить данные
         * @return Подробная информация в виде структуры
         */
        AgentInfo getAgentInfo(const std::string& name) noexcept;

        /**
         * @return true, если при создании ядра не произошло ошибок
         */
        bool isCorrect() noexcept;

    private:
        /**
         * @brief Осуществляет поиск и загрузку новых агентов из директории ./agents.
         * Работает в отдельном потоке
         */
        void _searchNewAgents() noexcept;

        std::unique_ptr<KernelManager> _kernel_manager; ///< Отвечает за всю работу с агентами
        std::unique_ptr<Logger> _logger; ///< Отвечает за запись данных в журнал
        std::unique_ptr<NotificationService> _notification_service; ///< Отвечает за оповещение пользователя

        std::string _users_email; ///< Email адрес пользователя, куда должны приходить сообщения

        std::thread _search_agents_thread; ///< Хранит поток, в котором происходит поиск агентов в директории ./agents
        std::atomic<bool> _work_flag; ///< Атомарный флаг, который показывает, нужно ли продолжать работу потоков
    };
}

#endif