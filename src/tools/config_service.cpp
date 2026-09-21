#include "tools/config_service.h"

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
            getAgentType(agent["type"].get<std::string>()),
            agent["update_interval"].get<int>(),
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

            configInfo.metricConfig_.push_back(
                { target, Threshold(operation, value) }
            );
        }

        result.push_back(std::move(configInfo));
    }

    return result;
}

AgentType ConfigService::getAgentType(std::string& agentType) const noexcept {
    AgentType agentTypeRes{};

    if (agentType == "CPU_AGENT")
        agentTypeRes = CPU_AGENT;
    else if (agentType == "MEMORY_AGENT")
        agentTypeRes = MEMORY_AGENT;
    else if (agentType == "NETWORK_AGENT")
        agentTypeRes = NETWORK_AGENT;
    
    return agentTypeRes;
}