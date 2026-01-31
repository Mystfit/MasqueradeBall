#include "level/level_generator.hpp"
#include "level/cubic_spline.hpp"

#include <cmath>
#include <algorithm>

LevelGenerator::LevelGenerator(StdinReader& reader)
    : reader_(reader),
      rng_(std::random_device{}()) {}

std::optional<LevelSegment> LevelGenerator::generateNext() {
    if (level_complete_) {
        return std::nullopt;
    }

    auto line_opt = reader_.popNextLine();
    if (!line_opt) {
        // Check if EOF reached
        if (reader_.isEof()) {
            level_complete_ = true;
            return generateGoalSegment();
        }
        return std::nullopt;
    }

    return generateSegmentFromText(*line_opt);
}

bool LevelGenerator::isLevelComplete() const {
    return level_complete_;
}

LevelSegment LevelGenerator::generateSegmentFromText(const std::string& line) {
    LevelSegment segment;
    segment.source_text = line;
    segment.start_x = current_x_;

    // Segment width based on text length (1 character ≈ 1 world unit)
    float segment_width = std::max(5.0f, static_cast<float>(line.length()) * 0.5f);
    segment.end_x = current_x_ + segment_width;

    // Generate control points with random Y variance
    int num_points = std::max(4, static_cast<int>(segment_width / 2.0f));
    float base_y = 0.0f;

    std::uniform_real_distribution<float> y_dist(-difficulty_, difficulty_);

    for (int i = 0; i < num_points; ++i) {
        float x = segment.start_x + (segment_width * i) / (num_points - 1);
        float y = base_y + y_dist(rng_);
        base_y = y; // Next point starts from previous height
        segment.spline_points.push_back({x, y});
    }

    // Sample the spline
    segment.sampled_points = CubicSpline::interpolate(segment.spline_points, 0.25f);

    // CRITICAL: Reverse points for correct chain winding (right-to-left)
    // Box2D chains need CCW winding for upward-facing normals
    std::reverse(segment.sampled_points.begin(), segment.sampled_points.end());

    // Random gap after this segment (newline = gap)
    std::uniform_real_distribution<float> gap_dist(2.0f, 4.0f);
    segment.gap_after = gap_dist(rng_);

    // Advance X position
    current_x_ = segment.end_x + segment.gap_after;

    // Increase difficulty
    difficulty_ += 0.1f;
    difficulty_ = std::min(difficulty_, 5.0f);

    segments_generated_++;

    return segment;
}

LevelSegment LevelGenerator::generateGoalSegment() {
    LevelSegment segment;
    segment.source_text = "GOAL";
    segment.start_x = current_x_;
    segment.end_x = current_x_ + 2.0f; // Small flat segment before goal
    segment.is_goal = true;
    segment.gap_after = 0.0f;

    // Flat segment
    segment.spline_points = {
        {segment.start_x, 0.0f},
        {segment.end_x, 0.0f}
    };
    segment.sampled_points = CubicSpline::interpolate(segment.spline_points, 0.25f);

    // Reverse for correct chain winding
    std::reverse(segment.sampled_points.begin(), segment.sampled_points.end());

    return segment;
}
