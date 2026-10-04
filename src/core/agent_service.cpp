#include "core/agent_service.h"
#include <mutex>

namespace core {
    AgentService::~AgentService() noexcept {
        for (auto& [agent_name, agent_core]: _agents_list) {
            agent_core.agent->unload();

            agent_core.agent_work_thread.join();
        }
    }

    void AgentService::update(const std::vector<ConfigInfo> &agent_data_list) noexcept {
        // Захватываем мьютекс на запись
        std::unique_lock<std::shared_mutex> lock(_agents_mutex);

        // Проверяем, были ли такие агенты загружены ранее
        for (auto &config: agent_data_list) {
            if (!_agents_list.contains(config.agentName)) {
                // Загружаем новый агент
                std::string path_agent = "lib" + config.agentName + ".so";
                _agents_list[config.agentName] = AgentCore {};

                _agents_list[config.agentName].agent =
                    std::make_unique<AgentHandler>(path_agent, config.agentType, config.updateInterval.count());

                // Запускаем агент работать в отдельном потоке
                _agents_list[config.agentName].agent_work_thread = std::thread(
                    &AgentHandler::work,
                    _agents_list[config.agentName].agent.get());

                // Передаем критические значения метрик
                _agents_list[config.agentName].critical_metrics_values = config.metricConfig;
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
                if (!found) {
                    // Удаляем обработчик агента и рабочий поток
                    agent.second.agent->unload();
                    agent.second.agent_work_thread.join();

                    _agents_list.erase(agent.first);
                }
            }
        }
    }

    std::unique_ptr<AgentHandler>& AgentService::getAgent(const std::string &name) {
        // Захватываем мьютекс на чтение с помощью мягкой блокировки
        std::shared_lock<std::shared_mutex> lock(_agents_mutex);

        for (auto& agent: _agents_list) {
            if (agent.first == name) {
                // Передаем ссылку на указатель обработчика агента
                return agent.second.agent;
            }
        }

        // Пока не уверен, нужен ли вообще выброс исключения
        throw std::logic_error("Agent \"" + name + "\" not found");
    }

    std::vector<agent::Metric> AgentService::collectMetrics() const noexcept {
        // Захватываем мьютекс на чтение с помощью мягкой блокировки
        std::shared_lock<std::shared_mutex> lock(_agents_mutex);

        std::vector<agent::Metric> metrics{};
        metrics.reserve(_agents_list.size() * 3);

        for (auto& agent: _agents_list) {
            metrics.insert(metrics.end(),
                agent.second.agent->metrics().begin(),
                agent.second.agent->metrics().end());
        }

        return metrics;
    }

    std::vector<std::string> AgentService::agentsNamesList() const noexcept {
        // Захватываем мьютекс на чтение с помощью мягкой блокировки
        std::shared_lock<std::shared_mutex> lock(_agents_mutex);

        std::vector<std::string> names{};
        names.reserve(_agents_list.size());

        for (auto& agent: _agents_list) {
            names.emplace_back(agent.first);
        }

        return names;
    }

    void AgentService::changeName(const std::string &name, const std::string &new_name) noexcept {
        // Захватываем мьютекс на запись
        std::unique_lock<std::shared_mutex> lock(_agents_mutex);

        // Извлекаем узел без лишних копирований
        auto node = _agents_list.extract(name);

        // Изменяем ключ и вставляем обратно
        // Проверка на то, что элемента с новым ключом не существует, проводится на стороне клиента
        node.key() = new_name;
        _agents_list.insert(std::move(node));
    }

    std::vector<MetricConfig> &AgentService::criticalMetricValues(const std::string &name) noexcept {
        // Захватываем мьютекс на запись
        std::unique_lock<std::shared_mutex> lock(_agents_mutex);

        return _agents_list[name].critical_metrics_values;
    }
}
