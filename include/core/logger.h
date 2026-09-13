#ifndef SYSTEM_MONITORING_LOGGER_H
#define SYSTEM_MONITORING_LOGGER_H

/**
 * @file logger.h
 * @brief В этом файле описан класс Logger
 * @date 13.09.2026
 * @author Georgiy Kovalev
 */

#include "agent/agent.h"
#include <fstream>

namespace core {
    /**
     * @class Logger
     * @brief Осуществляет запись актуальных метрик в виде списка в журнал системы мониторинга в папке ./logs
     */
    class Logger {
    public:
        /**
         * @brief Открывает файл в директории ./logs с именем из даты текущего дня.
         * Если такого файла нет, то создает его
         * @throw std::runtime_error Если не удалось открыть файл для журнала
         */
        Logger();

        /**
         * @brief Закрывает файл, куда шла запись логов
         */
        ~Logger() noexcept;

        /**
         * @brief Записывает лог в журнал
         * @param metrics Список актуальных метрик
         */
        void log(const std::vector<agent::Metric>& metrics) noexcept;

    private:
        std::ofstream _file; ///< Файловый поток, ассоциированный с журналом
    };
}

#endif