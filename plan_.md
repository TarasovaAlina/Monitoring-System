                         Kernel
                           │
                           ↓
                     KernelManager
                           │
             ┌─────────────┼─────────────┐
             ↓             ↓             ↓
      AgentService   ConfigService      ...
             │
             ↓
        AgentLoader
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