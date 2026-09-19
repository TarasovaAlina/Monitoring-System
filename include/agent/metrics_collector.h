#ifndef SYSTEM_MONITORING_METRICS_COLLECTOR_H
#define SYSTEM_MONITORING_METRICS_COLLECTOR_H

/**
 * @file metrics_collector.h
 * @brief В этом файле описан базовый класс IMetrcisCollector и его производные классы
 * @date 19.09.2026
 * @author Kovalev Georgiy
 */

#include <unordered_map>
#include <string>
#include <vector>
#include <map>

namespace agent {
    /**
     * @class IMetricsCollector
     * @brief Интерфейс, описывающий классы, отвечающие за непосредственный сбор метрик системы
     */
    class IMetricsCollector {
    public:
        virtual std::vector<Metric> update() noexcept = 0;
        virtual ~IMetricsCollector() = default;
        void setUsingMetrics(const std::vector<std::string>& metrics_list);

    protected:
        /**
         * @brief Хранит информацию о том, какие метрики из стандартных агент собирает
         */
        std::unordered_map<std::string, bool> _using_metrics;
    };

    /**
     * @class CPUMetricsCollector
     * @brief Сборщик метрик загрузки процессора
     */
    class CPUMetricsCollector : public IMetricsCollector {
    public:
        /**
         * @brief Подсчет загрузки процессора осуществляется через чтение файла @code /proc/stat@endcode.
         * В первой строке располагаются значения тиков времени для всего процессора (метка cpu).
         * Считывается продолжительность работы процессора в разных режимах и время простоя,
         * после чего рассчитывается загруженность по формуле:\n
         * @code cpu(%) = 1.0 - (idle2 - idle1) / (total2 - total1) @endcode
         *
         * Количество процессов в системе равно количеству файлов в @code /proc/<PID>@endcode,
         * где <PID> - целочисленное положительное значение
         */
        std::vector<Metric> update() noexcept override;

    private:
        /**
         * @brief Вычисляет значения, которые хранятся в @code/proc/stat@endcode:
         * - Время простоя процессора (idle)
         * - Общее пройденное время (total)
         */
        static std::pair<unsigned long long, unsigned long long> _readCPUStats() noexcept;

        /**
         * @brief Расчитывает количество файлов в директории @code/proc@endcode,
         * название которых состоит из целочисленного значения
         * @return Количество процессов в системе
         */
        static int _readCountProcesses() noexcept;
    };

    /**
     * @class MemoryMetricsCollector
     * @brief Сборщик метрик загрузки RAM и HDD
     */
    class MemoryMetricsCollector : public IMetricsCollector {
    public:
        /**
         * @brief Обновляет метрики использования памяти системы.\n
         * Основная статистика использования памяти системы хранится в файле @code/proc/meminfo@endcode.
         * - Метка MemTotal хранит общий объем ОЗУ в Кб.
         * - Метка MemAvailable хранит объем доступной ОЗУ в Кб.
         *
         * Таким образом, ram_total равен MemTotal, а значение ram рассчитывается как
         * @code 1 - (MemTotal - MemAvailable) @endcode
         *
         * Для получения объема диска используется библиотека statvfs
         *
         * Для получения статистики I/O использования жесткого диска считываются данные из файла @code/sys/block/<dev>/stat@endcode:
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
        std::vector<Metric> update() noexcept override;

    private:
        /**
         * @brief Парсит из файла @code/proc/meminfo@endcode значения меток MemTotal и MemAvailable
         * @return пара значений {MemAvailable, MemTotal}
         */
        static std::pair<unsigned long, unsigned long> _readRAMStat() noexcept;

        /**
         * @brief Парсит из файла @code/sys/block/<dev>/stat@endcode
         * @return Пара значений {sectors_read, sectors_write}
         */
        static std::pair<unsigned long, unsigned long> _readPartitionDiskStat(const std::string& disk_name) noexcept;
    };

    /**
     * @class NetworkMetricsCollector
     * @brief Сборщик метрик загрузки сети
     */
    class NetworkMetricsCollector : public IMetricsCollector {
    public:
        /**
         * @brief Обновляет метрики использования сетевых интерфейсов системы.\n
         *
         * Для получения доступности url используется библиотека libcurl.
         * С ее помощью отправляется HTTP-запрос типа HEAD до нужного сайта и ожидается код ответа 2XX или 3XX
         *
         * Для расчета пропускной способности парсится файл @code/proc/net/dev@endcode:
         * - Поле 1: название интерфейса
         * - Поле 2: кол-во принятых байтов
         * - Поле 10: кол-во отправленных байтов
         *
         * Для этого через определенный промежуток времени считываются значения байтов.
         * Пропускная способность рассчитывается по формуле:\n
         * @code throughput = ((receive_bytes2 - receive_bytes1) + (transmit_bytes2 - transmit_bytes1)) / det_time
         */
        std::vector<Metric> update() noexcept override;

        /**
         * @brief Добавляет новый url для отслеживания его доступности
         * @param url Адрес сайта, который указан в конфиге
         */
        void addURL(const std::string& url) noexcept;

    private:
        /**
         * @brief Считывает 2-е и 10-е поля из файла @code/proc/net/dev@endcode для каждого сетевого интерфейса
         * @return Пара <считанные_байты; отправленные_байты> для каждого сетевого интерфейса, доступного в системе
         */
        static std::map<std::string, std::pair<unsigned long, unsigned long>> _readNetInterfaces() noexcept;
    };
}

#endif