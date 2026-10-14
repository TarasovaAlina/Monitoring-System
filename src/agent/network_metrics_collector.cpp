#include "agent/metrics_collector.h"
#include <fstream>
#include <sstream>
#include <thread>
#include <cpr/cpr.h>

namespace agent {
    std::vector<Metric> NetworkMetricsCollector::update(const std::vector<std::string>& metric_names_list) noexcept {
        std::vector<Metric> result;
        result.reserve(metric_names_list.size());

        for (const auto& name : metric_names_list) {
            if (name == "inet_throughput") {
                // Считываем данные два раза
                auto result_inet = _readNetInterfaces();
                auto cur_point = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration<double>(cur_point - _last_point).count();

                double throughput = 0.0;
                int count = 0;
                for (const auto& [interface, count_bytes]: result_inet) {
                    if (_net_stat.contains(interface)) {
                        auto det_receive_bytes = count_bytes.first - _net_stat[interface].first;
                        auto det_transmit_bytes = count_bytes.second - _net_stat[interface].first;

                        throughput += static_cast<double>(det_receive_bytes + det_transmit_bytes) / elapsed;
                        count++;
                    }
                }

                result.emplace_back(Metric("inet_throughput", throughput / count));
                _net_stat = result_inet;
            }

            else {
                result.emplace_back(Metric(name, _isUrlAvailable(name)));
            }
        }

        return result;
    }

    std::map<std::string, std::pair<unsigned long, unsigned long> > NetworkMetricsCollector::_readNetInterfaces() noexcept {
        std::ifstream file("/proc/net/dev");
        std::map<std::string, std::pair<unsigned long, unsigned long>> result;

        if (file.is_open()) {
            std::string line, label;
            std::stringstream ss;

            // Необходимые данные начинаются с третьей строки
            std::getline(file, line);
            std::getline(file, line);
            while (std::getline(file, line)) {
                ss = std::stringstream(line);
                ss >> label; // Считываем название интерфейса
                label = label.substr(1, label.length() - 1); // Отсекаем символ ':' в конце

                unsigned long temp, receive_bytes, transmit_bytes;
                ss >> receive_bytes; // Считываем количество принятых байт на данный момент
                ss >> temp >> temp >> temp >> temp >> temp >> temp >> temp;
                ss >> transmit_bytes;

                // Записываем считанные данные в словарь
                result[label] = {receive_bytes, transmit_bytes};
            }
        }

        return result;
    }

    bool NetworkMetricsCollector::_isUrlAvailable(const std::string &url) noexcept {
        cpr::Response response = cpr::Head(
            cpr::Url{url},
            cpr::Timeout{200},
            cpr::Redirect{true});

        // Считаем сайт доступным, если статус ответа в диапазоне 200-399
        return response.status_code >= 200 && response.status_code < 400;
    }
}