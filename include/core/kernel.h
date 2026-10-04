#ifndef SYSTEM_MONITORING_KERNEL_H
#define SYSTEM_MONITORING_KERNEL_H

#include "agent_service.h"
#include "tools/logger.h"
#include "config_service.h"
#include <memory>
#include <thread>

/**
 * @file kernel.h
 * @brief В этом файле описан класс Kernel и структура AgentInfo
 * @date 13.09.2026
 * @authors Tarasova Alina, Georgiy Kovalev
 */

#define SCAN_TIMEOUT 5000

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
        std::vector<MetricConfig> critical_metrics; ///< Список поддерживаемых метрик с их критическими значениями
        long elapsed_time_ms; ///< Время, прошедшее с момента создания агента (мс)
        long refresh_time_ms; ///< Таймаут изменения метрик (мс)
        bool is_active; ///< Активен ли агент (выводятся ли его метрики на экран)
    };

    /**
     * @class Kernel
     * @brief Главный класс программы.\n
     * Осуществляет с помощью класса AgentService подключение новых агентов к системе.
     * Также с заданной переодичностью с помощью класса ConfigService осуществляет обновление актуальных метрик и запись их в журнал
     *
     * Поддерживаемый функционал:
     * - Получение списка загруженных агентов
     * - Получение подробной информации об агенте
     * - Изменение конфигурации уже запущенного агента:
     *  - Имя агента
     *  - Тип агента
     *  - Список крит.значений метрик
     *  - Время обновления метрик
     * - Отключение/включение выбранного агента
     * - Уведомление пользователя, если значения достигли критических значений
     * - Включение/выключение дублирования оповещения на указанный email
     * - Менять email пользователя
     *
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
         * @brief Передает список загруженных агентов
         * @return Список имен агентов
         */
        std::vector<std::string> agentsList() const noexcept;

        /**
         * @brief Передает в интерфейс подробную информацию об агенте
         * @param name Имя агента, о котором нужно получить данные
         * @return Подробная информация в виде структуры
         */
        AgentInfo getAgentInfo(const std::string& name) noexcept;

        /**
         * @brief Меняет имя конкретного агента
         * @param old_name Старое имя агента
         * @param new_name Новое имя агента
         */
        void changeAgentSetting(const std::string& old_name, const std::string& new_name) noexcept;

        /**
         * Меняет тип агента
         * @param name Имя конкретного агента
         * @param new_type Новый тип этого агента
         */
        void changeAgentSetting(const std::string& name, agent::AgentType new_type) noexcept;

        /**
         * Меняет критические значения метрик агента
         * @param name Имя конкретного агента
         * @param critical_metrics_list Список критических значений метрик
         */
        void changeAgentSetting(const std::string& name, const std::vector<MetricConfig>& critical_metrics_list) noexcept;

        /**
         * Меняет время обновления метрик агента
         * @param name Имя конкретного агента
         * @param new_timeout Новое время
         */
        void changeAgentSetting(const std::string& name, long new_timeout) noexcept;

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
        void enableNotificationsDuplication(bool is_enable) noexcept;

        /**
         * @brief Установление нового email адреса, куда программа будет присылать предупреждения
         * @param email Корректный email адрес
         */
        void setEmail(const std::string& email) noexcept;

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

        std::unique_ptr<AgentService> _agent_service; ///< Управляет агентами, загруженными как динамические библиотеки
        std::unique_ptr<ConfigService> _config_service; ///< Считывает конфиги агентов и передает внутренние данные
        std::unique_ptr<tools::Logger> _logger; ///< Отвечает за запись данных в журнал

        std::thread _search_agents_thread; ///< Хранит поток, в котором происходит поиск агентов в директории ./agents
        std::atomic<bool> _work_flag; ///< Атомарный флаг, который показывает, нужно ли продолжать работу потоков
    };
}

#endif