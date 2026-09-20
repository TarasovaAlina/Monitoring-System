#include "core/agent_service.h"

namespace core {
    void AgentService::update(const std::vector<ConfigInfo> &agent_data_list) noexcept {
        // Проверяем, были ли такие агенты загружены ранее
        for (auto &config: agent_data_list) {
            if (!_agents_list.contains(config.agentName)) {
                // Загружаем новый агент
                std::string path_agent = "lib" + config.agentName + ".so";
                _agents_list[config.agentName] = std::make_unique<AgentHandler>(
                    // Нужно переделать поле agentType, чтобы оно имело тип agent::AgentType
                    path_agent, static_cast<agent::AgentType>(config.agentType), config.updateInterval);
            }
        }

        // Проверяем, хранятся ли агенты, которые уже удалены пользователем
        if (agent_data_list.size() < _agents_list.size()) {
            for (auto& agent: _agents_list) {
                bool found = false;

                for (auto& config: agent_data_list) {
                    if (config.agentName == agent.first) {
                        found = true;
                        break;
                    }
                }

                // Если агента с таким именем нет в новом списке, значит его удалил пользователь
                if (!found) _agents_list.erase(agent.first);
            }
        }
    }

    AgentInfo AgentService::getAgentInfo(const std::string &name) {
        for (auto& agent: _agents_list) {
            if (agent.first == name) {
                // Формируется информация о нужном агенте
                // return AgentInfo {
                //     agent.second.get()->type(),
                //     agent.second.get()->metrics(),
                //     agent.second.get()->timeout()
                // }
            }
        }

        // Пока не уверен, нужен ли вообще выброс исключения
        throw std::logic_error("Agent \"" + name + "\" not found");
    }

    std::vector<agent::Metric> AgentService::collectMetrics() noexcept {
        std::vector<agent::Metric> metrics{};
        metrics.reserve(_agents_list.size() * 3);

        for (auto& agent: _agents_list) {
            metrics.insert(metrics.end(),
                agent.second.get()->metrics().begin(),
                agent.second.get()->metrics().end());
        }

        return metrics;
    }

    void AgentService::enable(const std::string &name) noexcept {
        auto agent = _agents_list.find(name);

        if (agent == _agents_list.end()) {
            throw std::logic_error("Agent \"" + name + "\" not found");
        }

        agent->second->setSleepMode(false);
    }

    void AgentService::disable(const std::string &name) noexcept {
        auto agent = _agents_list.find(name);

        if (agent == _agents_list.end()) {
            throw std::logic_error("Agent \"" + name + "\" not found");
        }

        agent->second->setSleepMode(true);
    }

}