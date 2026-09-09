class IAgentService {
public:
    ~IAgentService() = default;

    virtual void updateAgents() = 0;
    virtual Metrics collectMetrics() = 0;
};