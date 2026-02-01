#include "physics/softbody_ball.hpp"

#include <cmath>
#include <algorithm>

SoftbodyBall::SoftbodyBall(b2WorldId world_id, b2Vec2 start_pos)
    : world_id_(world_id),
      original_spoke_length_(BALL_RADIUS) {

    // Create central core body
    b2BodyDef core_def = b2DefaultBodyDef();
    core_def.type = b2_dynamicBody;
    core_def.position = start_pos;
    core_def.linearDamping = 0.5f;
    core_def.angularDamping = 0.3f;
    core_id_ = b2CreateBody(world_id, &core_def);

    // Add core shape
    b2ShapeDef shape_def = b2DefaultShapeDef();
    shape_def.density = 2.0f;
    shape_def.material.friction = 0.6f;
    shape_def.material.restitution = 0.3f;
    b2Circle core_circle = {{0, 0}, CORE_RADIUS};
    b2CreateCircleShape(core_id_, &shape_def, &core_circle);

    // Create rim bodies in a circle
    for (int i = 0; i < RIM_COUNT; ++i) {
        float angle = (2.0f * M_PI * i) / RIM_COUNT;
        b2Vec2 rim_pos = {
            start_pos.x + BALL_RADIUS * cosf(angle),
            start_pos.y + BALL_RADIUS * sinf(angle)
        };

        b2BodyDef rim_def = b2DefaultBodyDef();
        rim_def.type = b2_dynamicBody;
        rim_def.position = rim_pos;
        rim_def.linearDamping = 0.3f;
        rim_def.isBullet = true; // CCD prevents tunneling through terrain
        rim_ids_[i] = b2CreateBody(world_id, &rim_def);

        // Add rim shape
        shape_def.density = 0.5f;
        shape_def.material.friction = 1.0f;
        b2Circle rim_circle = {{0, 0}, RIM_CIRCLE_RADIUS};
        b2CreateCircleShape(rim_ids_[i], &shape_def, &rim_circle);
    }

    // Connect adjacent rim bodies (ring)
    for (int i = 0; i < RIM_COUNT; ++i) {
        int next = (i + 1) % RIM_COUNT;

        b2Vec2 pos_a = b2Body_GetPosition(rim_ids_[i]);
        b2Vec2 pos_b = b2Body_GetPosition(rim_ids_[next]);
        b2Vec2 diff = {pos_b.x - pos_a.x, pos_b.y - pos_a.y};
        float length = sqrtf(diff.x * diff.x + diff.y * diff.y);

        b2DistanceJointDef jd = b2DefaultDistanceJointDef();
        jd.bodyIdA = rim_ids_[i];
        jd.bodyIdB = rim_ids_[next];
        jd.localAnchorA = {0, 0};
        jd.localAnchorB = {0, 0};
        jd.length = length;
        jd.enableSpring = true;
        jd.hertz = SPRING_HERTZ;
        jd.dampingRatio = SPRING_DAMPING;
        jd.enableLimit = true;
        jd.minLength = length * 0.5f;  // Prevent ring from collapsing
        jd.maxLength = length * 1.5f;  // Prevent excessive stretching
        jd.collideConnected = false;

        rim_joints_.push_back(b2CreateDistanceJoint(world_id, &jd));
    }

    // Connect each rim body to core (spokes)
    for (int i = 0; i < RIM_COUNT; ++i) {
        b2DistanceJointDef jd = b2DefaultDistanceJointDef();
        jd.bodyIdA = core_id_;
        jd.bodyIdB = rim_ids_[i];
        jd.localAnchorA = {0, 0};
        jd.localAnchorB = {0, 0};
        jd.length = BALL_RADIUS;
        jd.enableSpring = true;
        jd.hertz = SPRING_HERTZ;
        jd.dampingRatio = SPRING_DAMPING;
        jd.enableLimit = true;
        jd.minLength = BALL_RADIUS * 0.5f;  // Hard floor — prevents rim collapse through terrain
        jd.maxLength = BALL_RADIUS * 1.2f;  // Prevents excessive stretching
        jd.collideConnected = false;

        spoke_joints_.push_back(b2CreateDistanceJoint(world_id, &jd));
    }
}

SoftbodyBall::~SoftbodyBall() {
    // Destroy joints first
    for (auto joint : rim_joints_) {
        if (B2_IS_NON_NULL(joint)) {
            b2DestroyJoint(joint);
        }
    }
    for (auto joint : spoke_joints_) {
        if (B2_IS_NON_NULL(joint)) {
            b2DestroyJoint(joint);
        }
    }

    // Destroy bodies
    if (B2_IS_NON_NULL(core_id_)) {
        b2DestroyBody(core_id_);
    }
    for (auto rim_id : rim_ids_) {
        if (B2_IS_NON_NULL(rim_id)) {
            b2DestroyBody(rim_id);
        }
    }
}

