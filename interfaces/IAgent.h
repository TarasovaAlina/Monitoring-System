class IAgent {
public:
    ~IAgent() = default;

    virtual Metrics updateMetrics() = 0; //функция по тз
};