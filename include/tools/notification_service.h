#ifndef SYSTEM_MONITORING_NOTIFICATION_SERVICE_H
#define SYSTEM_MONITORING_NOTIFICATION_SERVICE_H

#include "tools/notification_channels.h"
#include <memory>

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
         * @brief Инициализирует каналы связи - через бота в Telegram и Email
         */
        NotificationService() noexcept;

        /**
         * @brief Устанавливает возможность дублирования сообщения
         * на указанный email-адрес
         * @param enable Будет ли включена/выключена эта функция
         */
        void enableEmailDuplication(bool enable) noexcept;

        /**
         * @brief Формирует сообщение и передает его _telegram_channel и _email_channel
         * @param metric_name Имя метрики, которая достигла критического значения
         * @param value Значение этой метрики
         */
        void notify(std::string metric_name, double value) noexcept;

    private:
        std::unique_ptr<INotificationChannel> _telegram_channel;
        std::unique_ptr<INotificationChannel> _email_channel;
        bool _is_enable_duplication; ///< Разрешено ли дублирование сообщения на email-адрес
    };

}

#endif
