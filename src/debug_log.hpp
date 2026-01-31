#pragma once

#include <fstream>
#include <sstream>
#include <mutex>
#include <chrono>
#include <iomanip>

class DebugLog {
public:
    static DebugLog& instance() {
        static DebugLog log;
        return log;
    }

    template<typename... Args>
    void write(Args&&... args) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!file_.is_open()) return;

        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        file_ << std::put_time(std::localtime(&time), "%H:%M:%S")
              << "." << std::setfill('0') << std::setw(3) << ms.count() << " ";

        ((file_ << std::forward<Args>(args)), ...);
        file_ << std::endl;
    }

private:
    DebugLog() {
        file_.open("/tmp/masquerade_debug.log", std::ios::out | std::ios::trunc);
        if (file_.is_open()) {
            write("=== Debug log started ===");
        }
    }

    ~DebugLog() {
        if (file_.is_open()) {
            write("=== Debug log ended ===");
            file_.close();
        }
    }

    std::ofstream file_;
    std::mutex mutex_;
};

#define DEBUG_LOG(...) DebugLog::instance().write(__VA_ARGS__)
