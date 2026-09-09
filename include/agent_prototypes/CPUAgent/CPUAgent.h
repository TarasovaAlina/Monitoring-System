class CPUAgent : public IAgent {
    double cpu_; // cat /proc/loadavg
    int processes_; //кол-во числовых подкаталогов в /proc

    std::unique_ptr<IMetricCollector> metricCollector_;

public:
    CPUAgent(): metricCollector_{std::make_unique{CPUMetricCollector}} {}
    Metrics updateMetrics() override;
};

Metrics CPUAgent::updateMetrics() {
    return metricCollector_->collectMetrics();
}