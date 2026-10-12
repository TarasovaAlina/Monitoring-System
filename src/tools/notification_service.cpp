#include "tools/notification_service.h"
#include <fstream>

namespace tools {
    NotificationService::NotificationService() noexcept
    : _telegram_channel(std::make_unique<TelegramChannel>())
    , _email_channel(std::make_unique<EmailChannel>())
    , _is_enable_duplication(false) {}

    bool &NotificationService::emailDuplication() noexcept {
        return _is_enable_duplication;
    }

    void NotificationService::notify(std::string metric_name, double value) noexcept {
        std::ifstream file("/etc/hostname");
        std::string host_name;

        // Считываем имя хоста для формирования сообщения
        file >> host_name;
        file.close();

        // Формируем сообщение для отправки
        std::string message =
            "⚠️ Alert from Host: " + host_name + "\n" +
            "📊 Metric: " + metric_name + "\n" +
            "📈 Value: " + value;

        // Отправляем сообщение для последующей передачи
        _telegram_channel->send(message);
        if (_is_enable_duplication) {
            _email_channel->send(metric_name);
        }
    }
}