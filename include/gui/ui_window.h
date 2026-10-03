#ifndef SYSTEM_MONITORING_UI_WINDOW_H
#define SYSTEM_MONITORING_UI_WINDOW_H

#include <QFormLayout>
#include <QPushButton>

#include "core/config_service.h"
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
class QComboBox;
class QLineEdit;
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
        void applyRequested(const QString& agentName, int agentTypeIndex, const QList<core::MetricConfig>& metrics, long updateIntervalMs);

    private slots:
        void onApplyButtonClicked();
        void addMetricSettingRow(const QString& name = "", const QString& condition = ">", double value = 0.0) noexcept;
        void onAgentSelectorChanged(int index) noexcept;

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

        /**
         * @struct MetricSettingRow
         * @brief Структура для хранения указателей на элементы динамической строки
         */
        struct MetricSettingRow {
            QWidget* containerWidget;
            QLineEdit* nameEdit;
            QComboBox* conditionCombo;
            QDoubleSpinBox* valueSpin;
        };

        QWidget* widget_agentBar;

        QHBoxLayout* layout_rootLayout;
        QVBoxLayout* layout_leftColumnLayout;
        QHBoxLayout* layout_agentBarLayout;
        QVBoxLayout* layout_metricsLayout;
        QVBoxLayout* layout_outerLayout;

        QButtonGroup* group_agentButtons;

        QGroupBox* box_metricsPanelGroup;
        QGroupBox* box_settingsGroup;

        // Элементы управления для панели настроек
        QComboBox* combo_agentSelector;
        QCheckBox* box_enabledCheck;
        QWidget* widget_settingsDetails; // Контейнер для настроек ниже чекбокса
        QComboBox* combo_agentType;
        QLineEdit* edit_agentName;
        QSpinBox* box_intervalSpin;
        QVBoxLayout* layout_metricsSettings;
        QPushButton* button_addMetric;
        QPushButton* button_applyButton;

        QVector<MetricSettingRow> vector_dynamicMetrics; // Хранилище строк метрик
    };
}

#endif