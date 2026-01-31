#pragma once

#include "level/level_segment.hpp"
#include "level/stdin_reader.hpp"
#include "level/perlin_noise.hpp"

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
    static constexpr float Y_SMOOTHING_FACTOR = 0.3f;  // Reduced to allow more variation

    StdinReader& reader_;
    float current_x_ = 0.0f;
    float current_y_ = 0.0f;           // Track last raw Y for per-point clamping
    float smoothed_y_ = 0.0f;          // EMA of Y baseline (prevents cumulative drift)
    float last_segment_end_x_ = 0.0f;  // Exact X of previous segment's rightmost sampled point
    float last_segment_end_y_ = 0.0f;  // Exact Y of previous segment's rightmost sampled point
    float difficulty_ = 0.1f;
    int segments_generated_ = 0;
    std::mt19937 rng_;
    PerlinNoise perlin_;
    bool level_complete_ = false;


    LevelSegment generateSegmentFromText(const std::string& line);
    LevelSegment generateGoalSegment();
};

