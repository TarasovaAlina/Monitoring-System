#ifndef SYSTEM_MONITORING_KERNEL_MANAGER_H
#define SYSTEM_MONITORING_KERNEL_MANAGER_H

#include "tools/agent_service.h"
#include "tools/config_service.h"
#include <memory>

/**
 * @file kernel_manager.h
 * @brief В этом файле описан класс KernelManager
 * @date 15.09.2026
 * @authors Tarasova Alina, Georgiy Kovalev
 */

#define SCAN_TIMEOUT 5000

namespace core {
    /**
     * @class KernelManager
     * @brief Этот класс отвечает за всю внутреннюю логику,
     * связанную с агентами и взаимодействие с ними
     */
    class KernelManager {
    public:
        KernelManager();

        AgentInfo getAgentInfo(const std::string& name) noexcept;

        /**
         * @brief Производится поиск в директории ./agents динамических библиотек через каждые SCAN_TIMEOUT мс.
         * Если какая-то библиотека, которая раньше находилась в директории, исчезла,
         * то соответствующий агент выгружается из программы. Если при сканировании найдется новая библиотека,
         * то она загружается в программу как новый агент
         */
        void scan() noexcept;

    private:
        std::unique_ptr<AgentService> _agent_service; ///< Управляет агентами, загруженными как динамические библиотеки
        std::unique_ptr<ConfigService> _config_service; ///< Считывает конфиги агентов и передает внутренние данные
    };
}

#endif