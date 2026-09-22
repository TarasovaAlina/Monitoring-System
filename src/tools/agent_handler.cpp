#include "core/agent_handler.h"
#include <dlfcn.h>
#include <thread>

namespace core {
    AgentHandler::AgentHandler(const std::string &path, agent::AgentType type, int milliseconds)
    : _running(true)
    , _sleeping(false)
    , _type(type)
    , _timeout(milliseconds) {
        _shared_lib = dlopen(path.c_str(), RTLD_LAZY);

        if (_shared_lib == nullptr) {
            throw std::runtime_error("Failed to load library: " + path);
        }

        auto createFunc = static_cast<CreateAgentFunc>(dlsym(_shared_lib, "CreateAgent"));
        auto destroyFunc = static_cast<DestroyAgentFunc>(dlsym(_shared_lib, "DestroyAgent"));

        if (!createFunc || !destroyFunc) {
            throw std::runtime_error("Failed to load symbols from the library: " + path);
        }

        _agent = std::make_unique<agent::IAgent, DestroyAgentFunc>(createFunc(_type), destroyFunc);
    }

    AgentHandler::~AgentHandler() noexcept {
        unload();
    }

    void AgentHandler::work() noexcept {
        while (_running.load()) {
            // Обновление метрик только если агент находится в активном состоянии
            if (!_sleeping.load()) {
                _agent->updateMetrics();

                _metrics = _agent->getMetrics();

                std::this_thread::sleep_for(std::chrono::milliseconds(_timeout));
            }
        }
    }

    void AgentHandler::setSleepMode(bool is_sleep) noexcept {
        if (_sleeping.load() != is_sleep) {
            _sleeping.store(is_sleep);
        }
    }

    void AgentHandler::unload() noexcept {
        if (_shared_lib != nullptr) {
            dlclose(_shared_lib);

            _shared_lib = nullptr;
            _running.store(false);
        }
    }

    agent::AgentType &AgentHandler::type() noexcept {
        return _type;
    }

    std::chrono::milliseconds &AgentHandler::timeout() noexcept {
        return _timeout;
    }

    std::vector<agent::Metric> &AgentHandler::metrics() noexcept {
        return _metrics;
    }

}
