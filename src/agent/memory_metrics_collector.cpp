#include "agent/metrics_collector.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <thread>

namespace agent {
    std::vector<Metric> MemoryMetricsCollector::update(const std::vector<std::string>& metric_names_list) noexcept {
        std::pair<unsigned long, unsigned long> ram_metrics = _readRAMStat();

        // Сохраняем статистику использования ОЗУ
        auto ram_total = static_cast<double>(ram_metrics.second);
        auto ram = ram_total - ram_metrics.first;

        unsigned long long totalBytes = 0;
        int det_reads = 0, det_writes = 0;
        int det_sectors_read = 0, det_sectors_write = 0;

        for (const auto& dir_entry: std::filesystem::directory_iterator("/sys/block")) {
            if (dir_entry.path().filename().string().find("ram") == std::string::npos &&
                dir_entry.path().filename().string().find("loop") == std::string::npos) {

                std::string disk_name = dir_entry.path().filename().string(); // Название устройства (чтобы не было ram и loop)
                totalBytes += _getDiskSize(disk_name);

                std::pair<unsigned long, unsigned long> io_sectors = _readIOSectorsStat(disk_name);
                std::pair<unsigned long, unsigned long> io_operations = _readIOOperationsStat(disk_name);

                if (_io_sectors_stat.contains(disk_name)) {
                    det_sectors_read += io_sectors.first - _io_sectors_stat[disk_name].first;
                    det_sectors_write += io_sectors.second - _io_sectors_stat[disk_name].second;

                    det_reads += io_operations.first - _io_operations_stat[disk_name].first;
                    det_writes += io_operations.second - _io_operations_stat[disk_name].second;
                }

                _io_sectors_stat[disk_name] = io_sectors;
                _io_operations_stat[disk_name] = io_operations;
            }
        }

        auto cur_point = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration<double>(cur_point - _last_point).count();
        auto hard_volume = totalBytes;
        auto hard_ops = (det_reads + det_writes) / elapsed;
        auto hard_throughput = (det_sectors_read + det_sectors_write) * 512 / elapsed;

        _last_point = cur_point;

        std::vector<Metric> result_metrics;
        result_metrics.reserve(metric_names_list.size());
        // Добавляем только те метрики, которые указаны пользователем
        if (std::ranges::find(metric_names_list.begin(), metric_names_list.end(), "ram_total") != metric_names_list.end()) {
            result_metrics.emplace_back(Metric("ram_total", ram_total));
        }

        if (std::ranges::find(metric_names_list.begin(), metric_names_list.end(), "ram") != metric_names_list.end()) {
            result_metrics.emplace_back(Metric("ram", ram));
        }

        if (std::ranges::find(metric_names_list.begin(), metric_names_list.end(), "hard_volume") != metric_names_list.end()) {
            result_metrics.emplace_back(Metric("hard_volume", hard_volume));
        }

        if (std::ranges::find(metric_names_list.begin(), metric_names_list.end(), "hard_ops") != metric_names_list.end()) {
            result_metrics.emplace_back(Metric("hard_ops", hard_ops));
        }

        if (std::ranges::find(metric_names_list.begin(), metric_names_list.end(), "hard_throughput") != metric_names_list.end()) {
            result_metrics.emplace_back(Metric("hard_throughput", hard_throughput));
        }

        return result_metrics;
    }

    std::pair<unsigned long, unsigned long> MemoryMetricsCollector::_readIOSectorsStat(const std::string &disk_name) noexcept {
        unsigned long sectors_read, sectors_write;
        std::ifstream file("/sys/block/" + disk_name + "/stat");

        file >> sectors_read >> sectors_read >> sectors_read; // Считываем 3-е поле
        file >> sectors_write >> sectors_write >> sectors_write >> sectors_write; // Считываем 7-е поле

        return {sectors_read, sectors_write};
    }

    std::pair<unsigned long, unsigned long> MemoryMetricsCollector::_readIOOperationsStat(const std::string &disk_name) noexcept {
        unsigned long operations_read, operations_write;
        std::ifstream file("/sys/block/" + disk_name + "/stat");

        file >> operations_read; // Считываем 1-е поле
        file >> operations_write >> operations_write >> operations_write >> operations_write; // Считываем 5-е поле

        return {operations_read, operations_write};
    }

    std::pair<unsigned long, unsigned long> MemoryMetricsCollector::_readRAMStat() noexcept {
        std::ifstream file("/proc/meminfo");
        if (!file.is_open()) return {0, 0};

        unsigned long total, available;
        std::string line, label;

        std::getline(file, line); // Считываем первую строку "MemTotal: ..."
        std::stringstream ss(line);

        ss >> label; // Считываем метку 'MemTotal:'
        ss >> total;

        std::getline(file, line); // Считываем вторую строку "MemFree: ..."
        std::getline(file, line); // Считываем третью строку "MemAvailable: ..."

        ss = std::stringstream(line);
        ss >> label; // Считываем метку 'MemAvailable:'
        ss >> available;

        return {available, total};
    }

    unsigned long long MemoryMetricsCollector::_getDiskSize(const std::string &disk_name) noexcept {
        std::ifstream file("/sys/block/" + disk_name + "/size");

        uint64_t sectors = 0;

        if (!(file >> sectors)) {
            return 0;
        }

        return sectors * 512ULL;
    }

}