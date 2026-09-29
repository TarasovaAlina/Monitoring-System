#ifndef SYSTEM_MONITORING_LOG_READER_H
#define SYSTEM_MONITORING_LOG_READER_H
#include "agent/metrics_collector.h"

/**
 * @file log_reader.h
 * @brief В этом файле описан класс LogReader
 * @date 29.09.2026
 * @author Georgiy Kovalev
 */

/**
 * @namespace tools
 * @brief В этом пространстве имен описаны вспомогательные классы,
 * которые решают второстепенные задачи, не связанные напрямую с логикой программы
 */
namespace tools {
    /**
     * @class LogReader
     * @brief Вспомогательный класс, реализующий чтение метрик из файла логов.
     * Используется классом gui::MainWindow, чтобы получать данные для отображения в UI
     */
    class LogReader {
    public:
        /**
         * @brief Решает, какой файл нужно читать
         */
        LogReader() noexcept;

        /**
         * @brief Сравнивает последнее сохраненное значение размера файла с текущим.
         * Если они различаются, значит были записаны новые данные и их нужно передать в UI
         * @return true, если содержимое файла изменилось, иначе false
         */
        bool isUpdated() const noexcept;

        /**
         * @brief Считывает данные метрик и дату из последней строки в файле логов.\n
         * Формат данных в строке:\n
         * [<TIMESTAMP>] | <Metric1> : <Value1> |  <Metric2> : <Value2> |  <Metric3> : <Value3> | ...
         * - <TIMESTAMP> — временная метка в формате yy-MM-dd HH:mm:ss
         * - <MetricN> — N-ная метрика
         * - <ValueN> — значение N-ной метрики
         *
         * @return Список данных метрик вместе с временной меткой, указывающая, когда этот лог был записан.
         */
        std::pair<std::string, std::vector<agent::Metric>> readData() noexcept;

    private:
        /**
         * @brief Считывает последнюю строку в файле
         */
        std::string _getLastLine() noexcept;

        std::string _log_file; ///< Имя файла логов, откуда нужно читать данные
        long long _current_size_file; ///< Запоминаем текущий размер файла
    };

}

#endif