#include "agent/metrics_collector.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <thread>
#include <sys/statvfs.h>

namespace agent {
    std::vector<Metric> MemoryMetricsCollector::update() noexcept {
        std::pair<unsigned long, unsigned long> ram_metrics = _readRAMStat();

        // Сохраняем статистику использования ОЗУ
        auto ram_total = static_cast<double>(ram_metrics.second);
        auto ram = 1.0 - static_cast<double>(ram_metrics.first) / ram_total;

        unsigned long long totalBytes = 0, freeBytes = 0;
        int det_reads = 0, det_writes = 0;
        for (const auto& dir_entry: std::filesystem::directory_iterator("/sys/block")) {
            if (dir_entry.path().filename().string().find("ram") == std::string::npos &&
                dir_entry.path().filename().string().find("loop") == std::string::npos) {

                std::string disk_name = dir_entry.path().filename().string(); // Название устройства (чтобы не было ram и loop)

                struct statvfs stat;
                if (statvfs(("/dev/" + disk_name).c_str(), &stat) == 0) {
                    totalBytes += stat.f_blocks * stat.f_frsize;
                    freeBytes += stat.f_bavail * stat.f_frsize;
                }

                std::pair<unsigned long, unsigned long> disk_metrics1 = _readPartitionDiskStat(disk_name);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                std::pair<unsigned long, unsigned long> disk_metrics2 = _readPartitionDiskStat(disk_name);

                det_reads += disk_metrics1.first - disk_metrics1.first;
                det_writes += disk_metrics2.second - disk_metrics1.second;
            }
        }

        auto hard_volume = totalBytes - freeBytes;
        auto hard_ops = (det_reads + det_writes) * 10;
        auto hard_throughput = hard_ops * 512;

        return {
            Metric("ram_total", ram_total),
            Metric("ram", ram),
            Metric("hard_volume", hard_volume),
            Metric("hard_ops", hard_ops),
            Metric("hard_throughput", hard_throughput)
        };
    }

    std::pair<unsigned long, unsigned long> MemoryMetricsCollector::_readPartitionDiskStat(const std::string &disk_name) noexcept {
        unsigned long sectors_read, sectors_write;
        std::ifstream file("/sys/block/" + disk_name + "/stat");

        file >> sectors_read >> sectors_read >> sectors_read; // Считываем 3-е поле
        file >> sectors_write >> sectors_write >> sectors_write >> sectors_write; // Считываем 7-е поле

        return {sectors_read, sectors_write};
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
}