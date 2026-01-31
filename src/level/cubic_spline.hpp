#pragma once

#include <box2d/box2d.h>

#include <vector>

class CubicSpline {
public:
    // Interpolate a natural cubic spline through control points
    // Returns densely sampled points along the spline
    static std::vector<b2Vec2> interpolate(
        const std::vector<b2Vec2>& control_points,
        float sample_interval = 0.25f);

    // Interpolate a closed cubic spline (for ball outline)
    static std::vector<b2Vec2> interpolateClosed(
        const std::vector<b2Vec2>& control_points,
        float sample_interval = 0.1f);
};
