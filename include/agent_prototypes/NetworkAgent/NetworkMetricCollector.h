

class NetworkMetricCollector : public IMetricCollector {

    std::string address;
    std::string interface;

    Metrics collectUrl() const;
    Metrics collectInetThroughput() const;
public:
    Metrics collectMetrics() const override;
};

Metrics NetworkMetricCollector::collectUrl() const {
    Metrics metrics_{};
    std::string command = "ping -c 1 -W 1 " + address + " > /dev/null 2>&1";

    metrics_.emplace_back(Metric{"<url>", std::system(command.c_str()) == 0 ? 1 : 0});

    return metrics_;
}

Metrics NetworkMetricCollector::collectInetThroughput() const {
    int intervalSeconds = 1;
    Metrics metrics_{};

    auto readBytes = [&interface]() -> long long {
        std::ifstream file("/proc/net/dev");

        if (!file.is_open()) {
            return -1;
        }

        std::string line;

        while (std::getline(file, line)) {
            if (line.find(interface + ":") == std::string::npos) {
                continue;
            }

            auto colon = line.find(':');

            std::istringstream ss(line.substr(colon + 1));

            long long rxBytes{};
            long long rxPackets{};
            long long rxErrors{};
            long long rxDrops{};
            long long rxFifo{};
            long long rxFrame{};
            long long rxCompressed{};
            long long rxMulticast{};

            long long txBytes{};
            long long txPackets{};

            ss >> rxBytes
               >> rxPackets
               >> rxErrors
               >> rxDrops
               >> rxFifo
               >> rxFrame
               >> rxCompressed
               >> rxMulticast
               >> txBytes
               >> txPackets;

            return rxBytes + txBytes;
        }

        return -1;
    };

    const long long start = readBytes();

    if (start < 0) {
        return -1.0;
    }

    std::this_thread::sleep_for(
        std::chrono::seconds(intervalSeconds)
    );

    const long long end = readBytes();

    if (end < 0) {
        return -1.0;
    }

    const long long deltaBytes = end - start;

    metrics_.emplace_back(Metric{"inet_throughput_", 
        static_cast<double>(deltaBytes) / intervalSeconds / (1024.0 * 1024.0)});

    return metrics_;
}

Metrics NetworkMetricCollector::collectMetrics() const {
    Metrics metrics_ { collectUrl() + collectInetThroughput() };
    
    return metrics_;
}