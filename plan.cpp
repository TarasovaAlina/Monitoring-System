/*
Агенты
должны будут собирать метрики и передавать их ядру, которое логирует
данные метрики. При достижении критических значений система должна
отправлять уведомления пользователю.
*/

class Kernel {
    std::unique_ptr<IKernelManager> KernelManager_;
    std::unique_ptr<ILogger> logger_;
    std::unique_ptr<INotificationService> notificationService_;

    //в потоке или в очереди
    void 
public:

    // записывать актуальные метрики в виде 
    // списка в журнал
    // системы мониторинга в папке ./logs/ .
    void putLogs() {
        const Metrics metrics = KernelManager_->collectMetrics();
        logger_->write(metrics);
        if (KernelManager_->compareMetrics(metrics))
            notifyUser();
    }
    
    void notifyUser();

};

class IKernelManager {
public:
    ~IKernelManager() = default;

    virtual void updateAgents() = 0;
    virtual Metrics collectMetrics() = 0;
};

class KernelManager : public IKernelManager {
    std::unique_ptr<IAgentLoader> agentLoader_;
    std::unique_ptr<AgentService> AgentService_;
    std::unique_ptr<IAgentConfigService> agentConfigService_;
public:
    void updateAgents() override;

    //возвращет общий metrics, берет из AgentService::updateMetrics
    Metrics collectMetrics() override;
    bool compareMetrics(Metrics& metrics_) const;
};

//  отвечает за жизненный цикл агентов
class AgentService {
    std::vector<std::unique_ptr<IAgent>> agents_;

public:
    std::vector<std::unique_ptr<IAgent>>& agents_() const noexcept;

    void scan(); //проверить новых агентов
    Metrics updateMetrics(); //обновить метрики
};

// отвечает за техническую загрузку .so
class IAgentLoader {
    virtual ~IAgentLoader() = default;
    //здесь выгружаются созданные агенты .so
    virtual void load() = 0; //загрузить агента
    virtual void scan() = 0; //выгрузить агента

    // Программа-ядро в отдельном потоке должна с заданной
    // периодичностью в несколько секунд сканировать папку
    // ./agents/ на наличие новых агентов, которых необходимо
    // подключить к системе и отобразить в интерфейсе.
};

//основной класс агента, может быть несколько
// хранятся в каталоге ./agents/
class IAgent {
public:
    ~IAgent() = default;

    virtual Metrics updateMetrics() = 0; //функция по тз
};

class CPUAgent : public IAgent {
    double cpu_; // cat /proc/loadavg
    int processes_; //кол-во числовых подкаталогов в /proc

    std::unique_ptr<IMetricCollector> metricCollector_;
};

class MemoryAgent : public IAgent {
    double ram_total_; // cat /proc/meminfo | grep MemTotal
    double ram_;
    double hard_volume_; //std::filesystem::space()
    int hard_ops_; // /proc/diskstats
    double hard_throughput_;

    std::unique_ptr<IMetricCollector> metricCollector_;
};

class NetworkAgent : public IAgent {
    int url_;
    double inet_throughput_;
    std::unique_ptr<IMetricCollector> metricCollector_;
};

class ILogger {
    virtual ~ILogger() = default;
    virtual void write(const Metrics& metrics) = 0;
};

struct Metric {
    std::string name;
    double value;
};

using Metrics = std::vector<Metric>;

//класс для считывания метрик
class IMetricCollector {
public:
    ~IMetricCollector() = default;
    virtual Metrics collectMetrics() const = 0;
};

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

class MemoryMetricCollector : public IMetricCollector {
    double ram_total_;
    double ram_;
    double hard_volume_;
    int hard_ops_;
    double hard_throughput_;

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

struct Threshold {
    std::string operation;
    double value;
};

struct MetricConfig {
    std::string target;
    Threshold threshold;
};

class IAgentConfigService {
public:
    ~IAgentConfigService() = default;
    virtual Threshold parseCondition(const std::string& condition) const = 0;
    virtual std::vector<MetricConfig> load() const = 0;
    virtual bool compareMetrics(const Metrics& metrics_) const = 0;
};

class AgentConfigService : public IAgentConfigService {
public:

    std::vector<MetricConfig> load();
    Threshold parseCondition(const std::string& condition);
    bool compareMetrics(Metrics& metrics_) const;
};

std::vector<MetricConfig> load() {
    std::ifstream file("config/config.json");
}

bool checkThreshold(double actual, const Threshold& threshold);


class INotificationService {
    ~INotificationService() = default;

    virtual void notify(const Alert& alert) = 0;
};

// Вместо базы данных с метриками предлагаем использовать единый
// лог-файл для всей системы мониторинга