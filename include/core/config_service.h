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
     * @brief Класс по работе с config файлом
     */
    class ConfigService {
    public:
        /**
         * @brief Обработка config файла для последующей загрузки в агентов
         * @return вектор с информацией для каждого агента с соответствующими параметрами
         */
        std::vector<ConfigInfo> load() const;

        /**
         * @brief Изменяет настройки конкретной метрики конкретного агента в config-файле
         * @param agent_name Имя агента, настройки метрики которого необходимо изменить
         * @param metric_config Новые настройки метрики: имя, оператор сравнения и значение
         * @return true, если метрика успешно изменена, иначе false
         *
         * Использование:
         * @code
         * MetricConfig metric {
         *     "ram",
         *     {">=", 95}
         * };
         *
         * configService.updateMetric("memory", metric); @endcode
         */
        bool updateMetric(const std::string& agent_name, const ConfigInfo& settings) const;

    private:
        /**
         * @brief Вычисляет тип агента по записи в JSON файле
         * @param str_type Строчное написание типа
         * @return Тип агента в виде перечисления AgentType
         */
        const AgentType _getAgentType(const std::string& str_type) const noexcept;
    };

}

#endif