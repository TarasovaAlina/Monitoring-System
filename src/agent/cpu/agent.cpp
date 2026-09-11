#include "agent/cpu_monitoring_agent.h"
#include <fstream>
#include <sstream>
#include <bits/this_thread_sleep.h>
#include <filesystem>

namespace agent {
    CPUAgent::CPUAgent() noexcept : _cpu(0.0), _processes(0) {}

    double CPUAgent::cpu() const noexcept {
        return _cpu;
    }

    int CPUAgent::processes() const noexcept {
        return _processes;
    }

    void CPUAgent::updateMetrics() noexcept {
        auto stat1 = _readCPUStats();
        std::this_thread::sleep_for(std::chrono::microseconds(500));
        auto stat2 = _readCPUStats();

        // Обновляем статистику
        _cpu = 1.0 - static_cast<double>(stat2.first - stat1.first) / (stat2.second - stat1.second);
        _processes = _readCountProcesses();
    }

    std::pair<unsigned long long, unsigned long long> CPUAgent::_readCPUStats() noexcept {
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

    int CPUAgent::_readCountProcesses() noexcept {
        // Проходимся по содержимому директории /proc и ищем файлы с целочисленным названием
        int count = 0;

        for (auto const& dir_entry : std::filesystem::directory_iterator("/proc")) {
            // Если в строке будет символ, отличный от цифры, то вернет 0
            count += (std::atol(dir_entry.path().filename().string().c_str()) > 0);
        }

        return count;
    }

}
