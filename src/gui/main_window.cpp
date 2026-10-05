#include "gui/main_window.h"
#include "core/kernel.h"
#include "tools/log_reader.h"
#include <QMessageBox>
#include <QTimer>

namespace gui {
    MainWindow::MainWindow(QWidget* parent) noexcept
    : QMainWindow(parent)
    , _log_reader_thread(new QThread(this))
    , _agent_poller_thread(new QThread(this))
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

        _ui->setActiveAgentIndex(0);
        pushCurrentAgentToUI();

        _worker = new Worker();
        _worker->moveToThread(_log_reader_thread);

        connect(_worker, &Worker::updateMetricsList, this, &MainWindow::updatingLogs);
        connect(_log_reader_thread, &QThread::finished, _worker, &QObject::deleteLater);

        _agent_poller = new AgentPoller(_kernel.get());
        _agent_poller->moveToThread(_agent_poller_thread);

        connect(_agent_poller, &AgentPoller::agentsListFetched, this, &MainWindow::reloadAgentList);
        connect(_agent_poller_thread, &QThread::finished, _agent_poller_thread, &QObject::deleteLater);

        _log_reader_thread->start();
        _agent_poller_thread->start();
    }

    MainWindow::~MainWindow() noexcept {
        _log_reader_thread->quit();
        _log_reader_thread->wait();

        _agent_poller_thread->quit();
        _agent_poller_thread->wait();
    }

    void MainWindow::reloadAgentList(const QStringList& agents) noexcept {
        std::unique_lock<std::shared_mutex> lock(_agent_names_mutex);
        _agent_names = agents;
        _ui->setAgentNames(_agent_names);

        qDebug() << "[MainWindow] Обновлен список агентов в UI";
    }

    void MainWindow::updatingLogs(const std::vector<agent::Metric> &metrics_list) noexcept {
        _metrics_list.insert(_metrics_list.end(), metrics_list);

        if (_metrics_list.size() > NUMBER_DISPLAY_ROWS) {
            _metrics_list.erase(_metrics_list.begin());
        }

        _ui->showMetrics(_metrics_list);

        qDebug() << "[MainWindow] Отображены новые метрики в левой панели";
    }

    QString MainWindow::currentAgentName() const noexcept {
        std::shared_lock<std::shared_mutex> lock(_agent_names_mutex);

        return _agent_names.value(_currentAgentIndex);
    }

    void MainWindow::onAgentSelected(int index) noexcept {
        std::shared_lock<std::shared_mutex> lock(_agent_names_mutex);

        if (index < 0 || index >= _agent_names.size())
            return;

        _currentAgentIndex = index;
        qDebug() << "[MainWindow] Выбран агент index = " << index;
        pushCurrentAgentToUI();
    }

    void MainWindow::pushCurrentAgentToUI() const noexcept {
        std::shared_lock<std::shared_mutex> lock(_agent_names_mutex);

        if (!_kernel || _agent_names.isEmpty())
            return;

        const std::string agentName = currentAgentName().toStdString();
        core::AgentInfo info = _kernel->getAgentInfo(agentName);

        _ui->showAgentSettings(QString::fromStdString(agentName), info);
        qDebug() << "[MainWindow] Обновлены данные на панели настроек агента";
    }

    void MainWindow::onAgentEnabledChanged(bool enabled) const noexcept {
        std::shared_lock<std::shared_mutex> lock(_agent_names_mutex);
        if (!_kernel || _agent_names.isEmpty())
            return;

        const std::string agentName = currentAgentName().toStdString();

        if (enabled) {
            _kernel->connectionAgent(agentName);
            qDebug() << "[MainWindow] Агент " << QString::fromStdString(agentName) << " стал активен";
        }
        else {
            _kernel->disconnectionAgent(agentName);
            qDebug() << "[MainWindow] Агент " << QString::fromStdString(agentName) << " отключен";
        }
    }

    void MainWindow::onApplyRequested(const QString& new_name, int index,
        const QList<core::MetricConfig>& critical_metrics_list, long timeout_ms) const noexcept {
        std::shared_lock<std::shared_mutex> lock(_agent_names_mutex);

        if (!_kernel || _agent_names.isEmpty())
            return;

        const std::string curAgentName = currentAgentName().toStdString();

        _kernel->changeAgentSetting(curAgentName, new_name.toStdString());
        _kernel->changeAgentSetting(curAgentName, timeout_ms);

        // Позже заменить на вычисление типа через получения значения от выпадающего списка
        _kernel->changeAgentSetting(curAgentName, static_cast<agent::AgentType>(index));

        const std::vector<core::MetricConfig> vec_metrics(critical_metrics_list.constBegin(), critical_metrics_list.constEnd());
        _kernel->changeAgentSetting(curAgentName, vec_metrics);

        qDebug() << "[MainWindow] Настройки применены к текущему агенту" << QString::fromStdString(curAgentName);
    }
}