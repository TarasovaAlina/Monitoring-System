

class MemoryMetricCollector : public IMetricCollector {

    Metrics collectRamTotalAndRam() const;
    Metrics collectHardVolume() const;
    Metrics collectHardOps() const;
    Metrics collectHardThroughput() const;

public:
    Metrics collectMetrics() const override;

};

Metrics MemoryMetricCollector::collectRamTotalAndRam() const {
    std::ifstream file("/proc/meminfo");
    double ram_total_, ram_free_, ram_usage_{};
    Metrics metrics_{};

    if (file.is_open()) {
        std::string line;
        std::getline(file, line);
        line >> ram_total_;
        std::getline(file, line);
        line >> ram_free_;
    }
    file.close();

    ram_usage_ = (ram_total_ - ram_free_) / ram_total_ * 100.;

    metrics_.emplace_back(Metric{"ram_total_", ram_total_});
    metrics_.emplace_back(Metric{"ram_usage_", ram_usage_});

    return metrics_;
}

Metrics MemoryMetricCollector::collectHardVolume() const {
    struct statvfs stat{};
    Metrics metrics_{};

    if (statvfs("/", &stat) == 0) {
        const auto total = stat.f_blocks * stat.f_frsize;
        const auto available = stat.f_bavail * stat.f_frsize;
        const auto used = total - available;

        metrics_.emplace_back(Metric{"DiskUsage" ,100.0 * static_cast<double>(used) / total});
    }

    return metrics_;
}

Metrics MemoryMetricCollector::collectHardOps() const {
    std::string device{"sda"};
    auto readOps = [&device]() -> long long {
        std::ifstream file("/proc/diskstats");

        if (!file.is_open()) {
            return -1;
        }

    
    int intervalSeconds = 1
    long long read_ops{};
    long long write_ops{};
    Metrics metrics_{};
    std::string line;

    while (std::getline(file, line)) {
            std::istringstream ss(line);

            int major, minor;
            std::string name;

            long long read_ops{};
            long long read_merges{};
            long long read_sectors{};
            long long read_time{};

            long long write_ops{};
            long long write_merges{};
            long long write_sectors{};
            long long write_time{};

            ss >> major >> minor >> name
               >> read_ops >> read_merges >> read_sectors >> read_time
               >> write_ops >> write_merges >> write_sectors >> write_time;

            if (name == device) {
                return read_ops + write_ops;
            }
        }

        return -1;
    };

    const long long start = readOps();

    if (start < 0) {
        return -1;
    }

    std::this_thread::sleep_for(
        std::chrono::seconds(intervalSeconds)
    );

    const long long end = readOps();

    if (end < 0) {
        return -1;
    }

    metrics_.emplace_back(Metric{"hard_ops_", (end - start) / intervalSeconds});

    return metrics_;
}

Metrics MemoryMetricCollector::collectHardThroughput() const {
    std::string device{"sda"};
    int intervalSeconds = 1
    Metrics metrics_{};

    auto readSectors = [&device]() -> long long {
        std::ifstream file("/proc/diskstats");

        if (!file.is_open()) {
            return -1;
        }

        std::string line;

        while (std::getline(file, line)) {
            std::istringstream ss(line);

            int major, minor;
            std::string name;

            long long readOps{};
            long long readMerges{};
            long long readSectors{};
            long long readTime{};

            long long writeOps{};
            long long writeMerges{};
            long long writeSectors{};
            long long writeTime{};

            ss >> major >> minor >> name
               >> readOps >> readMerges >> readSectors >> readTime
               >> writeOps >> writeMerges >> writeSectors >> writeTime;

            if (name == device) {
                return readSectors + writeSectors;
            }
        }

        return -1;
    };

    constexpr double sectorSize = 512.0;

    const long long start = readSectors();

    if (start < 0) {
        return -1.0;
    }

    std::this_thread::sleep_for(
        std::chrono::seconds(intervalSeconds)
    );

    const long long end = readSectors();

    if (end < 0) {
        return -1.0;
    }

    const long long deltaSectors = end - start;

    const double bytes =
        deltaSectors * sectorSize;

    const double seconds =
        static_cast<double>(intervalSeconds);

    metrics_.emplace_back(Metric{"hard_throughput_", bytes / seconds / (1024.0 * 1024.0)});

    return metrics_;

}

Metrics MemoryMetricCollector::collectMetrics() const {
    Metrics metrics_ { collectRamTotalAndRam() + collectHardVolume() 
        + collectHardOps() + collectHardThroughput() };
    
    return metrics_;
}