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

    EmailChannel::EmailChannel() noexcept {
        // Считываем данные конфигурации
        std::ifstream file(EMAIL_CONF);
        json config = json::parse(file);

        if (config.contains("url") && config["url"].is_string()) {
            _smtp_url = config["url"].get<std::string>();
        }

        if (config.contains("sender") && config["sender"].is_string()) {
            _sender = config["sender"].get<std::string>();
        }

        if (config.contains("recipient") && config["recipient"].is_string()) {
            _recipient = config["recipient"].get<std::string>();
        }

        if (config.contains("password") && config["password"].is_string()) {
            _password = config["password"].get<std::string>();
        }
    }

    void EmailChannel::send(const std::string &message) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            throw std::runtime_error("Failed to initialize libcurl");
        }

        // Формируем MIME-заголовки и тело письма
        std::string emailBody =
            "From: <" + _sender + ">\r\n"
            "To: <" + _recipient + ">\r\n"
            "Subject: Monitoring Alert: Metric Threshold\r\n"
            "\r\n" + message + "\r\n";

        struct EmailPayload payload = { emailBody.c_str(), emailBody.size() };

        curl_easy_setopt(curl, CURLOPT_URL, _smtp_url.c_str());
        curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_USERNAME, _sender.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, _password.c_str());
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, ("<" + _sender + ">").c_str());

        struct curl_slist* recipients = nullptr;
        recipients = curl_slist_append(recipients, ("<" + _recipient + ">").c_str());
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        curl_easy_setopt(curl, CURLOPT_READFUNCTION, _PayloadSource);
        curl_easy_setopt(curl, CURLOPT_READDATA, &payload);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        CURLcode res = curl_easy_perform(curl);

        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    }

    size_t EmailChannel::_PayloadSource(void *ptr, size_t size, size_t nmemb, void *userp) {
        struct EmailPayload* email = (struct EmailPayload*)userp;
        size_t bufferSize = size * nmemb;
        if (bufferSize > email->size) {
            bufferSize = email->size;
        }
        if (bufferSize > 0) {
            memcpy(ptr, email->data, bufferSize);
            email->data += bufferSize;
            email->size -= bufferSize;
            return bufferSize;
        }
        return 0;
    }

}