#pragma once

#include <box2d/box2d.h>

#include <array>
#include <vector>

class SoftbodyBall {
public:
    SoftbodyBall(b2WorldId world_id, b2Vec2 start_pos);
    ~SoftbodyBall();

    // Movement and jump control
    void applyMovement(float direction); // -1.0 = left, 1.0 = right
    void applyJumpImpulse(float magnitude);
    void startCompression(); // Begin held jump
    void updateCompression(float dt); // Continue compression while held
    void releaseJump(float input_direction = 0.0f); // Release after compression with directional control

    // State queries
    b2Vec2 getCenterPosition() const;
    std::vector<b2Vec2> getRimPositions() const;
    float getSpeed() const;
    bool isOnGround() const;

    b2BodyId getCoreBodyId() const { return core_id_; }

    // Constants - tuned for testing
    static constexpr int RIM_COUNT = 12;
    static constexpr float BALL_RADIUS = 0.5f;
    static constexpr float RIM_CIRCLE_RADIUS = 0.15f;  // Larger for robust collision
    static constexpr float CORE_RADIUS = 0.15f;        // Small core — rim handles terrain contact
    static constexpr float SPRING_HERTZ = 12.0f;       // Stiffer springs resist deformation
    static constexpr float SPRING_DAMPING = 0.9f;
    static constexpr float FORCE_MAGNITUDE = 8.0f;
    static constexpr float ROLL_TORQUE = 7.5f;

private:
    b2WorldId world_id_;
    b2BodyId core_id_;
    std::array<b2BodyId, RIM_COUNT> rim_ids_;
    std::vector<b2JointId> rim_joints_;   // Ring connections
    std::vector<b2JointId> spoke_joints_; // Rim-to-core connections

    // Jump compression state
    bool compressing_ = false;
    float compression_time_ = 0.0f;
    float original_spoke_length_;
    static constexpr float MAX_COMPRESSION_TIME = 0.5f;
    static constexpr float COMPRESSION_RATE = 0.3f; // How much to shorten spokes
};
