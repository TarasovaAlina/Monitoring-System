#include "core/kernel.h"

namespace core {
    Kernel::Kernel() noexcept
    : _work_flag(true) {
        try {
            _kernel_manager = std::make_unique<KernelManager>();
            _logger = std::make_unique<Logger>();
            _notification_service = std::make_unique<NotificationServer>();

            _search_agents_thread = std::thread(&Kernel::_searchNewAgents, this);

        } catch (const std::exception& e) {
            _work_flag.store(false);
        }
    }

    Kernel::~Kernel() noexcept {
        _work_flag.store(false);
    }

    bool Kernel::isCorrect() noexcept {
        return _work_flag.load();
    }

    void Kernel::_searchNewAgents() noexcept {
        // Происходит поиск до тех пор, пока нет сигнала завершения
        while (_work_flag.load()) {

        }
    }


}