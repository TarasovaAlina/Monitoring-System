#include "core/config_service.h"

namespace core {
    std::vector<ConfigInfo> ConfigService::load() const noexcept {
        std::vector<ConfigInfo> result;
        std::ifstream file("config/config.json");

        json agents = json::parse(file);

        for (auto it = agents.begin(); it != agents.end(); ++it) {
            ConfigInfo agent_config;
            auto& agent_settings = it.value();

            agent_config.agentName = it.key();

            agent_config.agentType =
                (agent_settings.contains("type") && agent_settings["type"].is_string())
                ? _getAgentTypeFromStr(agent_settings["type"].get<std::string>())
                : agent::CPU_AGENT;

            agent_config.updateInterval =
                (agent_settings.contains("update_interval") && agent_settings["update_interval"].is_number())
                ? std::chrono::milliseconds(agent_settings["update_interval"].get<int>())
                : std::chrono::milliseconds(100);

            if (agent_settings.contains("critical_metrics")) {
                std::vector<MetricConfig> metric_configs;

                // Считываем все метрики и их критические значения для этого агента
                for (auto metric_it = agent_settings["critical_metrics"].begin();
                    metric_it != agent_settings["critical_metrics"].end();
                    ++metric_it) {

                    MetricConfig config;
                    config.target = metric_it.key();

                    auto config_settings = metric_it.value();
                    config.threshold.operation =
                        (config_settings.contains("operator") && config_settings["operator"].is_string())
                        ? config_settings["operator"].get<std::string>()
                        : "==";

                    config.threshold.value =
                        (config_settings.contains("value") && config_settings["value"].is_number())
                        ? config_settings["value"].get<double>()
                        : 0.0;

                    metric_configs.push_back(config);
                }

                agent_config.metricConfig = std::move(metric_configs);
            }

            result.push_back(std::move(agent_config));
        }

        return result;
    }

    void ConfigService::update(const std::string& agent_name, const ConfigInfo& settings) const noexcept {
        std::ifstream inputFile("config/config.json");
        json config = json::parse(inputFile);

        // Находим в записях JSON файла необходимый агент
        if (!config.contains(agent_name)) {
            return;
        }

        auto agent_setting = config[agent_name];

        agent_setting["type"] = _setAgentTypeToStr(settings.agentType);
        agent_setting["update_interval"] = settings.updateInterval.count();

        auto metric_setting = agent_setting["critical_metrics"];

        // Находим лишние записи в JSON
        for (auto metric_it = metric_setting.begin(); metric_it != metric_setting.end(); ++metric_it) {
            auto metric_name = metric_it.key();
            auto crit_value_it =
                std::find_if(settings.metricConfig.begin(), settings.metricConfig.end(),
                [metric_name](const MetricConfig& crit_value) {
                return crit_value.target == metric_name;
            });

            // Удаляем элемент, если его нет в списке
            if (crit_value_it == settings.metricConfig.end()) {
                metric_setting.erase(metric_it);
            }
        }

        // Проходимся по активным метрикам
        for (auto& crit_value : settings.metricConfig) {
            // Обновляем запись, если такая уже есть
            if (metric_setting.contains(crit_value.target)) {
                metric_setting[crit_value.target]["operator"] = crit_value.threshold.operation;
                metric_setting[crit_value.target]["value"] = crit_value.threshold.value;
            }

            // Если запись не найдена, добавляем
            else {
                json metric;

                metric["name"] = crit_value.target;
                metric["operator"] = crit_value.threshold.operation;
                metric["value"] = crit_value.threshold.value;

                metric_setting.push_back(metric);
            }
        }

        // Обновляем имя агента в самом конце
        if (agent_name != settings.agentName && !config.contains(settings.agentName)) {
            config[settings.agentName] = std::move(config[agent_name]);
            config.erase(agent_name);
        }

        // Записываем новые данные в тот же файл
        std::ofstream outputFile("config/config.json");
        outputFile << config.dump(4);
    }

    const AgentType ConfigService::_getAgentTypeFromStr(const std::string& agentType) const noexcept {
        AgentType agentTypeRes{};

        if (agentType == "CPU_AGENT")
            agentTypeRes = agent::CPU_AGENT;
        else if (agentType == "MEMORY_AGENT")
            agentTypeRes = agent::MEMORY_AGENT;
        else if (agentType == "NETWORK_AGENT")
            agentTypeRes = agent::NETWORK_AGENT;

        return agentTypeRes;
    }

    const std::string ConfigService::_setAgentTypeToStr(const AgentType &type) const noexcept {
        switch(type) {
            case agent::CPU_AGENT:
                return "CPU_AGENT";
            case agent::MEMORY_AGENT:
                return "MEMORY_AGENT";
            case agent::NETWORK_AGENT:
                return "NETWORK_AGENT";
        }
    }

}