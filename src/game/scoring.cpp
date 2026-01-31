#include "game/scoring.hpp"

#include <algorithm>
#include <cmath>

void Scoring::update(float distance_delta, float current_speed, float dt) {
    // Update multiplier based on speed
    if (current_speed > SPEED_THRESHOLD) {
        float speed_ratio = current_speed / SPEED_THRESHOLD;
        speed_multiplier_ = std::min(speed_ratio, MAX_MULTIPLIER);
    } else {
        // Decay toward 1.0 when slow
        speed_multiplier_ += (1.0f - speed_multiplier_) * 0.1f;
    }

    // Accumulate score
    raw_score_ += distance_delta * speed_multiplier_;
}

void Scoring::reset() {
    raw_score_ = 0.0f;
    speed_multiplier_ = 1.0f;
}
