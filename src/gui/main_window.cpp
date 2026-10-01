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
            MetricConfig data;

            data.name = QString::fromStdString(metric.name);
            data.value = metric.value;
            data.criticalValue = metric.critical_value;

            metrics.append(data);
        }

        _ui->showMetrics(metrics);
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

    void MainWindow::onApplyRequested(const QString& new_name, int index, const QList<core::MetricConfig>& critical_metrics_list, long timeout_ms) noexcept {
        if (!_kernel || _agentNames.isEmpty())
            return;

        const std::string curAgentName = currentAgentName().toStdString();

        _kernel->changeAgentSetting(curAgentName, new_name.toStdString());
        _kernel->changeAgentSetting(curAgentName, timeout_ms);
        
        // Позже заменить на вычисление типа через получения значения от выпадающего списка
        _kernel->changeAgentSetting(curAgentName, static_cast<agent::AgentType>(index));

        const std::vector<core::MetricConfig> vec_metrics(critical_metrics_list.constBegin(), critical_metrics_list.constEnd());
        _kernel->changeAgentSetting(curAgentName, vec_metrics);
    }
}