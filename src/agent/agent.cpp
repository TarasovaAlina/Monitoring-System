#include "agent/agent.h"

namespace agent {
    Agent::Agent(AgentType type) noexcept : _metrics_collector(nullptr) {
        switchType(type);
    }

    void Agent::updateMetrics() noexcept {
        if (_metrics_collector != nullptr) {
            _metrics = _metrics_collector->update();
        }
    }

    void Agent::switchType(const AgentType &new_type) noexcept {
        if (new_type == CPU_AGENT) {
            _metrics_collector = std::make_unique<CPUMetricsCollector>();
        }
        else if (new_type == MEMORY_AGENT) {
            _metrics_collector = std::make_unique<MemoryMetricsCollector>();
        }
        else if (new_type == NETWORK_AGENT) {
            _metrics_collector = std::make_unique<NetworkMetricsCollector>();
        }

        _type = new_type;
    }

    const std::vector<Metric> &Agent::getMetrics() const noexcept {
        return _metrics;
    }

    AgentType Agent::type() const noexcept {
        return _type;
    }

}