#include "level/stdin_reader.hpp"

#include <iostream>
#include <fstream>
#include <unistd.h>

StdinReader::StdinReader() {
    // Detect if stdin is a pipe
    is_pipe_ = !isatty(STDIN_FILENO);
}

StdinReader::~StdinReader() {
    stop();
}

void StdinReader::start() {
    if (running_.load()) {
        return;
    }

    running_.store(true);
    reader_thread_ = std::thread(&StdinReader::readerLoop, this);
}

void StdinReader::stop() {
    running_.store(false);
    if (reader_thread_.joinable()) {
        reader_thread_.join();
    }
}

void StdinReader::readerLoop() {
    if (is_pipe_) {
        // Read from STDIN pipe
        std::string line;
        while (running_.load() && std::getline(std::cin, line)) {
            std::lock_guard<std::mutex> lock(mutex_);
            line_buffer_.push(line);
        }
    } else {
        // Load lorem ipsum fallback
        std::ifstream file("assets/lorem_ipsum.txt");
        if (file.is_open()) {
            std::string line;
            while (running_.load() && std::getline(file, line)) {
                std::lock_guard<std::mutex> lock(mutex_);
                line_buffer_.push(line);
            }
            file.close();
        }
    }

    eof_.store(true);
}

std::optional<std::string> StdinReader::popNextLine() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (line_buffer_.empty()) {
        return std::nullopt;
    }

    std::string line = line_buffer_.front();
    line_buffer_.pop();
    return line;
}
