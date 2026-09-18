#include "tools/logger.h"
#include <chrono>

namespace tools {
    Logger::Logger() noexcept {
        auto time = std::chrono::system_clock::now();
        std::time_t now_date = std::chrono::system_clock::to_time_t(time);

        std::stringstream ss;
        ss << std::put_time(std::localtime(&now_date), "%d.%m.%Y");

        std::string name = ss.str() + ".log";

        _file.open(name, std::ios::app);

        if (!_file.is_open()) {
            throw std::runtime_error("The file cannot be opened");
        }
    }

    Logger::~Logger() noexcept {
        if (_file.is_open()) {
            _file.close();
        }
    }

    void Logger::log(const std::vector<agent::Metric> &metrics) noexcept {
        static std::vector<std::string> list_double_values {
            "cpu", "ram_total", "ram", "hard_volume",
            "hard_throughput", "inet_throughput"
        };
        std::string line;

        for (const auto& metric : metrics) {
            line += metric.name + ": ";

            if (std::find(list_double_values.begin(), list_double_values.end(), metric.name) != list_double_values.end()) {
                line += metric.value;
            }
            else {
                line += static_cast<int>(metric.value);
            }

            line += " ";
        }

        _file << line;
    }

}