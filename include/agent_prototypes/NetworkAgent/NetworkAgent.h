class NetworkAgent : public IAgent {
    int url_;
    double inet_throughput_;
    std::unique_ptr<IMetricCollector> metricCollector_;

public:
    NetworkAgent(): metricCollector_{std::make_unique{NetworkMetricCollector}} {}
    Metrics updateMetrics() override;
};

Metrics NetworkAgent::updateMetrics() {
    return metricCollector_->collectMetrics();
}