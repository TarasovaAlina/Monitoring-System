#include "core/agent_handler.h"
#include <dlfcn.h>
#include <thread>

namespace core {
    AgentHandler::AgentHandler(const std::string &path, agent::AgentType type, int milliseconds)
    : _running(true)
    , _sleeping(false)
    , _type(type)
    , _timeout(milliseconds)
    , _agent(nullptr, nullptr)
    , _logger(std::make_unique<tools::Logger>())
    , _notification_service(std::make_unique<tools::NotificationService>()) {
        _shared_lib = dlopen(path.c_str(), RTLD_LAZY);

        if (_shared_lib == nullptr) {
            throw std::runtime_error("Failed to load library: " + path);
        }

        auto createFunc = (CreateAgentFunc)dlsym(_shared_lib, "CreateAgent");
        auto destroyFunc = (DestroyAgentFunc)dlsym(_shared_lib, "DestroyAgent");

        if (!createFunc || !destroyFunc) {
            throw std::runtime_error("Failed to load symbols from the library: " + path);
        }

        _agent_func = std::make_pair(createFunc, destroyFunc);
        _agent = std::unique_ptr<agent::IAgent, DestroyAgentFunc>(createFunc(_type), destroyFunc);
        _start_time = std::chrono::steady_clock::now();
    }

    AgentHandler::~AgentHandler() noexcept {
        unload();
    }

    void AgentHandler::work() noexcept {
        while (_running.load()) {
            // Обновление метрик только если агент находится в активном состоянии
            if (!_sleeping.load()) {
                std::vector<std::string> metric_names;
                metric_names.reserve(_metrics_list.size());

                for (const auto& metric : _metrics_list) {
                    metric_names.emplace_back(metric.target);
                }

                auto data = _agent->updateMetrics(metric_names);
                _logger->log(data);

                // Сравниваем полученные данные с критическими значениями
                for (auto metric_res : data) {
                    // Находим запись об этой метрике
                    auto metric_it = std::ranges::find_if(_metrics_list.begin(), _metrics_list.end(), [&metric_res](const auto& iter) {
                        return metric_res.name == iter->target;
                    });

                    auto operation = metric_it->threshold.operation;
                    auto crit_value = metric_it->threshold.value;

                    if (operation == "<" && metric_res.value < crit_value ||
                        operation == ">" && metric_res.value > crit_value ||
                        operation == "<=" && metric_res.value >= crit_value ||
                        operation == ">=" && metric_res.value <= crit_value ||
                        operation == "==" && metric_res.value >= crit_value) {
                        _notification_service->notify(metric_res.name, metric_res.value);
                    }
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(_timeout));
            }
        }
    }

    void AgentHandler::setSleepMode(bool is_sleep) noexcept {
        if (_sleeping.load() != is_sleep) {
            _sleeping.store(is_sleep);
        }
    }

    void AgentHandler::switchType(const agent::AgentType &type) noexcept {
        if (type != _type) {
            _type = type;
            _agent = std::unique_ptr<agent::IAgent, DestroyAgentFunc>(_agent_func.first(_type), _agent_func.second);
        }
    }

    void AgentHandler::unload() noexcept {
        if (_shared_lib != nullptr) {
            _agent.reset();
            dlclose(_shared_lib);

            _shared_lib = nullptr;
            _running.store(false);
        }
    }

    agent::AgentType AgentHandler::type() const noexcept {
        return _type;
    }

    long &AgentHandler::timeout() noexcept {
        return _timeout;
    }

    long AgentHandler::timeElapsedSinceStart() const noexcept {
        // Вычисляем, сколько времени прошло с создания агента
        auto end_time = std::chrono::steady_clock::now();

        return std::chrono::duration_cast<std::chrono::milliseconds>(end_time - _start_time).count();
    }

    std::vector<MetricConfig> &AgentHandler::metricsList() noexcept {
        return _metrics_list;
    }

    bool AgentHandler::isActive() const noexcept {
        return !_sleeping.load();
    }
}
