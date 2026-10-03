#include "gui/worker.h"

namespace gui {
    Worker::Worker(QObject *parent)
    : QObject(parent)
    , _log_reader(std::make_unique<tools::LogReader>())
    , _timer(new QTimer(this)) {
        connect(_timer, &QTimer::timeout, this, &Worker::readingLogs);
        _timer->start(TIMER_TIMEOUT);
    }

    void Worker::readingLogs() noexcept {
        if (_log_reader->isUpdated()) {
            auto data = _log_reader->readData();

            emit updateMetricsList(data.second);
        }
    }
}