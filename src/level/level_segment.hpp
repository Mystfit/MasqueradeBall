#pragma once

#include <box2d/box2d.h>

#include <string>
#include <vector>

struct LevelSegment {
    std::string source_text;           // Original text for rendering
    std::vector<b2Vec2> spline_points; // Cubic spline control points
    std::vector<b2Vec2> sampled_points; // Densely sampled spline output
    float start_x = 0.0f;
    float end_x = 0.0f;
    float gap_after = 0.0f;            // Width of gap after this segment
    bool is_goal = false;              // Final segment with goal posts
};
