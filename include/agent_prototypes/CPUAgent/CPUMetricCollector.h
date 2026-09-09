

class CPUMetricCollector : public IMetricCollector {
    Metrics collectCpu() const;
    Metrics collectProcesses() const;

public:
    Metrics collectMetrics() const override;

};

Metrics CPUMetricCollector::collectCpu() const {
    std::ifstream file("/proc/loadavg");
    double 1_m{}, 5_m{}, 15_m{};
    Metrics metrics_{};

    if (file.is_open()) {
        std::string line;
        std::getline(file, line);
        line >> 1_m >> 5_m >> 15_m;
    }
    file.close();

    metrics_.emplace_back(Metric{"1 minute", 1_m});
    metrics_.emplace_back(Metric{"5 minutes", 5_m});
    metrics_.emplace_back(Metric{"15 minutes", 15_m});

    return metrics_;
}

Metrics CPUMetricCollector::collectMetrics() const {
    Metrics res{};
    res = collectCpu() + collectProcesses();

        return res;
}

Metrics CPUMetricCollector::collectProcesses() const {
    std::ifstream file("/proc");
    double 1_m{}, 5_m{}, 15_m{};
    Metrics metrics_{};

    if (file.is_open()) {
        std::string line;
        std::getline(file, line);
        line >> 1_m >> 5_m >> 15_m;
    }
    file.close();

    metrics_.emplace_back(Metric{"1 minute", 1_m});
    metrics_.emplace_back(Metric{"5 minutes", 5_m});
    metrics_.emplace_back(Metric{"15 minutes", 15_m});

    return metrics_;
}

Metrics CPUMetricCollector::collectMetrics() const {
    int count{};
    Metrics metrics_{};

    for (const auto& entry : std::filesystem::directory_iterator("/proc")) {
        if (entry.is_directory()) {
            const std::string name = entry.path().filename().string();

            if (!name.empty() &&
                std::all_of(name.begin(), name.end(), ::isdigit)) {
                ++count;
            }
        }
    }

    metrics_.emplace_back(Metric{"processes", count});

    return metrics_;
}