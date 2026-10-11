#include "core/config_service.h"

namespace core {
    std::vector<ConfigInfo> ConfigService::load() const {
        std::ifstream file("config/config.json");

        if (!file.is_open()) {
            throw std::runtime_error("Cannot open config file");
        }

        json config;
        file >> config;

        if (!config.contains("agents") || !config["agents"].is_array()) {
            throw std::runtime_error("Invalid config: agents must be an array");
        }

        std::vector<ConfigInfo> result;

        for (const auto& agent : config["agents"]) {
            if (!agent.contains("name") || !agent["name"].is_string()) {
                throw std::runtime_error("Invalid agent name");
            }

            if (!agent.contains("type") || !agent["type"].is_string()) {
                throw std::runtime_error("Invalid agent type");
            }

            if (!agent.contains("update_interval") ||
                !agent["update_interval"].is_number_integer()) {
                throw std::runtime_error("Invalid update interval");
            }

            if (!agent.contains("critical_metrics") ||
                !agent["critical_metrics"].is_array()) {
                throw std::runtime_error("Invalid critical metrics");
            }

            ConfigInfo configInfo{
                agent["name"].get<std::string>(),
                _getAgentTypeFromStr(agent["type"].get<std::string>()),
                static_cast<std::chrono::milliseconds>(agent["update_interval"].get<int>()),
                {}
            };

            for (const auto& metric : agent["critical_metrics"]) {
                if (!metric.contains("name") || !metric["name"].is_string()) {
                    throw std::runtime_error("Invalid metric name");
                }

                if (!metric.contains("operator") ||
                    !metric["operator"].is_string()) {
                    throw std::runtime_error("Invalid metric operator");
                }

                if (!metric.contains("value") ||
                    !metric["value"].is_number()) {
                    throw std::runtime_error("Invalid metric value");
                }

                const std::string target = metric["name"].get<std::string>();
                const std::string operation = metric["operator"].get<std::string>();
                const double value = metric["value"].get<double>();

                if (operation != "<" &&
                    operation != "<=" &&
                    operation != "==" &&
                    operation != ">=" &&
                    operation != ">") {
                    throw std::runtime_error("Invalid metric operator");
                }

                configInfo.metricConfig.push_back(
                    { target, Threshold(operation, value) }
                );
            }

            result.push_back(std::move(configInfo));
        }

        return result;
    }

    void ConfigService::update(const std::string& agent_name, const ConfigInfo& settings) const {
        std::ifstream inputFile("config/config.json");

        if (!inputFile.is_open()) {
            throw std::runtime_error("Cannot open config file");
        }

        json config;
        inputFile >> config;

        if (!config.contains("agents") || !config["agents"].is_array()) {
            throw std::runtime_error("Invalid config: agents must be an array");
        }

        auto agent_iter = std::find_if(config["agents"].begin(), config["agents"].end(), [agent_name](const json& agent) {
            return agent.contains("name") && agent["name"].is_string() && agent["name"].get<std::string> == agent_name;
        });

        if (agent_iter == config["agents"].end()) {
            return;
        }

        auto agent = *agent_iter;

        agent["type"] = _setAgentTypeToStr(settings.agentType);
        agent["update_interval"] = settings.updateInterval.count();

        // Находим лишние записи в JSON
        for (auto& metric : agent["critical_metrics"]) {
            auto crit_value_it = std::find_if(settings.metricConfig.begin(), settings.metricConfig.end(),
                [metric](const MetricConfig& crit_value) {
                return crit_value.target == metric["name"].get<std::string>();
            });

            // Удаляем элемент с неправильным форматом или если его нет в списке
            if (!metric.contains("name") || !metric["name"].is_string() ||
                crit_value_it == settings.metricConfig.end()) {
                agent["critical_metrics"].erase(metric);
            }
        }

        // Обновляем значения активных метрик
        for (auto& crit_value : settings.metricConfig) {
            auto metric_it = std::find_if(agent["critical_metrics"].begin(), agent["critical_metrics"].end(),
                [crit_value](const json& metric) {
                    return crit_value.target == metric["name"].get<std::string>();
                });

            // Обновляем запись, если такая уже есть
            if (metric_it != agent["critical_metrics"].end()) {
                auto metric = *metric_it;

                metric["operator"] = crit_value.threshold.operation;
                metric["value"] = crit_value.threshold.value;
            }

            // Если запись не найдена, добавляем
            else {
                json metric;

                metric["name"] = crit_value.target;
                metric["operator"] = crit_value.threshold.operation;
                metric["value"] = crit_value.threshold.value;

                agent["critical_metrics"].push_back(metric);
            }
        }

        // Обновляем имя агента в самом конце
        agent["name"] = settings.agentName;

        // Записываем новые данные в тот же файл
        std::ofstream outputFile("config/config.json");

        if (!outputFile.is_open()) {
            throw std::runtime_error(
                "Cannot open config file for writing");
        }

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