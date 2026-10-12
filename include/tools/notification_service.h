#ifndef SYSTEM_MONITORING_NOTIFICATION_SERVICE_H
#define SYSTEM_MONITORING_NOTIFICATION_SERVICE_H

#include "tools/notification_channels.h"

/**
 * @file notification_service.h
 * @brief В этом файле описан класс NotificationService
 * @author Georgiy Kovalev
 * @date 12.10.2026
 */

namespace tools {
    /**
     * @class NotificationService
     * @brief Управляет каналами передачи сообщения пользователю
     */
    class NotificationService {
    public:

        /**
         * @brief Доступ к приватному полю _is_enable_duplication
         */
        static bool& emailDuplication() noexcept;

        /**
         * @brief Формирует сообщение и передает его _telegram_channel и _email_channel
         * @param metric_name Имя метрики, которая достигла критического значения
         * @param value Значение этой метрики
         */
        static void notify(std::string metric_name, double value) noexcept;

    private:
        static TelegramChannel _telegram_channel;
        static EmailChannel _email_channel;
        static bool _is_enable_duplication; ///< Разрешено ли дублирование сообщения на email-адрес
    };

}

#endif
