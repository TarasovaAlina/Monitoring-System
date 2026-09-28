#ifndef SYSTEM_MONITORING_MAIN_WINDOW_H
#define SYSTEM_MONITORING_MAIN_WINDOW_H

#include "gui/ui_window.h"
#include "core/kernel.h"
#include <QMainWindow>
#include <QStringList>
#include <QVector>
#include <memory>

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

// ---------------------------------------------------------------------------
// MainWindow
//
// Управление приложением: владеет ядром (core::Kernel), таймером опроса
// метрик, хранит индекс текущего выбранного агента и переводит данные между
// Kernel и UIWindow. Сами виджеты и разметка живут в UIWindow - здесь их нет.
// ---------------------------------------------------------------------------

namespace gui {
    class MainWindow : public QMainWindow {
        Q_OBJECT

    public:
        explicit MainWindow(QWidget* parent = nullptr);
        ~MainWindow() override = default;

    private slots:
        void onAgentSelected(int index);
        void onAgentEnabledChanged(bool enabled);
        void onApplyRequested(int updateIntervalMs, QVector<core::MetricConfig> criticalValues);
        void onRefreshTimerTick();

    private:
        void reloadAgentList();
        void pushCurrentAgentToUi();
        QString currentAgentName() const;

        std::unique_ptr<core::Kernel> _kernel;
        UIWindow* _ui;

        QStringList _agentNames;
        int _currentAgentIndex = 0;

        QTimer* _refreshTimer = nullptr;
    };
}

#endif