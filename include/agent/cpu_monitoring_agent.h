#ifndef SYSTEM_MONITORING_CPU_MONITORING_AGENT_H
#define SYSTEM_MONITORING_CPU_MONITORING_AGENT_H

#include "agent.h"
#include <utility>

/**
 * @file cpu_monitoring_agent.h
 * @brief В этом файле описан класс агента CPUAgent
 * @author Georgiy Kovalev
 * @date 04.09.2026
 */

namespace agent {
    /**
     * @class CPUAgent
     * @brief Собирает информацию о загруженности CPU.
     * - Загрузка процессора считывается из фалйа
     */
    class CPUAgent final : public IAgent {
    public:
        CPUAgent() noexcept;

        /**
         * - Подсчет загрузки процессора осуществляется через чтение файла `/proc/stat`.
         * В первой строке располагаются значения тиков времени для всего процессора (метка cpu).
         * Считывается продолжительность работы процессора в разных режимах и время простоя,
         * после чего рассчитывается загруженность по формуле:\n
         * cpu(%) = 1.0 - (idle2 - idle1) / (total2 - total1)
         *
         * - Количество процессов в системе равно количеству файлов в /proc/<PID>,
         * где <PID> - целочисленное положительное значение
         */
        void updateMetrics() noexcept override;

        /**
         * @return Значения метрик cpu и processes
         */
        std::vector<Metric> getMetrics() noexcept override;

    private:
        /**
         * @brief Вычисляет значения, которые хранятся в /proc/stat:
         * - Время простоя процессора (idle)
         * - Общее пройденное время (total)
         */
        static std::pair<unsigned long long, unsigned long long> _readCPUStats() noexcept;

        /**
         * @brief Расчитывает количество файлов в директории /proc,
         * название которых состоит из целочисленного значения
         * @return Количество процессов в системе
         */
        static int _readCountProcesses() noexcept;

        double _cpu; ///< Загрузка процессора (от 0.0 до 1.0)
        int _processes; ///< Количество процессов в системе
    };
}

#endif