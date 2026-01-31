#pragma once

#include "game/scoring.hpp"
#include "physics/physics_world.hpp"
#include "physics/softbody_ball.hpp"
#include "physics/terrain_body.hpp"
#include "level/level_generator.hpp"
#include "level/level_segment.hpp"
#include "input/input_action.hpp"

#include <memory>
#include <vector>

class GameSession {
public:
    GameSession(StdinReader& stdin_reader);

    void update(float dt, const InputSnapshot& input);

    // Accessors for rendering
    const SoftbodyBall& ball() const { return *ball_; }
    const std::vector<LevelSegment>& segments() const { return segments_; }
    int score() const { return scoring_.score(); }
    float speedMultiplier() const { return scoring_.multiplier(); }
    bool isGameOver() const { return game_over_; }
    bool isLevelComplete() const { return level_complete_; }

    void restart();

    PhysicsWorld& physics() { return physics_; }

private:
    PhysicsWorld physics_;
    std::unique_ptr<SoftbodyBall> ball_;
    TerrainBody terrain_;
    LevelGenerator level_gen_;
    Scoring scoring_;

    std::vector<LevelSegment> segments_;
    float elapsed_time_ = 0.0f;
    bool game_over_ = false;
    bool level_complete_ = false;
    float last_ball_x_ = 0.0f;

    // Jump state
    bool jump_held_ = false;

    void processInput(const InputSnapshot& input, float dt);
    void generateAheadOfCamera();
    void checkFallThroughGap();
    void checkGoalReached();
};
