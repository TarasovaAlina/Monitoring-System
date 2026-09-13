#include "agent/network_monitoring_agent.h"
#include <fstream>
#include <sstream>
#include <bits/this_thread_sleep.h>

namespace agent {

    NetworkAgent::NetworkAgent() noexcept : IAgent(NETWORK_AGENT), _inet_throughput(0.0) {}

    std::map<std::string, std::pair<unsigned long, unsigned long>> NetworkAgent::_readNetInterfaces() noexcept {
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
                result[label] = std::make_pair<unsigned long, unsigned long>{receive_bytes, transmit_bytes};
            }
        }

        return result;
    }

    void NetworkAgent::updateMetrics() noexcept {
        // Считываем данные два раза
        auto result_before = _readNetInterfaces();
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        auto result_after = _readNetInterfaces();

        double throughputs = 0.0;
        int count = 0;
        for (const auto& [interface, count_bytes]: result_after) {
            if (result_before.contains(interface)) {
                auto det_receive_bytes = count_bytes.first - result_before[interface].first;
                auto det_transmit_bytes = count_bytes.second - result_before[interface].first;

                throughputs += static_cast<double>(det_receive_bytes + det_transmit_bytes) * 0.33;
                count++;
            }
        }

        _inet_throughput = throughputs / count;
    }

    void NetworkAgent::addURL(const std::string &url) noexcept {
        // Доступность url определится в updateMetrics()
        _url_availability[url] = false;
    }

    std::vector<Metric> NetworkAgent::getMetrics() noexcept {
        std::vector<Metric> result {Metric("inet_throughput", _inet_throughput)};

        for (const auto& [url, availability]: _url_availability) {
            result.push_back(Metric(url, availability == true));
        }

        return result;
    }

}
