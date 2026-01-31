#pragma once

#include "level/level_segment.hpp"
#include "level/stdin_reader.hpp"

#include <optional>
#include <random>

class LevelGenerator {
public:
    explicit LevelGenerator(StdinReader& reader);

    // Generate the next segment; returns nullopt if no data ready
    std::optional<LevelSegment> generateNext();

    bool isLevelComplete() const;

    float currentX() const { return current_x_; }

private:
    StdinReader& reader_;
    float current_x_ = 0.0f;
    float difficulty_ = 1.0f;
    int segments_generated_ = 0;
    std::mt19937 rng_;
    bool level_complete_ = false;

    LevelSegment generateSegmentFromText(const std::string& line);
    LevelSegment generateGoalSegment();
};
