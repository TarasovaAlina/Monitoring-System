#ifndef SYSTEM_MONITORING_WORKER_H
#define SYSTEM_MONITORING_WORKER_H

#include "tools/log_reader.h"
#include <QTimer>

#define TIMER_TIMEOUT 50

namespace gui {
    class Worker final : public QObject {
        Q_OBJECT
    public:
        explicit Worker(QObject* parent = nullptr);

    private slots:
        void readingLogs() noexcept;

    signals:
        /**
         * @brief Сигнал к MainWindow о том, чтобы обновить таблицу метрик для вывода на UI
         * @param metricsList Список метрик, считанный из файла с логами
         */
        void updateMetricsList(const std::vector<agent::Metric>& metricsList) noexcept;

    private:
        QTimer* _timer; ///< Таймер, который обеспечивает периодическую работу класса LogReader
        std::unique_ptr<tools::LogReader> _log_reader; ///< Отвечает за считывание данных из файла с логами
    };
}

#endif