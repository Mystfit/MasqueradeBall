#pragma once

#include <string>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <optional>

class StdinReader {
public:
    StdinReader();
    ~StdinReader();

    void start();  // Launch reader thread
    void stop();

    // Thread-safe access
    std::optional<std::string> popNextLine();
    bool isEof() const { return eof_.load(); }
    bool hasPipeInput() const { return is_pipe_; }

    // Reset read position to beginning (for restart)
    void reset();

private:
    std::thread reader_thread_;
    std::mutex mutex_;
    std::queue<std::string> line_buffer_;
    std::vector<std::string> line_cache_;  // Cache all lines for replay on restart
    size_t read_position_ = 0;             // Current position in cache
    std::atomic<bool> eof_{false};
    std::atomic<bool> running_{false};
    bool is_pipe_ = false;

    void readerLoop();
};
