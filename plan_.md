                         Kernel
                           │
                           ↓
                    IAgentService
                           │
                           ↓
                     AgentService
                           │
             ┌─────────────┼─────────────┐
             ↓             ↓             ↓
      AgentManager   ConfigService    ...
             │
             ↓
       IAgentLoader
             │
       ┌─────┴─────┐
       ↓           ↓
    cpu.so      memory.so ...
       │
       ↓
     IAgent
       │
       ↓
 Metrics
       ↑
       │
 MetricCollector
       │
       ↓
      /proc