#include "core/agent_handler.h"
#include <dlfcn.h>
#include <thread>

namespace core {
    AgentHandler::AgentHandler(const std::string &path, agent::AgentType type, int milliseconds)
    : _running(true)
    , _sleeping(false)
    , _type(type)
    , _timeout(milliseconds)
    , _agent(nullptr, nullptr) {
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
                _metrics = _agent->updateMetrics(_metric_names_list);

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

    std::vector<agent::Metric> &AgentHandler::metrics() noexcept {
        return _metrics;
    }

    long AgentHandler::timeElapsedSinceStart() const noexcept {
        // Вычисляем, сколько времени прошло с создания агента
        auto end_time = std::chrono::steady_clock::now();

        return std::chrono::duration_cast<std::chrono::milliseconds>(end_time - _start_time).count();
    }

    std::vector<std::string> &AgentHandler::metricNamesList() noexcept {
        return _metric_names_list;
    }

    bool AgentHandler::isActive() const noexcept {
        return _sleeping.load();
    }
}
