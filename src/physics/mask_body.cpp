#include "physics/mask_body.hpp"

#include <cmath>

MaskBody::MaskBody(b2WorldId world_id, b2BodyId core_id, float mask_half_width) {
    b2Vec2 core_pos = b2Body_GetPosition(core_id);

    // Create mask body offset to the right and slightly above the core
    b2BodyDef body_def = b2DefaultBodyDef();
    body_def.type = b2_dynamicBody;
    body_def.position = {core_pos.x + MASK_OFFSET_X, core_pos.y + MASK_OFFSET_Y};
    body_def.fixedRotation = true;
    body_def.linearDamping = 1.0f;
    body_def.gravityScale = 0.3f;
    body_id_ = b2CreateBody(world_id, &body_def);

    // No collision shape — add a tiny sensor circle just so Box2D has a mass
    b2ShapeDef shape_def = b2DefaultShapeDef();
    shape_def.density = 0.1f;
    shape_def.isSensor = true;
    b2Circle sensor = {{0, 0}, 0.05f};
    b2CreateCircleShape(body_id_, &shape_def, &sensor);

    // Compute joint length from core center to mask offset position
    float dx = MASK_OFFSET_X;
    float dy = MASK_OFFSET_Y;
    float base_length = sqrtf(dx * dx + dy * dy);

    // Left joint: core (0,0) → mask (-mask_half_width, 0)
    {
        b2DistanceJointDef jd = b2DefaultDistanceJointDef();
        jd.bodyIdA = core_id;
        jd.bodyIdB = body_id_;
        jd.localAnchorA = {0, 0};
        jd.localAnchorB = {-mask_half_width, 0};
        jd.length = sqrtf((dx - mask_half_width) * (dx - mask_half_width) + dy * dy);
        jd.enableSpring = true;
        jd.hertz = JOINT_HERTZ;
        jd.dampingRatio = JOINT_DAMPING;
        jd.collideConnected = false;
        joints_[0] = b2CreateDistanceJoint(world_id, &jd);
    }

    // Right joint: core (0,0) → mask (+mask_half_width, 0)
    {
        b2DistanceJointDef jd = b2DefaultDistanceJointDef();
        jd.bodyIdA = core_id;
        jd.bodyIdB = body_id_;
        jd.localAnchorA = {0, 0};
        jd.localAnchorB = {mask_half_width, 0};
        jd.length = sqrtf((dx + mask_half_width) * (dx + mask_half_width) + dy * dy);
        jd.enableSpring = true;
        jd.hertz = JOINT_HERTZ;
        jd.dampingRatio = JOINT_DAMPING;
        jd.collideConnected = false;
        joints_[1] = b2CreateDistanceJoint(world_id, &jd);
    }
}

MaskBody::~MaskBody() {
    for (auto joint : joints_) {
        if (B2_IS_NON_NULL(joint)) {
            b2DestroyJoint(joint);
        }
    }
    if (B2_IS_NON_NULL(body_id_)) {
        b2DestroyBody(body_id_);
    }
}

b2Vec2 MaskBody::getPosition() const {
    return b2Body_GetPosition(body_id_);
}
