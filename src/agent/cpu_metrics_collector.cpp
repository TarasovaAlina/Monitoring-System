#include "agent/metrics_collector.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <thread>

namespace agent {
    std::vector<Metric> CPUMetricsCollector::update(const std::vector<std::string>& metric_names_list) noexcept {
        std::vector<Metric> metrics{};
        metrics.reserve(metric_names_list.size());

        for (const auto& name : metric_names_list) {
            if (name == "cpu") {
                auto now_stat = _readCPUStats();

                auto total = static_cast<double>(now_stat.second - _cpu_stat.second);
                double cpu = (total) ? 1.0 - static_cast<double>(now_stat.first - _cpu_stat.first) / total : 0.0;
                metrics.emplace_back(Metric{name, cpu});

                _cpu_stat = now_stat;
            }

            else if (name == "processes") {
                metrics.emplace_back(Metric{name, static_cast<double>(_readCountProcesses())});
            }
        }

        return metrics;
    }

    int CPUMetricsCollector::_readCountProcesses() noexcept {
        // Проходимся по содержимому директории /proc и ищем файлы с целочисленным названием
        int count = 0;

        for (auto const& dir_entry : std::filesystem::directory_iterator("/proc")) {
            // Если в строке будет символ, отличный от цифры, то вернет 0
            count += (std::atol(dir_entry.path().filename().string().c_str()) > 0);
        }

        return count;
    }

    std::pair<unsigned long long, unsigned long long> CPUMetricsCollector::_readCPUStats() noexcept {
        std::ifstream file("/proc/stat");
        if (!file.is_open()) return {0, 0};

        std::string line;
        std::getline(file, line); // Считываем первую строку "cpu ..."
        std::stringstream ss(line);

        std::string label;
        ss >> label; // Считываем метку 'cpu'

        unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
        ss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

        unsigned long long idleTime = idle + iowait; // Время задержки
        unsigned long long totalTime = user + nice + system + idle + iowait + irq + softirq + steal;

        return {idleTime, totalTime};
    }
}