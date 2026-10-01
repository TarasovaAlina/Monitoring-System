#include "gui/ui_window.h"
#include "core/config_service.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>

namespace gui {
    UIWindow::UIWindow(QWidget* parent) : QWidget(parent) {
        layout_rootLayout = new QHBoxLayout(this);

        // Левая колонка
        layout_leftColumnLayout = new QVBoxLayout();

        buildAgentBar();
        layout_leftColumnLayout->addWidget(widget_agentBar, 0);

        buildMetricsPanel();
        layout_leftColumnLayout->addWidget(box_metricsPanelGroup, 1);

        layout_rootLayout->addLayout(layout_leftColumnLayout, 4);

        // Правая колонка
        buildSettingsPanel();
        layout_rootLayout->addWidget(box_settingsGroup, 1);
    }

    void UIWindow::buildAgentBar() {
        widget_agentBar = new QWidget(this);
        layout_agentBarLayout = new QHBoxLayout(widget_agentBar);
        layout_agentBarLayout->setContentsMargins(0, 0, 0, 0);
        layout_agentBarLayout->setSpacing(0);

        group_agentButtons = new QButtonGroup(this);
        group_agentButtons->setExclusive(true);

        connect(group_agentButtons, &QButtonGroup::idClicked, this, &UIWindow::agentSelected);
    }

    void UIWindow::setAgentNames(const QStringList& names) {
        // Полностью пересобираем панель вкладок (безопасно и для первого
        // вызова, и для случая, когда список агентов изменился).
        QLayoutItem* item;
        while ((item = layout_agentBarLayout->takeAt(0)) != nullptr) {
            if (auto* button = qobject_cast<QAbstractButton*>(item->widget())) {
                group_agentButtons->removeButton(button);
                button->deleteLater();
            }
            delete item;
        }

        for (int i = 0; i < names.size(); ++i) {
            auto* button = new QPushButton(names.at(i), this);
            button->setCheckable(true);
            button->setMinimumHeight(28);
            button->setStyleSheet(
                "QPushButton { border: 1px solid #333; padding: 4px 16px; }"
                "QPushButton:checked { background-color: #d8d8d8; font-weight: bold; }");
            group_agentButtons->addButton(button, i);
            layout_agentBarLayout->addWidget(button);
        }
        layout_agentBarLayout->addStretch(1);
    }

    void UIWindow::setActiveAgentButton(int index) {
        if (auto* button = group_agentButtons->button(index))
            button->setChecked(true);
    }

    void UIWindow::buildMetricsPanel() {
        box_metricsPanelGroup = new QGroupBox(QStringLiteral("Вывод метрик на экран"), this);

        layout_metricsLayout = new QVBoxLayout(box_metricsPanelGroup);
        layout_metricsLayout->addStretch(1);
    }

    void UIWindow::buildSettingsPanel() {
        box_settingsGroup = new QGroupBox(QStringLiteral("Настройка агента"), this);

        layout_formSettingsPanelLayout = new QFormLayout();

        box_intervalSpin = new QSpinBox(box_settingsGroup);
        box_intervalSpin->setRange(100, 60000);
        box_intervalSpin->setSingleStep(100);
        box_intervalSpin->setSuffix(QStringLiteral(" мс"));
        layout_formSettingsPanelLayout->addRow(QStringLiteral("Обновление:"), box_intervalSpin);

        box_enabledCheck = new QCheckBox(QStringLiteral("Агент активен"), box_settingsGroup);
        connect(box_enabledCheck, &QCheckBox::toggled, this, &UIWindow::agentEnabledChanged);
        layout_formSettingsPanelLayout->addRow(QString(), box_enabledCheck);

        label_hintLabel = new QLabel(
            QStringLiteral("Критическое значение задаётся отдельно для каждой "
                            "метрики - в её строке слева."),
            box_settingsGroup);
        label_hintLabel->setWordWrap(true);
        label_hintLabel->setStyleSheet("color: #666; font-size: 11px;");

        button_applyButton = new QPushButton(QStringLiteral("Применить"), box_settingsGroup);
        connect(button_applyButton, &QPushButton::clicked, this, &UIWindow::onApplyButtonClicked);

        layout_outerLayout = new QVBoxLayout(box_settingsGroup);
        layout_outerLayout->addLayout(layout_formSettingsPanelLayout);
        layout_outerLayout->addWidget(label_hintLabel);
        layout_outerLayout->addStretch(1);
        layout_outerLayout->addWidget(button_applyButton);
    }

