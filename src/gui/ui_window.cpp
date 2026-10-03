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
#include <QComboBox>
#include <QLineEdit>

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

        const QSignalBlocker blocker(combo_agentSelector);
        combo_agentSelector->clear();
        combo_agentSelector->addItems(names);
    }

    void UIWindow::setActiveAgentButton(int index) {
        if (auto* button = group_agentButtons->button(index)) {
            button->setChecked(true);
        }

        const QSignalBlocker blocker(combo_agentSelector);
        combo_agentSelector->setCurrentIndex(index);
    }

    void UIWindow::buildMetricsPanel() {
        box_metricsPanelGroup = new QGroupBox(QStringLiteral("Вывод метрик на экран"), this);

        layout_metricsLayout = new QVBoxLayout(box_metricsPanelGroup);
        layout_metricsLayout->addStretch(1);
    }

    void UIWindow::buildSettingsPanel() {
        box_settingsGroup = new QGroupBox(QStringLiteral("Настройка агента"), this);
        layout_outerLayout = new QVBoxLayout(box_settingsGroup);

        // Выпадающий список выбора текущего агента
        layout_outerLayout->addWidget(new QLabel(QStringLiteral("Выбранный агент:")));
        combo_agentSelector = new QComboBox(box_settingsGroup);
        connect(combo_agentSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &UIWindow::onAgentSelectorChanged);
        layout_outerLayout->addWidget(combo_agentSelector);

        // Чекбокс активности агента (сразу под выбором агента)
        box_enabledCheck = new QCheckBox(QStringLiteral("Агент активен"), box_settingsGroup);
        layout_outerLayout->addWidget(box_enabledCheck);

        // Переключаем видимость контейнера с настройками при изменении состояния чекбокса
        connect(box_enabledCheck, &QCheckBox::toggled, widget_settingsDetails, &QWidget::setVisible);
        // Отправляем сигнал наружу в MainWindow
        connect(box_enabledCheck, &QCheckBox::toggled, this, &UIWindow::agentEnabledChanged);

        // -------------------------------------------------------------------
        //   Контейнер для всех остальных настроек, находящихся ниже чекбокса
        // -------------------------------------------------------------------
        widget_settingsDetails = new QWidget(box_settingsGroup);
        auto* detailsLayout = new QVBoxLayout(widget_settingsDetails);
        detailsLayout->setContentsMargins(0, 0, 0, 0); // Убираем отступы контейнера

        // Выпадающий список типов агентов
        detailsLayout->addWidget(new QLabel(QStringLiteral("Тип агента:")));
        combo_agentType = new QComboBox(box_settingsGroup);
        combo_agentType->addItems({
            QStringLiteral("Агент CPU"),
            QStringLiteral("Агент памяти"),
            QStringLiteral("Агент сети")
        });
        detailsLayout->addWidget(combo_agentType);

        // Поле для изменения имени агента
        detailsLayout->addWidget(new QLabel(QStringLiteral("Имя агента:")));
        edit_agentName = new QLineEdit(widget_settingsDetails);
        detailsLayout->addWidget(edit_agentName);

        // Таймаут обновления
        detailsLayout->addWidget(new QLabel(QStringLiteral("Таймаут обновления (мс):")));
        box_intervalSpin = new QSpinBox(widget_settingsDetails);
        box_intervalSpin->setRange(100, 60000);
        box_intervalSpin->setSingleStep(100);
        detailsLayout->addWidget(box_intervalSpin);

        // Динамические метрики
        detailsLayout->addWidget(new QLabel(QStringLiteral("Отслеживаемые метрики:")));
        layout_metricsSettings = new QVBoxLayout();
        detailsLayout->addLayout(layout_metricsSettings);

        button_addMetric = new QPushButton(QStringLiteral("+ Добавить метрику"), widget_settingsDetails);
        connect(button_addMetric, &QPushButton::clicked, this, [this](){ addMetricSettingRow(); });
        detailsLayout->addWidget(button_addMetric);

        detailsLayout->addStretch(1);

        // Кнопка применения
        button_applyButton = new QPushButton(QStringLiteral("Применить"), widget_settingsDetails);
        connect(button_applyButton, &QPushButton::clicked, this, &UIWindow::onApplyButtonClicked);
        detailsLayout->addWidget(button_applyButton);

        // Добавляем контейнер в основной слой панели настроек
        layout_outerLayout->addWidget(widget_settingsDetails);
        layout_outerLayout->addStretch(1);
    }

    void UIWindow::addMetricSettingRow(const QString& name, const QString& condition, double value) {
        auto* widget = new QWidget(box_settingsGroup);
        auto* rowLayout = new QHBoxLayout(widget);
        rowLayout->setContentsMargins(0, 0, 0, 0);

        auto* nameEdit = new QLineEdit(widget);
        nameEdit->setPlaceholderText(QStringLiteral("Название метрики"));
        nameEdit->setText(name);

        auto* conditionCombo = new QComboBox(widget);
        conditionCombo->addItems({">", "<", "==", "<=", ">=", "!="});
        conditionCombo->setCurrentText(condition);

        auto* valueSpin = new QDoubleSpinBox(widget);
        valueSpin->setRange(-1000000.0, 1000000.0);
        valueSpin->setValue(value);

        auto* removeBtn = new QPushButton(QStringLiteral("-"), widget);
        removeBtn->setFixedWidth(30);

        rowLayout->addWidget(nameEdit);
        rowLayout->addWidget(conditionCombo);
        rowLayout->addWidget(valueSpin);
        rowLayout->addWidget(removeBtn);

        layout_metricsSettings->addWidget(widget);

        MetricSettingRow row = { widget, nameEdit, conditionCombo, valueSpin };
        vector_dynamicMetrics.append(row);

        // Логика удаления строки
        connect(removeBtn, &QPushButton::clicked, this, [this, widget]() {
            for (int i = 0; i < vector_dynamicMetrics.size(); ++i) {
                if (vector_dynamicMetrics[i].containerWidget == widget) {
                    vector_dynamicMetrics.removeAt(i);
                    break;
                }
            }
            widget->deleteLater();
        });
    }

    void UIWindow::onApplyButtonClicked() {
        QVector<core::MetricConfig> metricsData;
        metricsData.reserve(vector_dynamicMetrics.size());

        for (const auto& row : vector_dynamicMetrics) {
            metricsData.append({
                row.nameEdit->text().toStdString(),
                core::Threshold(
                    row.conditionCombo->currentText().toStdString(),
                    row.valueSpin->value())
            });
        }

        emit applyRequested(
            edit_agentName->text(),
            combo_agentType->currentIndex(),
            metricsData,
            box_intervalSpin->value()
        );
    }

    void UIWindow::onAgentSelectorChanged(int index) {
        // Блокируем сигналы, чтобы избежать зацикливания при программном изменении
        if (index >= 0) {
            emit agentSelected(index);
        }
    }

    void UIWindow::showAgentSettings(int updateIntervalMs, bool enabled) {
        // Блокируем сигналы, чтобы при установке значения из кода не отправлялся повторный сигнал
        const QSignalBlocker b1(box_intervalSpin);
        const QSignalBlocker b2(box_enabledCheck);

        box_intervalSpin->setValue(updateIntervalMs);
        box_enabledCheck->setChecked(enabled);

        // Синхронизируем видимость панели с текущим состоянием
        widget_settingsDetails->setVisible(enabled);
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
