#include "tools/log_reader.h"
#include <fstream>
#include <chrono>
#include <sstream>

namespace tools {
    LogReader::LogReader() noexcept : _current_size_file(0) {
        const auto time = std::chrono::system_clock::now();
        const std::time_t now_date = std::chrono::system_clock::to_time_t(time);

        std::stringstream ss;
        ss << std::put_time(std::localtime(&now_date), "%d.%m.%Y");

        _log_file = ss.str() + ".log";
    }

    bool LogReader::isUpdated() const noexcept {
        std::ifstream file(_log_file);

        if (!file.is_open()) {
            // Пока не буду прописывать исключение
            return false;
        }

        // Перемещаем указатель в конец файла и находим его размер
        file.seekg(0, std::ios::end);
        auto size = file.tellg();

        return size != _current_size_file;
    }

    std::pair<std::string, std::vector<agent::Metric> > LogReader::readData() noexcept {
        auto last_line = _getLastLine();
        auto result = std::make_pair("", std::vector<agent::Metric>());

        if (!last_line.empty()) {
            std::stringstream ss(last_line);
            std::string temp;
            std::string date;

            // Сначала считываем <TIMESTAMP> в формате yy-MM-dd HH:mm:ss
            ss >> temp;
            date = temp;
            ss >> temp;
            date += " " + temp;
            result.first = date.c_str();

            ss >> temp; // Считываем первый ограничитель '|'
            while (std::getline(ss, temp, '|')) {
                std::stringstream metric_stream(temp);
                agent::Metric metric;

                // Считываем из подстроки "<Metric> : <Value>"
                // Получаем <name>
                metric_stream >> temp;
                metric.name = temp;

                // Считываем символ ':' и <value>
                double value;
                metric_stream >> temp >> temp;
                std::from_chars(temp.data(), temp.data() + temp.size(), value);
                metric.value = value;

                // Загружаем в общий список
                result.second.push_back(metric);

                ss >> temp;
            }
        }

        return result;
    }

    std::string LogReader::_getLastLine() noexcept {
        std::ifstream file(_log_file, std::ios::ate | std::ios::binary);
        std::string result{};

        if (file.is_open()) {
            // Сохраняем новое значение размера файла
            _current_size_file = file.tellg();
            long long pos = _current_size_file - 1;

            if (_current_size_file == 0) {
                return result;
            }

            file.seekg(pos);
            pos -= (file.peek() == '\n') ? 1 : 0;

            while (pos >= 0) {
                file.seekg(pos);

                if (file.peek() == '\n') {
                    break;
                }
                --pos;
            }

            file.seekg(pos + 1);
            std::getline(file, result);
        }

        return result;
    }
}