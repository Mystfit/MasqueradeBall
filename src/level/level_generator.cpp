#include "level/level_generator.hpp"
#include "level/cubic_spline.hpp"
#include "debug_log.hpp"

#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>

LevelGenerator::LevelGenerator(StdinReader& reader)
    : reader_(reader),
      rng_(std::random_device{}()),
      perlin_(std::random_device{}()),
      macro_perlin_(std::random_device{}()) {}

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

    // Use the line directly as display text (no word splitting)
    segment.display_text = line.empty() ? " " : line;

    // Each display character = 0.15 world units (matches text bar character width)
    constexpr float chars_to_world = 0.15f;
    constexpr int sample_interval = 5; // Sample every 5th character

    float segment_width = static_cast<float>(segment.display_text.length()) * chars_to_world;
    segment.end_x = current_x_ + segment_width;

    // Initialize state for this segment
    if (segments_generated_ == 0) {
        current_y_ = 0.0f;
        smoothed_y_ = 0.0f;
        DEBUG_LOG("  [FIRST SEGMENT] Initializing: current_y=0, smoothed_y=0");
    } else {
        current_y_ = last_segment_end_y_;
        smoothed_y_ += Y_SMOOTHING_FACTOR * (last_segment_end_y_ - smoothed_y_);
        DEBUG_LOG("  [CONTINUATION] Pinning to previous: current_y=", std::fixed, std::setprecision(3),
                  current_y_, " (from last_segment_end_y_)");
    }

    // Generate control points by sampling every Nth character
    // Calculate frequency based on total line length for smoother terrain
    float line_length = static_cast<float>(segment.display_text.length());
    float frequency = 1.5f / std::max(1.0f, line_length / 10.0f);
    float macro_frequency = MACRO_BASE_FREQUENCY / std::max(1.0f, line_length / 10.0f);

    for (size_t i = 0; i < segment.display_text.length(); ++i) {
        bool is_first = (i == 0);
        bool is_last = (i == segment.display_text.length() - 1);
        bool is_sample_point = (i % sample_interval == 0);

        if (is_first || is_last || is_sample_point) {
            // For the last point, extend to the end of the character (not just its start)
            float x = segment.start_x + (is_last ? line_length : i) * chars_to_world;

            // For the very first point of the very first segment, pin to Y=0
            // For the first point of subsequent segments, pin to previous segment's end Y
            float y;
            if (i == 0) {
                if (segments_generated_ == 0) {
                    y = 0.0f;
                } else {
                    y = last_segment_end_y_;
                }
            } else {
                // Micro noise: per-character terrain detail
                float micro_noise = perlin_.octaveNoise(x, frequency, 2, 0.5f);

                // Macro noise: broad hills and valleys
                float macro_noise = macro_perlin_.octaveNoise(x, macro_frequency, 2, 0.5f);

                // Downward slope: gentle monotonic descent
                float slope_offset = -DOWNWARD_SLOPE * x;

                // Combine all layers on top of the smoothed baseline
                y = smoothed_y_
                    + micro_noise * 0.8f
                    + macro_noise * MACRO_AMPLITUDE
                    + slope_offset;

                // Constrain Y to prevent steep jumps between consecutive points
                y = std::clamp(y, current_y_ - MAX_HEIGHT_VARIATION, current_y_ + MAX_HEIGHT_VARIATION);
            }

            segment.spline_points.push_back({x, y});
            current_y_ = y;
            smoothed_y_ += Y_SMOOTHING_FACTOR * (y - smoothed_y_);
        }
    }

    // Ensure at least 2 control points for spline interpolation
    if (segment.spline_points.size() < 2) {
        segment.spline_points.clear();
        float y = (segments_generated_ > 0) ? last_segment_end_y_ : 0.0f;
        segment.spline_points.push_back({segment.start_x, y});
        segment.spline_points.push_back({segment.end_x, y});
    }

    // Sample the spline
    segment.sampled_points = CubicSpline::interpolate(segment.spline_points, 0.25f);

    // CRITICAL: Reverse points for correct chain winding (right-to-left)
    // Box2D chains need CCW winding for upward-facing normals
    std::reverse(segment.sampled_points.begin(), segment.sampled_points.end());

    // Record the segment endpoint X and Y for next segment continuity
    // (after reversal, back() is the leftmost point, front() is the rightmost)
    if (!segment.sampled_points.empty()) {
        last_segment_end_x_ = segment.sampled_points.front().x;
        last_segment_end_y_ = segment.sampled_points.front().y;
    }

    // Debug logging for segment boundaries
    if (!segment.sampled_points.empty()) {
        DEBUG_LOG("Segment #", segments_generated_, " '", segment.display_text.substr(0, 20),
                  (segment.display_text.length() > 20 ? "..." : ""), "'");
        DEBUG_LOG("  start_x=", std::fixed, std::setprecision(3), segment.start_x,
                  " end_x=", segment.end_x);
        DEBUG_LOG("  First sampled point (rightmost): (",
                  segment.sampled_points.front().x, ", ",
                  segment.sampled_points.front().y, ")");
        DEBUG_LOG("  Last sampled point (leftmost): (",
                  segment.sampled_points.back().x, ", ",
                  segment.sampled_points.back().y, ")");
        DEBUG_LOG("  Total sampled points: ", segment.sampled_points.size());
        DEBUG_LOG("  Captured endpoint: (", last_segment_end_x_, ", ", last_segment_end_y_, ")");
        DEBUG_LOG("  macro_freq=", std::setprecision(5), macro_frequency,
                  " slope_at_end=", std::setprecision(3), -DOWNWARD_SLOPE * segment.end_x);
    }

    // No gap between segments — continuous terrain
    segment.gap_after = 0.0f;
    // Use the actual sampled endpoint X, not the calculated segment.end_x
    current_x_ = last_segment_end_x_;

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

    // Flat segment at the height where previous terrain ended
    segment.spline_points = {
        {segment.start_x, last_segment_end_y_},
        {segment.end_x, last_segment_end_y_}
    };
    segment.sampled_points = CubicSpline::interpolate(segment.spline_points, 0.25f);

    // Reverse for correct chain winding
    std::reverse(segment.sampled_points.begin(), segment.sampled_points.end());

    return segment;
}
