#include "gui/ui_window.h"
#include "core/config_service.h"
#include "core/kernel.h"

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

        buildMetricsPanel();
        layout_leftColumnLayout->addWidget(box_metricsPanelGroup, 1);

        layout_rootLayout->addLayout(layout_leftColumnLayout, 4);

        // Правая колонка
        buildSettingsPanel();
        layout_rootLayout->addWidget(box_settingsGroup, 1);
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

    void UIWindow::setActiveAgentIndex(int index) {
        if (auto* button = group_agentButtons->button(index)) {
            button->setChecked(true);
        }

        const QSignalBlocker blocker(combo_agentSelector);
        combo_agentSelector->setCurrentIndex(index);
    }

    void UIWindow::buildMetricsPanel() {
        box_metricsPanelGroup = new QGroupBox(QStringLiteral("Вывод метрик на экран"), this);

        layout_metricsLayout = new QVBoxLayout(box_metricsPanelGroup);
        layout_metricsLayout->setAlignment(Qt::AlignTop);
        layout_metricsLayout->setContentsMargins(5, 15, 5, 5);

        // Инициализируем пустой контейнер
        widget_metricsContainer = new QWidget(box_metricsPanelGroup);
        layout_metricsContainer = new QVBoxLayout(widget_metricsContainer);
        layout_metricsContainer->setContentsMargins(0, 0, 0, 0);

        layout_metricsLayout->addWidget(widget_metricsContainer);
        layout_metricsLayout->addStretch(1); // Пружина, прижимающая контейнер к верху
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

    void UIWindow::rebuildMetricsGrid(const QList<QList<agent::Metric>>& metrics) {
        // Удаляем старый контейнер со всеми внутренними Layout и виджетами
        if (widget_metricsContainer) {
            widget_metricsContainer->deleteLater();
        }
        grid_metrics.clear();

        // Создаем новый контейнер
        widget_metricsContainer = new QWidget(box_metricsPanelGroup);
        layout_metricsContainer = new QVBoxLayout(widget_metricsContainer);
        layout_metricsContainer->setContentsMargins(0, 0, 0, 0);
        layout_metricsContainer->setSpacing(8);

        grid_metrics.resize(metrics.size());

        for (int i = 0; i < metrics.size(); ++i) {
            auto* rowLayout = new QHBoxLayout();
            rowLayout->setContentsMargins(0, 0, 0, 0);
            rowLayout->setSpacing(5);

            grid_metrics[i].resize(metrics[i].size());

            for (int j = 0; j < metrics[i].size(); ++j) {
                // Визуальная ячейка
                auto* cellFrame = new QFrame(widget_metricsContainer);
                cellFrame->setFrameShape(QFrame::StyledPanel);
                // Стилизуем под плашку с рамкой
                cellFrame->setStyleSheet("QFrame { background-color: #f5f5f5; border: 1px solid #c0c0c0; border-radius: 4px; padding: 2px 6px; }");

                auto* cellLayout = new QHBoxLayout(cellFrame);
                cellLayout->setContentsMargins(4, 4, 4, 4);

                auto* label = new QLabel(QStringLiteral("-"), cellFrame);
                label->setStyleSheet("border: none; background: transparent;"); // Убираем рамку внутри самой метки

                cellLayout->addWidget(label);
                rowLayout->addWidget(cellFrame);

                grid_metrics[i][j] = { label };
            }

            rowLayout->addStretch(1); // Прижимаем все ячейки текущей строки к левому краю
            layout_metricsContainer->addLayout(rowLayout);
        }

        // Вставляем новый контейнер в самое начало основного слоя, перед пружиной addStretch(1)
        layout_metricsLayout->insertWidget(0, widget_metricsContainer);
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

    void UIWindow::showAgentSettings(const QString& name, const core::AgentInfo& info) {
        // Блокируем сигналы, чтобы при установке значения из кода не отправлялся повторный сигнал
        const QSignalBlocker b1(box_enabledCheck);
        const QSignalBlocker b2(combo_agentType);
        const QSignalBlocker b3(edit_agentName);
        const QSignalBlocker b4(box_intervalSpin);

        box_enabledCheck->setChecked(info.is_active);
        widget_settingsDetails->setVisible(info.is_active);

        combo_agentType->setCurrentIndex(static_cast<int>(info.type));
        edit_agentName->setText(name);
        box_intervalSpin->setValue(info.refresh_time_ms);

        // Синхронизируем видимость панели с текущим состоянием
        widget_settingsDetails->setVisible(info.is_active);

        // Собираем множество имён метрик, пришедших из info
        QSet<QString> infoMetricNames;
        for (const auto& metric : info.critical_metrics) {
            infoMetricNames.insert(QString::fromStdString(metric.target));
        }

        // Удаляем из UI те метрики, которых НЕТ в info
        // Итерируемся с конца массива, чтобы корректно удалять элементы по индексу
        for (int i = vector_dynamicMetrics.size() - 1; i >= 0; --i) {
            const QString uiMetricName = vector_dynamicMetrics[i].nameEdit->text();

            if (!infoMetricNames.contains(uiMetricName)) {
                // Удаляем QWidget из интерфейса и очищаем запись из вектора
                auto* widget = vector_dynamicMetrics[i].containerWidget;
                vector_dynamicMetrics.removeAt(i);
                widget->deleteLater();
            }
        }

        // Обновляем существующие в UI метрики и добавляем отсутствующие
        for (const auto& metric : info.critical_metrics) {
            const QString metricName = QString::fromStdString(metric.target);

            // ПРИМЕЧАНИЕ: скорректируйте обращения к полям ниже (condition / critical_value),
            // если в вашей структуре core::MetricConfig они называются иначе
            // (например, metric.threshold.condition или metric.criticalValue)
            const QString condition = QString::fromStdString(metric.threshold.operation);
            const double criticalValue = metric.threshold.value;

            // Ищем, есть ли уже строка с таким именем метрики в UI
            auto it = std::find_if(vector_dynamicMetrics.begin(), vector_dynamicMetrics.end(),
                                   [&metricName](const MetricSettingRow& row) {
                                       return row.nameEdit->text() == metricName;
                                   });

            if (it != vector_dynamicMetrics.end()) {
                // Метрика ЕСТЬ в UI -> обновляем её значения
                const QSignalBlocker bCond(it->conditionCombo);
                const QSignalBlocker bVal(it->valueSpin);

                it->conditionCombo->setCurrentText(condition);
                it->valueSpin->setValue(criticalValue);
            } else {
                // Метрики НЕТ в UI -> добавляем новую строчку
                addMetricSettingRow(metricName, condition, criticalValue);
            }
        }
    }

    void UIWindow::showMetrics(const QList<QList<agent::Metric>>& metrics) {
        // 1. Проверяем, совпадает ли размерность интерфейса с пришедшими данными
        bool needRebuild = false;
        if (metrics.size() != grid_metrics.size()) {
            needRebuild = true;
        } else {
            for (int i = 0; i < metrics.size(); ++i) {
                if (metrics[i].size() != grid_metrics[i].size()) {
                    needRebuild = true;
                    break;
                }
            }
        }

        // 2. Если количество строк или ячеек изменилось — перестраиваем сетку
        if (needRebuild) {
            rebuildMetricsGrid(metrics);
        }

        // 3. Обновляем текст в ячейках
        for (int i = 0; i < metrics.size(); ++i) {
            for (int j = 0; j < metrics[i].size(); ++j) {
                const auto& metric = metrics[i][j];

                // Форматируем строку вида "metric : value"
                QString text = QStringLiteral("%1 : %2")
                               .arg(QString::fromStdString(metric.name))
                               .arg(metric.value, 0, 'f', 2);

                grid_metrics[i][j]->setText(text);
            }
        }
    }
}
