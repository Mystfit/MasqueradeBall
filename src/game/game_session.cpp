#include "game/game_session.hpp"

GameSession::GameSession(StdinReader& stdin_reader)
    : terrain_(physics_.worldId()),
      level_gen_(stdin_reader) {

    // Create ball at starting position (above the terrain which starts at Y=0)
    b2Vec2 start_pos = {5.0f, 3.0f};
    ball_ = std::make_unique<SoftbodyBall>(physics_.worldId(), start_pos);
    last_ball_x_ = start_pos.x;

    // Generate initial terrain segments
    for (int i = 0; i < 5; ++i) {
        auto seg_opt = level_gen_.generateNext();
        if (seg_opt) {
            segments_.push_back(*seg_opt);
            terrain_.addSegment(seg_opt->sampled_points);
        }
    }
}

void GameSession::update(float dt, const InputSnapshot& input) {
    if (game_over_ || level_complete_) {
        return;
    }

    elapsed_time_ += dt;

    // Process input
    processInput(input, dt);

    // Step physics
    physics_.step(dt);

    // Update camera position based on ball
    b2Vec2 ball_pos = ball_->getCenterPosition();

    // Update scoring
    float distance_delta = ball_pos.x - last_ball_x_;
    if (distance_delta > 0.0f) {
        scoring_.update(distance_delta, ball_->getSpeed(), dt);
    }
    last_ball_x_ = ball_pos.x;

    // Generate terrain ahead
    generateAheadOfCamera();

    // Check game over / goal
    checkFallOffWorld();
    checkGoalReached();
}

void GameSession::processInput(const InputSnapshot& input, float dt) {
    // Horizontal movement
    if (input.move_left || input.horizontal_axis < -0.1f) {
        ball_->applyMovement(-1.0f);
    } else if (input.move_right || input.horizontal_axis > 0.1f) {
        ball_->applyMovement(1.0f);
    }

    // Jump mechanics
    if (input.jump_just_pressed) {
        ball_->startCompression();
        jump_held_ = true;
    }

    if (jump_held_ && input.jump_held) {
        ball_->updateCompression(dt);
    }

    if (input.jump_just_released && jump_held_) {
        ball_->releaseJump();
        jump_held_ = false;
    }
}

void GameSession::generateAheadOfCamera() {
    b2Vec2 ball_pos = ball_->getCenterPosition();
    float generation_horizon = ball_pos.x + 50.0f; // Generate 50 units ahead

    int generated_count = 0;
    while (level_gen_.currentX() < generation_horizon && !level_gen_.isLevelComplete()) {
        auto seg_opt = level_gen_.generateNext();
        if (seg_opt) {
            segments_.push_back(*seg_opt);
            terrain_.addSegment(seg_opt->sampled_points);
            generated_count++;
        } else {
            break;
        }
    }

}


void GameSession::checkFallOffWorld() {
    b2Vec2 ball_pos = ball_->getCenterPosition();

    // Safety net: if ball somehow falls far below terrain
    if (ball_pos.y < -5.0f) {
        game_over_ = true;
    }
}

void GameSession::checkGoalReached() {
    b2Vec2 ball_pos = ball_->getCenterPosition();

    // Check if ball crossed any goal segment
    for (const auto& seg : segments_) {
        if (seg.is_goal && ball_pos.x >= seg.end_x) {
            level_complete_ = true;
            return;
        }
    }
}

void GameSession::restart() {
    // Clear terrain
    terrain_.clear();
    segments_.clear();

    // Reset scoring
    scoring_.reset();

    // Recreate ball
    b2Vec2 start_pos = {5.0f, 3.0f};
    ball_ = std::make_unique<SoftbodyBall>(physics_.worldId(), start_pos);

    // Reset state
    game_over_ = false;
    level_complete_ = false;
    elapsed_time_ = 0.0f;
    last_ball_x_ = start_pos.x;
    jump_held_ = false;

    // Regenerate initial terrain
    for (int i = 0; i < 5; ++i) {
        auto seg_opt = level_gen_.generateNext();
        if (seg_opt) {
            segments_.push_back(*seg_opt);
            terrain_.addSegment(seg_opt->sampled_points);
        }
    }
}
