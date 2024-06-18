//
// Created by 缪浩楚 on 2024/6/18.
//

#ifndef FFM_LOGGER_H
#define FFM_LOGGER_H
#include <thread>
#include "fmt/core.h"
class Logger {
public:
    static const Logger& instance() {
        static Logger logger;
        return logger;
    };

    template<class... T>
    static void log(const char* fmt, T&&... args) {
        std::lock_guard<std::mutex> lock(instance().m_mutex);
        fmt::println(fmt, std::forward<T>(args)...);
    };
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
private:
    Logger() = default;
    ~Logger() = default;
     mutable std::mutex m_mutex;
};
#endif //FFM_LOGGER_H
