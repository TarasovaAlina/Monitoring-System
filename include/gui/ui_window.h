#ifndef SYSTEM_MONITORING_UI_WINDOW_H
#define SYSTEM_MONITORING_UI_WINDOW_H

#include <QFormLayout>
#include <QPushButton>

#include "core/config_service.h"
#include "agent/agent.h"
#include <QString>
#include <QVector>
#include <QWidget>

#include "core/kernel.h"

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
class QSplitter;
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

        // Вызывается снаружи (из MainWindow), чтобы обновить экран ----------
        void setAgentNames(const QStringList& names);
        void setActiveAgentIndex(int index);
        void showAgentSettings(const QString& name, const core::AgentInfo& info);
        void showMetrics(const QList<QList<agent::Metric>>& metrics);

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
        void buildMetricsPanel();
        void buildSettingsPanel();
        void rebuildMetricsGrid(const QList<QList<agent::Metric>>& metrics);

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

        QSplitter* splitter_main; ///< Разделитель между областями экрана

        QWidget* widget_metricsContainer; ///< Внутренний контейнер для сетки

        QVBoxLayout* layout_metricsContainer; ///< Слой внутри контейнера
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

        QVector<MetricSettingRow> vector_dynamicMetrics; ///< Хранилище строк метрик
        QVector<QVector<QLabel*>> grid_metrics; ///< Двумерный массив виджетов
    };
}

#endif