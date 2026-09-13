class Kernel {
    std::unique_ptr<IKernelManager> kernelManager_;
    std::unique_ptr<ILogger> logger_;
    std::unique_ptr<INotificationService> notificationService_;

public:

    // записывать актуальные метрики в виде 
    // списка в журнал
    // системы мониторинга в папке ./logs/ .
    void putLogs() {
        const Metrics metrics = kernelManager_->collectMetrics();
        logger_->write(metrics);
        if (agentService_->compareMetrics(metrics))
            notifyUser();
    }
    void mainprocess();
    // будет работать в бесконечном цикле по выходу
    // в отдельном потоке будет вызываться agentService_->

    void notifyUser();

};