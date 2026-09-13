

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

/*
SharedLibrary
     │
     ├── dlopen()  → загрузить .so
     │
     ├── dlsym()   → найти функцию внутри .so
     │
     └── dlclose() → выгрузить .so
*/

class SharedLibrary {
public:
    explicit SharedLibrary(const std::string& path);

    ~SharedLibrary();

    SharedLibrary(const SharedLibrary&) = delete;
    SharedLibrary& operator=(const SharedLibrary&) = delete;

    SharedLibrary(SharedLibrary&& other) noexcept;
    SharedLibrary& operator=(SharedLibrary&& other) noexcept;

    void* symbol(const char* name) const;

private:
    void* handle_{nullptr};
};

struct AgentDeleter {
    DestroyAgentFn destroy = nullptr;

    void operator()(IAgent* agent) const {
        if (agent && destroy) {
            destroy(agent);
        }
    }
};

// связывает конкретную библиотеку с конкретным экземпляром агента.
struct AgentHandle {
    std::string name;
    std::string path;

    SharedLibrary library;

    std::unique_ptr<IAgent, AgentDeleter> agent;

    bool enabled{true};
};

//  отвечает за жизненный цикл агентов
class AgentService {
    std::unordered_map<std::string, AgentHandle> agents_;

public:
    void scan();
    void load(const std::string& path);
    void unload(const std::string& name);

    void enable(const std::string& name);
    void disable(const std::string& name);

    std::vector<Metric> collectMetrics();

};