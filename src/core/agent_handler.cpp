#include "core/agent_handler.h"
#include <dlfcn.h>

namespace core {
    AgentHandler::AgentHandler(const std::string& path, agent::AgentType type, int milliseconds): _type{type}, _timeout{milliseconds} {
        _lib_agent = dlopen(path.c_str(), RTLD_LAZY);

        if (!_lib_agent) { throw std::runtime_error("File is not found."); }


    }
}