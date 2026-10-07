#include "agent/agent.h"

namespace agent {
    Agent::Agent(AgentType type) noexcept : _metrics_collector(nullptr) {
        if (type == CPU_AGENT) {
            _metrics_collector = std::make_unique<CPUMetricsCollector>();
        }
        else if (type == MEMORY_AGENT) {
            _metrics_collector = std::make_unique<MemoryMetricsCollector>();
        }
        else if (type == NETWORK_AGENT) {
            _metrics_collector = std::make_unique<NetworkMetricsCollector>();
        }
    }

    const std::vector<Metric> Agent::updateMetrics(const std::vector<std::string>& metric_names_list) const noexcept {
        return _metrics_collector->update(metric_names_list);
    }

    // Экспорт C-функций для возможной загрузки объекта Agent из динамической библиотеки
    extern "C" {
        __attribute__((visibility("default"))) IAgent* CreateAgent(AgentType type) {
            return new Agent(type);
        }

        __attribute__((visibility("default"))) void DestroyAgent(IAgent* agent) {
            delete agent;
        }
    }
}