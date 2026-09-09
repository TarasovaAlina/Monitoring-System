class MemoryAgent : public IAgent {
    double ram_total_; // cat /proc/meminfo | grep MemTotal
    double ram_;
    double hard_volume_; //std::filesystem::space()
    int hard_ops_; // /proc/diskstats
    double hard_throughput_;

    std::unique_ptr<IMetricCollector> metricCollector_;

public:
    MemoryAgent(): metricCollector_{std::make_unique{MemoryMetricCollector}} {}
    Metrics updateMetrics() override;
};

Metrics MemoryAgent::updateMetrics() {
    return metricCollector_->collectMetrics();
}