#ifndef SYSTEM_MONITORING_MEMORY_MONITORING_AGENT_H
#define SYSTEM_MONITORING_MEMORY_MONITORING_AGENT_H

#include "agent.h"
#include <map>
#include <string>

/**
 * @file memory_monitoring_agent.h
 * @brief В этом файле описан класс агента MemoryAgent
 * @author Georgiy Kovalev
 * @date 04.09.2026
 */

namespace agent {
    /**
     * @struct DiskInfo
     * @brief Хранит в себе информацию об использовании отдельного диске
     */
    struct DiskInfo {
        double hard_volume; ///< Использование объема жесткого диска
        int hard_ops; ///< Количество операций I/O для жесткого диска в секунду
        double hard_throughput; ///< Пропускная способность HDD
    };

    /**
     * @class MemoryAgent
     * @brief Собирает информацию об использовании памяти системы (ОЗУ и HDD)
     */
    class MemoryAgent final : public IAgent {
    public:
        MemoryAgent() noexcept = default;

        /**
         * @brief Обновляет метрики использования памяти системы.\n
         * Основная статистика использования памяти системы хранится в файле /proc/meminfo.
         * - Метка MemTotal хранит общий объем ОЗУ в Кб.
         * - Метка MemAvailable хранит объем доступной ОЗУ в Кб.
         *
         * Таким образом, ram_total равен MemTotal, а значение ram рассчитывается как
         * @code 1 - (MemTotal - MemAvailable) @endcode
         *
         * Для получения объема диска используется библиотека statvfs
         *
         * Для получения статистики I/O использования жесткого диска считываются данные из файла /sys/block/<dev>/stat:
         * - Поле 3: количество прочитанных секторов
         * - Поле 7: количество записанных секторов
         *
         * @note Размер одного сектора 512 байт, независимо от диска
         *
         * Эти данные считываются дважды через определенный промежуток времени, после чего вычисляется IOPS:
         * @code hard_ops = (det_read + det_write) / det_time @endcode
         *
         * В свою очередь, пропускная способность связана с IOPS через размер блока:
         * @code hard_throughput = hard_ops * block_size @endcode
         *
         */
        void updateMetrics() noexcept override;

        /**
         * @return Значение приватного поля _ram_total
         */
        double ram_total() const noexcept;

        /**
         * @return Значение приватного поля _ram
         */
        double ram() const noexcept;

        /**
         * @brief Осуществляет доступ к приватному полю _disk_io_stat
         * @return Метрики использования каждого диска
         */
        const std::map<std::string, DiskInfo>& diskIOStats() const noexcept;

    private:
        /**
         * @brief Парсит из файла /proc/meminfo значения меток MemTotal и MemAvailable
         * @return пара значений {MemAvailable, MemTotal}
         */
        static std::pair<unsigned long, unsigned long> _readRAMStat() noexcept;

        /**
         * @brief Парсит из файла /sys/block/<dev>/stat
         * @return Пара значений {sectors_read, sectors_write}
         */
        static std::pair<unsigned long, unsigned long> _readDiskStat(const std::string& disk_name) noexcept;

        double _ram_total; ///< Общий объем ОЗУ
        double _ram; ///< Загрузка ОЗУ (в %)
        std::map<std::string, DiskInfo> _disk_io_stat; ///< Системные метрики каждого отдельного диска
    };
}

#endif