#include "gui/main_window.h"
#include "core/kernel.h"
#include <QMessageBox>
#include <QTimer>

// ---------------------------------------------------------------------------
// ВНИМАНИЕ. В присланном kernel.h нет содержимого agent_service.h, где
// объявлены AgentInfo, MetricConfig и agent::AgentType. Здесь по-прежнему
// предполагаются поля AgentInfo.name/is_active/update_timeout/metrics и
// MetricConfig.metric_name/critical_value (см. pushCurrentAgentToUI() и
// onApplyRequested()). Пришлите agent_service.h - поправлю точные имена.
// ---------------------------------------------------------------------------

namespace gui {
    MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
        setWindowTitle(QStringLiteral("Системный монитор"));
        resize(900, 520);

        _kernel = std::make_unique<core::Kernel>();
        if (!_kernel->isCorrect()) {
            QMessageBox::critical(this,
                                   QStringLiteral("Ошибка"),
                                   QStringLiteral("Не удалось инициализировать ядро (Kernel)."));
        }

        _ui = new UIWindow(this);
        setCentralWidget(_ui);

        connect(_ui, &UIWindow::agentSelected, this, &MainWindow::onAgentSelected);
        connect(_ui, &UIWindow::agentEnabledChanged, this, &MainWindow::onAgentEnabledChanged);
        connect(_ui, &UIWindow::applyRequested, this, &MainWindow::onApplyRequested);

        reloadAgentList();

        _refreshTimer = new QTimer(this);
        connect(_refreshTimer, &QTimer::timeout, this, &MainWindow::onRefreshTimerTick);
        _refreshTimer->start(SCAN_TIMEOUT);

        _currentAgentIndex = 0;
        _ui->setActiveAgentButton(0);
        pushCurrentAgentToUI();
    }

    void MainWindow::reloadAgentList() {
        _agentNames.clear();
        if (!_kernel)
            return;

        for (const std::string& name : _kernel->agentsList())
            _agentNames.append(QString::fromStdString(name));

        if (_agentNames.isEmpty()) {
            // Чтобы окно не оказалось пустым, пока агенты ещё не загрузились
            // фоновым потоком поиска (_searchNewAgents).
            _agentNames = { QStringLiteral("<Нет доступных агентов>") };
        }

        _ui->setAgentNames(_agentNames);
    }

    QString MainWindow::currentAgentName() const {
        return _agentNames.value(_currentAgentIndex);
    }

    void MainWindow::onAgentSelected(int index) {
        if (index < 0 || index >= _agentNames.size())
            return;

        _currentAgentIndex = index;
        pushCurrentAgentToUI();
    }

    void MainWindow::pushCurrentAgentToUI() {
        if (!_kernel || _agentNames.isEmpty())
            return;

        const std::string agentName = currentAgentName().toStdString();
        core::AgentInfo info = _kernel->getAgentInfo(agentName);

        _ui->showAgentSettings(info.refresh_time_ms, info.is_active);

        QVector<agent::Metric> metrics;
        metrics.reserve(static_cast<int>(info.metrics.size()));
        for (const auto& metric : info.metrics) {
            MetricDisplayData data;

            data.name = QString::fromStdString(metric.name);
            data.value = metric.value;
            data.criticalValue = metric.critical_value;

            metrics.append(data);
        }

        _ui->showMetrics(metrics);
    }

    void MainWindow::onRefreshTimerTick() {
        if (_kernel)
            _kernel->update();

        pushCurrentAgentToUI();
    }

    void MainWindow::onAgentEnabledChanged(bool enabled) {
        if (!_kernel || _agentNames.isEmpty())
            return;

        const std::string agentName = currentAgentName().toStdString();

        if (enabled) {
            _kernel->connectionAgent(agentName);
        }
        else {
            _kernel->disconnectionAgent(agentName);
        }
    }

    void MainWindow::onApplyRequested(int updateIntervalMs, QVector<MetricConfigData> criticalValues) {
        if (!_kernel || _agentNames.isEmpty())
            return;

        const std::string agentName = currentAgentName().toStdString();

        // 1) период обновления метрик
        _kernel->changeAgentSetting(agentName, updateIntervalMs);

        // 2) критические значения метрик - по одному MetricConfig на каждую
        //    метрику, которую отдал UIWindow.
        std::vector<core::MetricConfig> criticalList;
        criticalList.reserve(static_cast<std::size_t>(criticalValues.size()));
        for (const MetricConfigData& item : criticalValues) {
            core::MetricConfig config;
            config.metric_name    = item.name.toStdString();
            config.critical_value = item.criticalValue;
            criticalList.push_back(config);
        }
        _kernel->changeAgentSetting(agentName, criticalList);

        pushCurrentAgentToUI();
    }
}