    void UIWindow::onApplyButtonClicked() {
        QVector<core::MetricConfig> criticalValues;
        criticalValues.reserve(vector_metricRows.size());

        for (const MetricRow& row : vector_metricRows) {
            criticalValues.append(core::MetricConfig {
                row.nameLabel->text().toStdString(),
                core::Threshold {
                    row.thresholdLabel->text().toStdString(),
                    row.criticalSpin->value()
                }
            });
        }
        emit applyRequested(box_intervalSpin->value(), criticalValues);
    }

    void UIWindow::showAgentSettings(int updateIntervalMs, bool enabled) {
        const QSignalBlocker b1(box_intervalSpin);
        const QSignalBlocker b2(box_enabledCheck);

        box_intervalSpin->setValue(updateIntervalMs);
        box_enabledCheck->setChecked(enabled);
    }

    void UIWindow::showMetrics(const QVector<agent::Metric>& metrics) {
        if (metrics.size() != vector_metricRows.size())
            rebuildMetricRows(metrics.size());

        for (int i = 0; i < metrics.size(); ++i) {
            auto& metric = metrics.at(i);
            const MetricRow& row = vector_metricRows[i];

            row.nameLabel->setText(QString::fromStdString(metric.name));
            row.valueLabel->setText(QString::number(metric.value, 'f', 1));

            // Предполагаем диапазон 0-100 (%) для прогресс-бара. Если у метрики
            // другой диапазон - поправьте масштабирование здесь.
            row.bar->setValue(qBound(0, static_cast<int>(metric.value), 100));

            if (!row.criticalSpin->hasFocus()) {
                const QSignalBlocker blocker(row.criticalSpin);
                row.criticalSpin->setValue(metric.value);
            }

            applyThresholdColor(row.bar, metric.value, metric.criticalValue);
        }
    }

    void UIWindow::rebuildMetricRows(int metricCount) {
        for (MetricRow& row : vector_metricRows) {
            layout_metricsLayout->removeWidget(row.frame);
            row.frame->deleteLater();
        }
        vector_metricRows.clear();

        for (int i = 0; i < metricCount; ++i) {
            auto* frame = new QFrame(box_metricsPanelGroup);
            frame->setFrameShape(QFrame::Box);
            frame->setFrameShadow(QFrame::Plain);

            auto* rowLayout = new QHBoxLayout(frame);
            rowLayout->setContentsMargins(8, 4, 8, 4);

            auto* nameLabel = new QLabel(frame);
            nameLabel->setMinimumWidth(90);

            auto* bar = new QProgressBar(frame);
            bar->setRange(0, 100);
            bar->setTextVisible(false);

            auto* valueLabel = new QLabel(frame);
            valueLabel->setMinimumWidth(60);
            valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

            auto* criticalLabel = new QLabel(QStringLiteral("крит.:"), frame);

            auto* criticalSpin = new QDoubleSpinBox(frame);
            criticalSpin->setRange(0.0, 100000.0);
            criticalSpin->setMaximumWidth(90);

            rowLayout->addWidget(nameLabel);
            rowLayout->addWidget(bar, 1);
            rowLayout->addWidget(valueLabel);
            rowLayout->addWidget(criticalLabel);
            rowLayout->addWidget(criticalSpin);

            layout_metricsLayout->insertWidget(layout_metricsLayout->count() - 1, frame);

            vector_metricRows.append({ frame, nameLabel, bar, valueLabel, criticalSpin });
        }
    }

    void UIWindow::applyThresholdColor(QProgressBar* bar, double value, double criticalValue) const {
        if (criticalValue <= 0.0) {
            bar->setStyleSheet(QString()); // критическое значение не задано
            return;
        }

        const double ratio = value / criticalValue;
        if (ratio >= 1.0) {
            bar->setStyleSheet("QProgressBar::chunk { background-color: #d9534f; }"); // красный
        } else if (ratio >= 0.75) {
            bar->setStyleSheet("QProgressBar::chunk { background-color: #f0ad4e; }"); // жёлтый
        } else {
            bar->setStyleSheet("QProgressBar::chunk { background-color: #5cb85c; }"); // зелёный
        }
    }
}
