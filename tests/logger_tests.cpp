#include "tools/logger.h"
#include <gtest/gtest.h>
#include <chrono>

TEST(Logger, NormalWork) {
    tools::Logger logger;

    const std::vector<agent::Metric> metrics_list {
        agent::Metric("name1", 3.4),
        agent::Metric("name2", 3.4),
    };

    logger.log(metrics_list);

    auto time = std::chrono::system_clock::now();
    std::time_t now_date = std::chrono::system_clock::to_time_t(time);

    std::stringstream filename;
    filename << std::put_time(std::localtime(&now_date), "%d.%m.%Y");

    std::string name = filename.str() + ".log";
    std::ifstream log_file(name);

    if (log_file.is_open()) {
        std::string line;
        int count = 0;
        while (std::getline(log_file, name)) {
            count++;
        }

        EXPECT_EQ(count, 1);
    }
    else {
        FAIL() << "The log file has not been created";
    }
}