#include "agent/memory_monitoring_agent.h"
#include <fstream>
#include <sstream>
#include <thread>
#include <filesystem>
#include <sys/statvfs.h>

namespace agent {
    double MemoryAgent::ram_total() const noexcept {
        return _ram_total;
    }

    double MemoryAgent::ram() const noexcept {
        return _ram;
    }

    const std::map<std::string, DiskInfo> &MemoryAgent::diskIOStats() const noexcept {
        return _disk_io_stat;
    }

    void MemoryAgent::updateMetrics() noexcept {
        std::pair<unsigned long, unsigned long> ram_metrics = _readRAMStat();

        // Сохраняем статистику использования ОЗУ
        _ram_total = ram_metrics.second;
        _ram = 1.0 - static_cast<double>(ram_metrics.first) / _ram_total;

        for (const auto& dir_entry: std::filesystem::directory_iterator("/sys/block")) {
            if (dir_entry.path().filename().string().find("ram") == std::string::npos &&
                dir_entry.path().filename().string().find("loop") == std::string::npos) {

                DiskInfo disk_info{};
                std::string disk_name = dir_entry.path().filename().string(); // Название диска (чтобы не было ram и loop)

                struct statvfs stat;
                if (statvfs(("/dev/" + disk_name).c_str(), &stat) == 0) {
                    unsigned long long totalBytes = stat.f_blocks * stat.f_frsize;
                    unsigned long long freeBytes = stat.f_bavail * stat.f_frsize;

                    disk_info.hard_volume = totalBytes - freeBytes;
                }

                std::pair<unsigned long, unsigned long> disk_metrics1 = _readDiskStat(disk_name);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                std::pair<unsigned long, unsigned long> disk_metrics2 = _readDiskStat(disk_name);

                unsigned long det_read = disk_metrics2.first - disk_metrics1.first;
                unsigned long det_write = disk_metrics2.second - disk_metrics1.second;

                disk_info.hard_ops = (det_read + det_write) * 10;
                disk_info.hard_throughput = disk_info.hard_ops * 512;

                _disk_io_stat[disk_name] = disk_info;
            }
        }
    }

    std::pair<unsigned long, unsigned long> MemoryAgent::_readRAMStat() noexcept {
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

    std::pair<unsigned long, unsigned long> MemoryAgent::_readDiskStat(const std::string &disk_name) noexcept {
        unsigned long sectors_read, sectors_write;
        std::ifstream file("/sys/block/" + disk_name + "/stat");

        file >> sectors_read >> sectors_read >> sectors_read; // Считываем 3-е поле
        file >> sectors_write >> sectors_write >> sectors_write >> sectors_write; // Считываем 7-е поле

        return {sectors_read, sectors_write};
    }

}