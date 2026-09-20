#include "core/agent_handler.h"
#include <dlfcn.h>
#include <thread>

namespace core {
    AgentHandler::AgentHandler(const std::string &path, agent::AgentType type, int milliseconds)
    : _running(true)
    , _sleeping(false)
    , _type(type)
    , _timeout(milliseconds) {
        _lib_agent = dlopen(path.c_str(), RTLD_LAZY);

        if (_lib_agent == nullptr) {
            throw std::runtime_error("Failed to load library: " + path);
        }

        auto updateFunc = dlsym(_lib_agent, "updateMetrics");
        auto getMetricsFunc = dlsym(_lib_agent, "getMetrics");

        if (!updateFunc || !getMetricsFunc) {
            throw std::runtime_error("Failed to load symbols from the library: " + path);
        }

        _updateMetricsCallback = updateFunc();
        _updateMetricsCallback = getMetricsFunc();
    }

    AgentHandler::~AgentHandler() noexcept {
        unload();
    }

    void AgentHandler::work() noexcept {
        while (_running.load()) {
            // Обновление метрик только если агент находится в активном состоянии
            if (!_sleeping.load()) {
                _updateMetricsCallback();

                _metrics = _gettingMetricsCallback();

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
        if (_lib_agent != nullptr) {
            dlclose(_lib_agent);

            _lib_agent = nullptr;
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
