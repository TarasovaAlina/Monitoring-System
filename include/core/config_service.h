#ifndef SYSTEM_MONITORING_CONFIG_SERVICE_H
#define SYSTEM_MONITORING_CONFIG_SERVICE_H

#include "agent/agent.h"
#include <fstream>
#include <vector>
#include <nlohmann/json.hpp>

/**
 * @file config_service.h
 * @brief В этом файле описан класс сервера,
 * который будет возвращать данные из config для дальнейшей работы с агентами
 * @author Tarasova Alina
 * @date 18.09.2026
 */

namespace core {
    using nlohmann::json;
    using agent::AgentType;

    /**
     * @struct Threshold
     * @brief Содержит информацию о значении конкретной метрике и операторе сравнения
     */
    struct Threshold {
        std::string operation; ///< Оператор сравнения (<, <=, ==, >=, >)
        double value; ///< Значение метрики
    };

    /**
     * @struct MetricConfig
     * @brief Содержит информацию об отдельной метрике
     */
    struct MetricConfig {
        std::string target; ///< Название метрики
        Threshold threshold; ///< Структура с численным значением и оператором сравнения
    };

    /**
     * @struct ConfigInfo
     * @brief Содержит информацию о конкретном агенте
     */
    struct ConfigInfo {
        std::string agentName; ///< Имя агента
        AgentType agentType; ///< Тип агента
        std::chrono::milliseconds updateInterval; ///< Время обновления метрик
        std::vector<MetricConfig> metricConfig; ///< Список критических значений метрик
    };

    /**
     * @class ConfigService
     * @brief Класс по работе с JSON конфигурации агентов
     */
    class ConfigService {
    public:
        /**
         * @brief Обработка JSON файла для последующего создания агентов со считанными настройками.
         *
         * Шаблон файла конфигурации:
         * - 1-й уровень: имя агента. Будет загружена динамическая библиотека @code./agents/lib{имя_агента}.so@endcode
         * - 2-й уровень: настройки агента. Здесь считаются поля type, update_interval, critical_metrics
         * (если какого-то поля не будет, то установится значение по умолчанию)
         * - 3-й уровень: имя метрики. Считывается и записывается в общий список метрик critical_metrics
         * - 4-й уровень: настройка метрики. Здесь считываются поля operator и value из структуры Threshold
         * @return Вектор с информацией для каждого агента с соответствующими параметрами
         * @note Файл для считывания - @code./config/config.json@endcode
         */
        std::vector<ConfigInfo> load() const noexcept;

        /**
         * @brief Изменяет в JSON файле имя агента, под которым сохранены настройки текущего агента
         * @param agent_name Текущее имя агента
         * @param new_name Новое имя агента
         */
        void overwriteAgentName(const std::string& agent_name, const std::string& new_name) noexcept;

        /**
         * @brief Изменяет в JSON файле запись о типе агента
         * @param agent_name Имя конкретного агена
         * @param new_type Новый тип этого агента
         */
        void overwriteAgentType(const std::string& agent_name, const AgentType& new_type) noexcept;

        /**
         * @brief Изменяет в JSON файле записи о критических значениях собираемых метрик
         * @param agent_name Имя конкретного агента
         * @param critical_metrics_list Список критических значений метрик
         */
        void overwriteAgentMetricsList(const std::string& agent_name, const std::vector<MetricConfig>& critical_metrics_list) noexcept;

        /**
         * @brief Изменяет в JSON файле запись о времени обновления метрик агента
         * @param agent_name Имя конкретного агента
         * @param timeout Новое время
         */
        void overwriteAgentTimeout(const std::string& agent_name, long timeout) noexcept;

    private:
        /**
         * @brief Вычисляет тип агента по записи в JSON файле
         * @param str_type Строчное написание типа
         * @return Тип агента в виде перечисления agent::AgentType
         */
        const AgentType _getAgentTypeFromStr(const std::string& str_type) const noexcept;

        /**
         * @brief Определяет запись типа агента в JSON файле
         * @param type Тип агента в виде перечисления agent::AgentType
         * @return Строчное написание, пригодное для записи в файл
         */
        const std::string _setAgentTypeToStr(const AgentType& type) const noexcept;
    };

}

#endif