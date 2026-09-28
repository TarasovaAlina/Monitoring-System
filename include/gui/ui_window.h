#ifndef SYSTEM_MONITORING_UI_WINDOW_H
#define SYSTEM_MONITORING_UI_WINDOW_H

#include <QFormLayout>
#include <QPushButton>

#include "agent/agent.h"
#include <QString>
#include <QVector>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QButtonGroup;
class QHBoxLayout;
class QVBoxLayout;
class QGroupBox;
class QProgressBar;
class QLabel;
class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QFrame;
QT_END_NAMESPACE

#define NUMBER_DISPLAY_ROWS 20 ///< Количество отображаемых строк на экране

// ---------------------------------------------------------------------------
// UIWindow
//
// Отвечает исключительно за визуальное представление: вкладки агентов,
// панель вывода метрик, панель настроек. Не хранит бизнес-логику и не знает
// про Kernel - только строит виджеты, показывает то, что ей передали, и
// сообщает наружу о действиях пользователя через сигналы.
// ---------------------------------------------------------------------------

namespace gui {
    class UIWindow final : public QWidget {
        Q_OBJECT

    public:
        explicit UIWindow(QWidget* parent = nullptr);

        // --- вызывается снаружи (из MainWindow), чтобы обновить экран ----------
        void setAgentNames(const QStringList& names);
        void setActiveAgentButton(int index);
        void showAgentSettings(int updateIntervalMs, bool enabled);
        void showMetrics(const QVector<agent::Metric>& metrics);

        signals:
        // --- действия пользователя, наружу --------------------------------------
        void agentSelected(int index);
        void agentEnabledChanged(bool enabled);
        void applyRequested(int updateIntervalMs, QVector<MetricConfigData> criticalValues);

    private slots:
        void onApplyButtonClicked();

    private:
        void buildAgentBar();
        void buildMetricsPanel();
        void buildSettingsPanel();
        void rebuildMetricRows(int metricCount);
        void applyThresholdColor(QProgressBar* bar, double value, double criticalValue) const;

        struct MetricRow {
            QFrame* frame;
            QLabel* nameLabel;
            QProgressBar* bar;
            QLabel* thresholdLabel;
            QSpinBox* thresholdSpinBox;
            QLabel* valueLabel;
            QDoubleSpinBox* criticalSpin;
        };

        QWidget* widget_agentBar;

        QHBoxLayout* layout_rootLayout;
        QVBoxLayout* layout_leftColumnLayout;
        QHBoxLayout* layout_agentBarLayout;
        QVBoxLayout* layout_metricsLayout;
        QVBoxLayout* layout_outerLayout;
        QFormLayout* layout_formSettingsPanelLayout;

        QButtonGroup* group_agentButtons;
        QLabel* label_hintLabel;
        QPushButton* button_applyButton;

        QGroupBox* box_metricsPanelGroup;
        QGroupBox* box_settingsGroup;
        QSpinBox* box_intervalSpin;
        QCheckBox* box_enabledCheck;

        QVector<MetricRow> vector_metricRows;
    };
}

#endif