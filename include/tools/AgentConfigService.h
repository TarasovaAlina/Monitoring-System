#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class AgentConfigService : public IAgentConfigService {
public:

    std::vector<MetricConfig> load() const override;
    Threshold parseCondition(const std::string& condition) const override;
    bool compareMetrics(const Metrics& metrics_) const override;
};

std::vector<MetricConfig> load() const {
    std::ifstream file("config/config.json");

    if (!file.is_open()) {
        throw std::runtime_error("Cannot open config file");
    }

    json config;
    file >> config;

    if (!config.contains("metrics") || !config["metrics"].is_array()) {
        throw std::runtime_error("Invalid config: metrics must be an array");
    }

    std::vector<MetricConfig> res{};

    for (const auto& metric : config["metrics"]) {
        if (!metric.contains("target") || !metric["target"].is_string()) {
            throw std::runtime_error("Invalid metric target");
        }

        if (!metric.contains("condition") || !metric["condition"].is_string()) {
            throw std::runtime_error("Invalid metric condition");
        }

        const std::string target{metric["target"]}, condition{metric["condition"]};

        res.push_back({target, parseCondition(condition)});
    }

    return res;
}

Threshold parseCondition(const std::string& condition) const {
    static const std::vector<std::string> operators { {"<=", ">=", "==", "<", ">"} };

    for (const auto& op : operators) {
        if (condition.starts_with(op)) {
            const std::string number = condition.substr(op.size());

            if (number.empty()) {
                throw std::runtime_error("Missing threshold value: " + condition);
            }

            try {
                size_t pos{};
                const double value = std::stod(number, &pos);

                if (pos != number.size()) {
                    throw std::runtime_error("Invalid threshold: " + condition);
                }

                return { op, value };
            }
            catch (const std::invalid_argument&) {
                throw std::runtime_error("Invalid threshold: " + condition);
            }
            catch (const std::out_of_range&) {
                throw std::runtime_error("Threshold out of range: " + condition);
            }
        }
    }

    throw std::runtime_error("Unknown comparison operator: " + condition);
}