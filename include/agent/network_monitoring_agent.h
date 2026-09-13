#ifndef SYSTEM_MONITORING_NETWORK_MONITORING_AGENT_H
#define SYSTEM_MONITORING_NETWORK_MONITORING_AGENT_H

#include "agent.h"
#include <map>
#include <string>

/**
 * @file network_monitoring_agent.h
 * @brief В этом файле описан класс агента NetworkAgent
 * @author Georgiy Kovalev
 * @date 04.09.2026
 */

namespace agent {
    /**
     * @class NetworkAgent
     * @brief Собирает информацию об использовании сети в системе
     */
    class NetworkAgent final : public IAgent {
    public:
        NetworkAgent() noexcept;

        /**
         * @brief Обновляет метрики использования сетевых интерфейсов системы.\n
         *
         * Для получения доступности url используется библиотека libcurl.
         * С ее помощью отправляется HTTP-запрос типа HEAD до нужного сайта и ожидается код ответа 2XX или 3XX
         *
         * Для расчета пропускной способности парсится файл @code/proc/net/dev@endcode:
         * - Поле 1: название интерфейса
         * - Поле 2: кол-во принятых байтов
         * - Поле 10: кол-во отправленных байтов
         *
         * Для этого через определенный промежуток времени считываются значения байтов.
         * Пропускная способность рассчитывается по формуле:\n
         * @codethroughput = ((receive_bytes2 - receive_bytes1) + (transmit_bytes2 - transmit_bytes1)) / det_time
         */
        void updateMetrics() noexcept override;

        /**
         * @brief Добавляет новый url для отслеживания его доступности
         * @param url Адрес сайта, который указан в конфиге
         */
        void addURL(const std::string& url) noexcept;

        /**
         * @return Значения метрик <url>, inet_throughput
         */
        std::vector<Metric> getMetrics() noexcept override;

    private:
        /**
         * @brief Считывает 2-е и 10-е поля из файла /proc/net/dev для каждого сетевого интерфейса
         * @return Пара <считанные_байты; отправленные_байты> для каждого сетевого интерфейса, доступного в системе
         */
        static std::map<std::string, std::pair<unsigned long, unsigned long>> _readNetInterfaces() noexcept;

        std::map<std::string, bool> _url_availability; ///< Доступность url, указанных в конфиге
        double _inet_throughput; ///< Средняя пропускная способность доступных сетевых интерфейсов
    };
}

#endif