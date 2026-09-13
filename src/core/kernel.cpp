#include "core/kernel.h"

namespace core {
    Kernel::Kernel() noexcept
    : _kernel_manager(std::make_unique<KernelManager>())
    , _logger(std::make_unique<Logger>())
    , _notification_service(std::make_unique<NotificationService>())
    , _work_flag(true) {
        _search_agents_thread = std::thread
    }

    Kernel::~Kernel() noexcept {
        _work_flag.store(false);
    }


    void Kernel::_searchNewAgents() noexcept {
        // Происходит поиск до тех пор, пока нет сигнала завершения
        while (_work_flag.load()) {

        }
    }


}