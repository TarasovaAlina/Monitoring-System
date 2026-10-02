#ifndef SYSTEM_MONITORING_MAIN_WINDOW_H
#define SYSTEM_MONITORING_MAIN_WINDOW_H

#include "gui/ui_window.h"
#include "core/kernel.h"
#include <QMainWindow>
#include <QStringList>
#include <QVector>
#include <memory>

namespace gui {
    /**
     * @class MainWindow
     * @brief Управление приложением: владеет ядром (core::Kernel), таймером опроса метрик,
     * хранит индекс текущего выбранного агента и переводит данные между классами Kernel и UIWindow.
     * Сами виджеты и разметка живут в UIWindow - здесь их нет.
     * Данные метрик берутся из текущего файла логов и отображаются на экране
     */
    class MainWindow : public QMainWindow {
        Q_OBJECT

    public:
        /**
         * @brief Создание окна приложения:
         * 1. Создание ядра приложения
         * 2. Создание UI
         * 3. Соединение сигналов от UI со слотами этого класса
         */
        explicit MainWindow(QWidget* parent = nullptr);
        ~MainWindow() override = default;

    private slots:
        /**
         * @brief Реакция на переключение пользователем вкладки с агентами
         * @param index Индекс агента, который выбрал пользователь
         */
        void onAgentSelected(int index);

        /**
         * @brief Реакция на включение/выключение тумблера активности агента,
         * чья информация показана на вкладке
         * @param enabled true - агент должен быть активен, false - переключиться в режим сна
         */
        void onAgentEnabledChanged(bool enabled);

        /**
         * @brief Реакция на изменение имени текущего агента
         * @param new_name Строка, которую ввел пользователь в поле ввода
         */
        void onApplyRequested(const QString& new_name, int index, const QList<core::MetricConfig>& critical_metrics_list, long timeout_ms) noexcept;

    private:
        /**
         * @brief Обновляет вкладку с агентами, запрашивая у _kernel актуальный список агентов
         */
        void reloadAgentList();

        /**
         * @brief Отображает на во вкладке агента его текущие настроки
         */
        void pushCurrentAgentToUI();

        /**
         * @return Имя агента, чья вкладка с настройками активна в данный момент
         */
        QString currentAgentName() const;

        std::unique_ptr<core::Kernel> _kernel; ///< Ядро программы, которое управляет внутренней логикой
        UIWindow* _ui; ///< Интерфейс программы, который занимается отображением текущей информации

        QStringList _agentNames; ///< Список имен агентов, которые работают в ядре
        int _currentAgentIndex = 0; ///< Индекс агента в списке, который открыт во вкладке настроек агентов

        QList<QList<agent::Metric>> _metrics_list; ///< Список метрик, которые должны быть показаны в UI
    };
}

#endif