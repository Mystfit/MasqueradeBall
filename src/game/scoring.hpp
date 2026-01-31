#pragma once

class Scoring {
public:
    Scoring() = default;

    void update(float distance_delta, float current_speed, float dt);

    int score() const { return static_cast<int>(raw_score_); }
    float multiplier() const { return speed_multiplier_; }

    void reset();

private:
    float raw_score_ = 0.0f;
    float speed_multiplier_ = 1.0f;
    static constexpr float SPEED_THRESHOLD = 3.0f; // Speed for 1x multiplier base
    static constexpr float MAX_MULTIPLIER = 5.0f;
};
