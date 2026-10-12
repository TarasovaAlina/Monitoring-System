#ifndef SYSTEM_MONITORING_NOTIFICATION_CHANNELS_H
#define SYSTEM_MONITORING_NOTIFICATION_CHANNELS_H

#include <string>

/**
 * @file notification_channels.h
 * @brief В этом файле описаны классы TelegramChannel и EmailChannel
 * @author Georgiy Kovalev
 * @date 12.10.2026
 */

#define TELEGRAM_CONF "config/telegram.json"
#define EMAIL_CONF "config/email.json"

namespace tools {
    /**
     * @class INotificationChannel
     * @brief Абстрактный интерфейс для каналов уведомлений
     */
    class INotificationChannel {
    public:
        virtual ~INotificationChannel() = default;
        virtual void send(const std::string& message) = 0;
    };

    /**
     * @class TelegramChannel
     * @brief Канал передачи сообщений через бота в Telegram
     */
    class TelegramChannel final : public INotificationChannel {
    public:
        /**
         * @brief Инициализация канала связи.
         * Считывает настройки из файла config/telegram.json.
         * Данные хранятся в следующем формате:
         * - token: токен бота
         * - id: ID чата или канала
         */
        TelegramChannel() noexcept;

        /**
         * @brief Реализация логики передачи сообщения через Telegram
         * @param message Сообщение, которое нужно передать пользователю
         */
        void send(const std::string& message) override;

    private:
        std::string _bot_token; ///< Токен бота
        std::string _chat_id; ///< ID чата или канала (в формате '@my_monitoring_channel')
    };

    /**
     * @class EmailChannel
     * @brief Реализация передачи сообщения через Email-сообщение
     */
    class EmailChannel final : public INotificationChannel {
    public:
        /**
         * @brief Инициализация канала связи.
         * Считывает настройки из файла config/email.json.
         * Данные хранятся в следующем формате:
         * - url: адрес SMTP-сервера
         * - sender: email-адрес отправителя
         * - recipient: email-адрес получателя
         * - password: Пароль учетной записи отправителя
         */
        EmailChannel() noexcept;

        /**
         * @brief Реализация передачи сообщения по протоколу SMTP
         * @param message Сообщение, которое нужно передать пользователю
         */
        void send(const std::string& message) override;

    private:
        /**
         * @brief Адрес SMTP-сервера
         * @note "gmail.com", "yandex.com" и т.д
         */
        std::string _smtp_url;

        std::string _sender; ///< Email-адрес отправителя (с которого будет отправляться письмо)
        std::string _recipient; ///< Email-адрес получателя (кому придет уведомление о критической метрике)
        std::string _password; ///< Пароль учетной записи отправителя
    };
}

#endif