void SoftbodyBall::applyMovement(float direction) {
    // Apply horizontal force to the core - friction will naturally cause rolling
    float force_magnitude = direction * FORCE_MAGNITUDE; // Increased for better responsiveness
    b2Vec2 force = {force_magnitude, 0.0f};
    b2Body_ApplyForceToCenter(core_id_, force, true);

    // Also apply a moderate torque to help initiate rolling
    float torque = direction * ROLL_TORQUE;
    b2Body_ApplyTorque(core_id_, torque, true);
}

void SoftbodyBall::applyJumpImpulse(float magnitude) {
    b2Vec2 impulse = {0.0f, magnitude};
    b2Body_ApplyLinearImpulseToCenter(core_id_, impulse, true);
}

void SoftbodyBall::startCompression() {
    compressing_ = true;
    compression_time_ = 0.0f;
}

void SoftbodyBall::updateCompression(float dt) {
    if (!compressing_) {
        return;
    }

    compression_time_ += dt;
    compression_time_ = std::min(compression_time_, MAX_COMPRESSION_TIME);

    // Shorten spoke joints to compress the ball
    float compression_factor = 1.0f - (compression_time_ / MAX_COMPRESSION_TIME) * COMPRESSION_RATE;
    float new_length = original_spoke_length_ * compression_factor;

    for (auto joint : spoke_joints_) {
        b2DistanceJoint_SetLength(joint, new_length);
    }

    // Slow horizontal movement during compression
    b2Vec2 velocity = b2Body_GetLinearVelocity(core_id_);
    velocity.x *= 0.95f; // Dampen horizontal speed
    b2Body_SetLinearVelocity(core_id_, velocity);
}

void SoftbodyBall::releaseJump(float input_direction) {
    if (!compressing_) {
        return;
    }

    // Restore spoke lengths
    for (auto joint : spoke_joints_) {
        b2DistanceJoint_SetLength(joint, original_spoke_length_);
    }

    // Only jump if on ground
    if (isOnGround()) {
        // Get current velocity to incorporate momentum
        b2Vec2 current_velocity = b2Body_GetLinearVelocity(core_id_);

        // Calculate jump impulse scaled by compression time
        float impulse_scale = 1.0f + (compression_time_ / MAX_COMPRESSION_TIME) * 1.15f;
        float vertical_impulse = 3.0f * impulse_scale;

        // Add directional component based on input and current velocity
        // Blend input direction with current horizontal momentum
        float horizontal_impulse = 0.0f;
        if (std::abs(input_direction) > 0.1f) {
            // Input direction contributes to jump direction
            horizontal_impulse = input_direction * 1.15f * impulse_scale;

            // Add a portion of current velocity to preserve momentum
            horizontal_impulse += current_velocity.x * 0.3f;
        } else {
            // No input, just preserve some current momentum
            horizontal_impulse = current_velocity.x * 0.2f;
        }

        // Apply combined directional impulse
        b2Vec2 impulse = {horizontal_impulse, vertical_impulse};
        b2Body_ApplyLinearImpulseToCenter(core_id_, impulse, true);
    }

    compressing_ = false;
    compression_time_ = 0.0f;
}

b2Vec2 SoftbodyBall::getCenterPosition() const {
    return b2Body_GetPosition(core_id_);
}

std::vector<b2Vec2> SoftbodyBall::getRimPositions() const {
    std::vector<b2Vec2> positions;
    positions.reserve(RIM_COUNT);
    for (const auto& rim_id : rim_ids_) {
        positions.push_back(b2Body_GetPosition(rim_id));
    }
    return positions;
}

float SoftbodyBall::getSpeed() const {
    b2Vec2 velocity = b2Body_GetLinearVelocity(core_id_);
    return sqrtf(velocity.x * velocity.x + velocity.y * velocity.y);
}

bool SoftbodyBall::isOnGround() const {
    // Check if any rim body is in contact with the world
    for (const auto& rim_id : rim_ids_) {
        int contact_capacity = b2Body_GetContactCapacity(rim_id);
        if (contact_capacity > 0) {
            std::vector<b2ContactData> contacts(contact_capacity);
            int contact_count = b2Body_GetContactData(rim_id, contacts.data(), contact_capacity);

            // If any rim body has active contacts, we're on the ground
            for (int i = 0; i < contact_count; ++i) {
                if (contacts[i].manifold.pointCount > 0) {
                    return true;
                }
            }
        }
    }

    return false;
}
