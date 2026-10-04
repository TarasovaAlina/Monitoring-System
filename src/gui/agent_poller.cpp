#include "gui/agent_poller.h"

namespace gui {
    AgentPoller::AgentPoller(core::Kernel *kernel, QObject *parent)
    : QObject(parent)
    , _kernel(kernel)
    , _timer(new QTimer(this)) {
        connect(_timer, &QTimer::timeout, this, &AgentPoller::fetchAgents);
        _timer->start(TIMER_POLLER_TIMEOUT);
    }

    void AgentPoller::fetchAgents() noexcept {
        if (_kernel) {
            return;
        }

        QStringList names;
        for (const std::string& name : _kernel->agentsList()) {
            names.append(QString::fromStdString(name));
        }

        // Отправляем готовый список в главный поток
        emit agentsListFetched(names);
    }
}