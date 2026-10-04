#ifndef SYSTEM_MONITORING_AGENT_POLLER_H
#define SYSTEM_MONITORING_AGENT_POLLER_H

#include "core/kernel.h"
#include <QTimer>

#define TIMER_POLLER_TIMEOUT 5

namespace gui {
    class AgentPoller final : public QObject {
        Q_OBJECT
    public:
        explicit AgentPoller(core::Kernel* kernel, QObject* parent = nullptr);

    private slots:
        void fetchAgents() noexcept;

    signals:
        /**
         * @brief Сигнал к MainWindow о том, чтобы обновить текущий список агентов
         * @param agents Список имен агентов, которые в данный момент загружены и работают в Kernel
         */
        void agentsListFetched(const QStringList& agents) noexcept;

    private:
        QTimer* _timer; ///< Таймер, который обеспечивает периодическое получение данных от Kernel
        core::Kernel* _kernel; ///< Указатель на Kernel, полученный из MainWindow. У него класс получает данные об агентах
    };
}

#endif