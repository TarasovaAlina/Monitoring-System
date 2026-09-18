#include <fstream>
using json = nlohmann::json;

/**
 * @file config_service.h
 * @brief В этом файле описан класс сервера,
 * который будет возвращать данные из config для дальнейшей работы с агентами
 * @author Tarasova Alina
 * @date 18.09.2026
 */

/**
 * @struct Threshold
 * @brief Содержит информацию о значении конкретной метрике и ператоре сравнения
 * таким как <, <=, ==, >=, >
 */
struct Threshold {
    std::string operation; ///< Оператор сравнения
    double value; ///< Значение метрики
};

/**
 * @struct MetricConfig
 * @brief Содержит информацию об отдельной метрике
 */
struct MetricConfig {
    std::string target; ///< Название метрики
    Threshold threshold; ///< Структура с численным значением и оператором сравнения
};

/**
 * @struct ConfigInfo
 * @brief Содержит информацию о конкретном агенте:
 * имени агента, типе агента, интервале обновления метрик и
 * сам список метрик
 */

struct ConfigInfo {
    std::string agentName_; ///< Имя агента
    std::string agentType_; ///< Тип агента (соответствует AgentType)
    int updateInterval_; ///< Время обновления метрик
    std::vector<MetricConfig> metricConfig_; ///< Список критических значений метрик
};

/**
 * @class ConfigService
 * @brief Класс по работе с config файлом
 */
class ConfigService {
public:
    /**
     * @brief Обработка config файла для последующей загрузки в агентов
     * @return вектор с информацией для каждого агента с соответствующими параметрами
     */
    std::vector<ConfigInfo> load() const override;
};