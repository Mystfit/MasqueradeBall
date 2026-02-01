#pragma once

#include <box2d/box2d.h>

#include <array>

class MaskBody {
public:
    MaskBody(b2WorldId world_id, b2BodyId core_id, float mask_half_width);
    ~MaskBody();

    b2Vec2 getPosition() const;

    static constexpr float MASK_OFFSET_X = 0.4f;
    static constexpr float MASK_OFFSET_Y = 0.2f;
    static constexpr float JOINT_HERTZ = 25.0f;
    static constexpr float JOINT_DAMPING = 1.0f;

private:
    b2BodyId body_id_;
    std::array<b2JointId, 2> joints_;
};
