#include "core/kernel.h"

namespace core {
    Kernel::Kernel() noexcept
    : _work_flag(true) {
        try {
            // Пытаемся создать вспомогательные инструменты
            _agent_service = std::make_unique<AgentService>();
            _config_service = std::make_unique<ConfigService>();

            _search_agents_thread = std::thread(&Kernel::_searchNewAgents, this);

        } catch (const std::exception& e) {
            _work_flag.store(false);
        }
    }

    Kernel::~Kernel() noexcept {
        _work_flag.store(false);
    }

    std::vector<std::string> Kernel::agentsList() const noexcept {
        return _agent_service->agentsNamesList();
    }

    AgentInfo Kernel::getAgentInfo(const std::string &name) const noexcept {
        const auto& agent = _agent_service->getAgent(name);

        return AgentInfo {
            agent->type(),
            _agent_service->getAgent(name)->metricsList(),
            agent->timeElapsedSinceStart(),
            agent->timeout(),
            agent->isActive()
        };
    }

    void Kernel::changeAgentSetting(const std::string &old_name, const std::string &new_name) noexcept {
        _agent_service->changeName(old_name, new_name);
        _config_service->overwriteAgentName(old_name, new_name);
    }

    void Kernel::changeAgentSetting(const std::string &name, agent::AgentType new_type) noexcept {
        _agent_service->getAgent(name)->switchType(new_type);
        _config_service->overwriteAgentType(name, new_type);
    }

    void Kernel::changeAgentSetting(const std::string &name, const std::vector<MetricConfig> &critical_metrics_list) noexcept {
        _agent_service->getAgent(name)->metricsList() = critical_metrics_list;
        _config_service->overwriteAgentMetricsList(name, critical_metrics_list);
    }

    void Kernel::changeAgentSetting(const std::string &name, long new_timeout) noexcept {
        _agent_service->getAgent(name)->timeout() = new_timeout;
        _config_service->overwriteAgentTimeout(name, new_timeout);
    }

    void Kernel::disconnectionAgent(const std::string &name) noexcept {
        _agent_service->getAgent(name)->setSleepMode(true);
    }

    void Kernel::connectionAgent(const std::string &name) noexcept {
        _agent_service->getAgent(name)->setSleepMode(false);
    }

    bool Kernel::isCorrect() const noexcept {
        return _work_flag.load();
    }

    void Kernel::_searchNewAgents() noexcept {
        // Происходит поиск до тех пор, пока нет сигнала завершения
        while (_work_flag.load()) {
            auto configs = _config_service->load();

            _agent_service->update(configs);

            std::this_thread::sleep_for(std::chrono::milliseconds(SCAN_TIMEOUT));
        }
    }
}