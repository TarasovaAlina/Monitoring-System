#ifndef SYSTEM_MONITORING_UI_WINDOW_H
#define SYSTEM_MONITORING_UI_WINDOW_H

#include "core/config_service.h"
#include "core/kernel.h"
#include "agent/agent.h"
#include <QString>
#include <QVector>
#include <QFormLayout>
#include <QPushButton>

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

namespace gui {
    /**
     * @class UIWindow
    * @brief Отвечает исключительно за визуальное представление: панель вывода метрик и панель настроек.
    * Не хранит бизнес-логику и не знает про Kernel - только строит виджеты,
    * показывает то, что ей передали, и сообщает наружу о действиях пользователя через сигналы.
     */
    class UIWindow final : public QWidget {
        Q_OBJECT

    public:
        /**
         * @brief Строит каркас окна и соединяем слоты со сигналами
         * @param parent
         */
        explicit UIWindow(QWidget* parent = nullptr) noexcept;

        /**
         * @brief Обновляет выпадающий список с вариантами выбора агентов
         * @param names Список имен текущих агентов
         */
        void setAgentNames(const QStringList& names) noexcept;

        /**
         * @brief Устанавливает текущий активный вариант из выпадающего списка
         * @param index Индекс имени агента из списка
         */
        void setActiveAgentIndex(int index) noexcept;

        /**
         * @brief Отображает на панели справа настройки выбранного агента
         * @param name Имя этого агента
         * @param info Конфигурация этого агента
         */
        void showAgentSettings(const QString& name, const core::AgentInfo& info) noexcept;

        /**
         * @brief Выводит на левой панели до 20 последних собранных данных метрик.
         * Информация выводится построчно, в каждой строке находятся N пар {имя_метрики : значение},
         * которые были собраны отдельным агентом
         * @param metrics Таблица данных, готовая к выводу на экран
         */
        void showMetrics(const std::vector<std::vector<agent::Metric>>& metrics) noexcept;

    signals: // Действия пользователя, наружу
        /**
         * @brief @brief Оповещает класс MainWindow,
         * когда пользователь выбрал вариант из выпадающего списка на правой панели,
         * отвечающего за выбор отображения настроек конкретного агента.
         * @param index Индекс этого агента из списка имен агентов, хранящийся внутри MainWindow
         */
        void agentSelected(int index);

        /**
         * @brief Оповещает класс MainWindow, что был нажат чекбокс,
         * ответственный за перевод выбранного агента в состояние сна или выход из него
         * @param enabled Если чекбокс активирован (true), то агент работает,
         * иначе переходит в состояние сна и перестает обновлять метрики и передавать в систему
         */
        void agentEnabledChanged(bool enabled);

        /**
         * @brief Оповещает класс MainWindow, что была нажата кнопка "Применить",
         * вследствие чего измененные пользователем настройки агента будут сохранены в программе
         * @param agent_name Имя агента, чьи настройки были изменены
         * @param index Индекс варианта из выпадающего списка типов агента, который выбрал пользователь
         * @param metrics Список критических значений метрик
         * @param timeout_ms Таймаут обновления метрик агентом (в мс)
         */
        void applyRequested(const QString& agent_name, int index, const QList<core::MetricConfig>& metrics, long timeout_ms) const;

    private slots:
        /**
         * @brief Обработчик сигнала нажатия на кнопку "Применить" на панели справа.
         * Собирает все данные, которые ввел пользователь и посылает сигнал applyRequested
         */
        void onApplyButtonClicked() const noexcept;

        /**
         * @brief Обрабатывает переключение нового варианта из выпадающего списка агентов на панели справа
         * @param index Индекс выбранного варианта
         */
        void onAgentSelectorChanged(int index) noexcept;

    private:
        /**
         * @brief Обработчик нажатия на кнопку "+".
         * Добавляет еще одну строку интерфейса для изменения этой метрики агента
         */
        void addMetricSettingRow(const QString& name = "", const QString& condition = ">", double value = 0.0) noexcept;

        void buildMetricsPanel() noexcept; ///< Конструирует панель слева, которая служит для вывода собранных метрик
        void buildSettingsPanel() noexcept; ///< Конструирует панель справа, которая служит для отображения настроек агентов

        /**
         * @brief Перестраивает панель слева, чтобы все актуальные данные смогли вместиться на экране
         * @param metrics Массив актуальных метрик, хранящийся отдельными строками для 20 последних обновлений агентов
         */
        void rebuildMetricsGrid(const std::vector<std::vector<agent::Metric>>& metrics) noexcept;

        /**
         * @struct MetricSettingRow
         * @brief Структура для хранения указателей на элементы динамической строки
         */
        struct MetricSettingRow {
            QWidget* containerWidget; ///< Виджет строки метрики
            QLineEdit* nameEdit; ///< Поле для изменения имени
            QComboBox* conditionCombo; ///< Выпадающий список с возможными условиями достижения крит.значения (<, >, ==, !=, <=, >=)
            QDoubleSpinBox* valueSpin; ///< Спинбокс для изменения значения критического значения
        };

        QSplitter* splitter_main; ///< Разделитель между областями экрана

        QWidget* widget_metricsContainer; ///< Внутренний контейнер для сетки

        QVBoxLayout* layout_metricsContainer; ///< Слой внутри контейнера
        QHBoxLayout* layout_rootLayout; ///< Главный горизонтальный слой окна
        QVBoxLayout* layout_leftColumnLayout; ///< Вертикальный макет для левой панели
        QVBoxLayout* layout_metricsLayout; ///< Вертикальный слой внутри левой панели, который располагает выводимые данные сверху вниз
        QVBoxLayout* layout_outerLayout; ///< Вертикальный слой внутри правой панели, который располагает элементы внутри строго сверху вниз

        QGroupBox* box_metricsPanelGroup; ///< Визуальный блок с рамкой для выделения выводимых метрик в отдельную область
        QGroupBox* box_settingsGroup; ///< Визуальный блок с рамкой для выделения настроек агента в отдельную область

        // Элементы управления для панели настроек
        QComboBox* combo_agentSelector; ///< Выпадающий список выбора агента
        QCheckBox* box_enabledCheck; ///< Чекбокс активации работы агента
        QWidget* widget_settingsDetails; ///< Контейнер для настроек ниже чекбокса
        QComboBox* combo_agentType; ///< Выпадающий список выбора типа агента
        QLineEdit* edit_agentName; ///< Текстовое поле для изменения имени агента
        QSpinBox* box_intervalSpin; ///< Спинбокс для ввода таймаута обновления метрик агентом
        QVBoxLayout* layout_metricsSettings; ///< Слой отображения динамически изменяемых настроек метрик
        QPushButton* button_addMetric; ///< Кнопка добавления настройки метрики
        QPushButton* button_applyButton; ///< Кнопка применения всех настроек из панели настроек

        QVector<MetricSettingRow> vector_dynamicMetrics; ///< Хранилище строк метрик
        QVector<QVector<QLabel*>> grid_metrics; ///< Двумерный массив виджетов
    };
}

#endif