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
        std::stringstream stream_line;

        // Создание лога в формате:
        // [<TIMESTAMP>] | <Metric1> : <Value1> |  <Metric2> : <Value2> |  <Metric3> : <Value3> | ...

        auto time = std::chrono::system_clock::now();
        std::time_t now_date = std::chrono::system_clock::to_time_t(time);

        // Сначала добавляем временную метку
        stream_line << std::put_time(std::localtime(&now_date), "%y.%m.%d %H:%M:%S");
        stream_line << " |";

        for (const auto&[name, value] : metrics) {
            // Добавляем имя метрики
            stream_line << " " << name << " : ";

            // Затем ее значение
            if (std::ranges::find(list_double_values.begin(), list_double_values.end(), name) != list_double_values.end()) {
                stream_line << value;
            }
            else {
                stream_line << static_cast<int>(value);
            }

            stream_line << " |";
        }

        stream_line << std::endl;
        _file << stream_line.str();
    }

}