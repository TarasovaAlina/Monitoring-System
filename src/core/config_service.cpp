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

    void ConfigService::overwriteAgentName(const std::string &agent_name, const std::string &new_name) noexcept {
        std::ifstream inputFile("config/config.json");
        json config = json::parse(inputFile);

        // Проверяем, чтобы была соответствующая запись в JSON файле
        if (!config.contains(agent_name)) {
            return;
        }

        // Обновляем имя агента в самом конце
        if (agent_name != new_name && !config.contains(new_name)) {
            config[new_name] = std::move(config[agent_name]);
            config.erase(agent_name);

            // Записываем в тот же файл
            std::ofstream outputFile("config/config.json");
            outputFile << config.dump(4);
            outputFile.close();
        }
    }

    void ConfigService::overwriteAgentType(const std::string &agent_name, const AgentType &new_type) noexcept {
        std::ifstream inputFile("config/config.json");
        json config = json::parse(inputFile);

        // Проверяем, чтобы была соответствующая запись в JSON файле
        if (config.contains(agent_name)) {
            config[agent_name]["type"] = _setAgentTypeToStr(new_type);

            // Записываем в тот же файл
            std::ofstream outputFile("config/config.json");
            outputFile << config.dump(4);
            outputFile.close();
        }
    }

    void ConfigService::overwriteAgentMetricsList(const std::string &agent_name, const std::vector<MetricConfig> &critical_metrics_list) noexcept {
        std::ifstream inputFile("config/config.json");
        json config = json::parse(inputFile);

        // Проверяем, чтобы была соответствующая запись в JSON файле
        if (config.contains(agent_name)) {
            auto& agent_settings = config[agent_name];
            json settings;

            // Формируем новый список critical_metrics
            for (auto& metric : critical_metrics_list) {
                json metric_setting;

                metric_setting["operator"] = metric.threshold.operation;
                metric_setting["value"] = metric.threshold.value;

                settings[metric.target] = metric_setting;
            }

            agent_settings["critical_metrics"] = std::move(settings);

            // Записываем в тот же файл
            std::ofstream outputFile("config/config.json");
            outputFile << config.dump(4);
            outputFile.close();
        }
    }

    void ConfigService::overwriteAgentTimeout(const std::string &agent_name, long timeout) noexcept {
        std::ifstream inputFile("config/config.json");
        json config = json::parse(inputFile);

        // Проверяем, чтобы была соответствующая запись в JSON файле
        if (config.contains(agent_name)) {
            config[agent_name]["update_interval"] = timeout;

            // Записываем в тот же файл
            inputFile.close();
            std::ofstream outputFile("config/config.json");
            outputFile << config.dump(4);
            outputFile.close();
        }
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