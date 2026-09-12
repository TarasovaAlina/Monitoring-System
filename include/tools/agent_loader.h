// отвечает за техническую загрузку .so
class AgentLoader : public IAgentLoader {
public:
    //здесь выгружаются созданные агенты .so
    std::vector<AgentHandle> AgentLoader::loadAgents()
    {
        std::vector<AgentHandle> result;

        for (const auto& entry :
            std::filesystem::directory_iterator("./agents")) {

            if (entry.path().extension() != ".so")
                continue;

            result.push_back(load(entry.path()));
        }

        return result;
    }
    virtual void scan() = 0; //выгрузить агента

    // Программа-ядро в отдельном потоке должна с заданной
    // периодичностью в несколько секунд сканировать папку
    // ./agents/ на наличие новых агентов, которых необходимо
    // подключить к системе и отобразить в интерфейсе.
};