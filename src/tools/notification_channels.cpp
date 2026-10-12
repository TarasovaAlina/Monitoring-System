#include "tools/notification_service.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <cpr/cpr.h>

using nlohmann::json;

namespace tools {
    TelegramChannel::TelegramChannel() noexcept {
        std::ifstream file(TELEGRAM_CONF);
        json config = json::parse(file);

        if (config.contains("token") && config["token"].is_string()) {
            _bot_token = config["token"].get<std::string>();
        }

        if (config.contains("id") && config["id"].is_string()) {
            _chat_id = config["id"].get<std::string>();
        }
    }

    void TelegramChannel::send(const std::string &message) {
        std::string url = "https://api.telegram.org/bot" + _bot_token + "/sendMessage";

        // Выполняем POST-запрос к Telegram Bot API
        cpr::Response r = cpr::Post(
            cpr::Url{url},
            cpr::Payload{{"chat_id", _chat_id}, {"text", message}}
        );
    }

}