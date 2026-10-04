#include "gui/main_window.h"
#include "core/kernel.h"
#include "tools/log_reader.h"
#include <QMessageBox>
#include <QTimer>

namespace gui {
    MainWindow::MainWindow(QWidget* parent) noexcept
    : QMainWindow(parent)
    , _log_reader_thread(new QThread(this))
    , _is_running(1)
    , _currentAgentIndex(0) {
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

        _ui->setActiveAgentIndex(0);
        pushCurrentAgentToUI();

        _worker = new Worker();
        _worker->moveToThread(_log_reader_thread);

        connect(_worker, &Worker::updateMetricsList, this, &MainWindow::updatingLogs);
        connect(_log_reader_thread, &QThread::finished, _worker, &QObject::deleteLater);

        _log_reader_thread->start();
    }

    MainWindow::~MainWindow() noexcept {
        _log_reader_thread->quit();
        _log_reader_thread->wait();
    }

    void MainWindow::reloadAgentList() noexcept {
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

    void MainWindow::updatingLogs(const std::vector<agent::Metric> &metrics_list) noexcept {
        _metrics_list.insert(_metrics_list.end(), metrics_list);

        if (_metrics_list.size() > NUMBER_DISPLAY_ROWS) {
            _metrics_list.erase(_metrics_list.begin());
        }

        _ui->showMetrics(_metrics_list);
    }

    QString MainWindow::currentAgentName() const noexcept {
        return _agentNames.value(_currentAgentIndex);
    }

    void MainWindow::onAgentSelected(int index) noexcept {
        if (index < 0 || index >= _agentNames.size())
            return;

        _currentAgentIndex = index;
        pushCurrentAgentToUI();
    }

    void MainWindow::pushCurrentAgentToUI() const noexcept {
        if (!_kernel || _agentNames.isEmpty())
            return;

        const std::string agentName = currentAgentName().toStdString();
        core::AgentInfo info = _kernel->getAgentInfo(agentName);

        _ui->showAgentSettings(QString::fromStdString(agentName), info);
    }

    void MainWindow::onAgentEnabledChanged(bool enabled) const noexcept {
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

    void MainWindow::onApplyRequested(const QString& new_name, int index,
        const QList<core::MetricConfig>& critical_metrics_list, long timeout_ms) const noexcept {

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