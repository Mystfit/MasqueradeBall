#pragma once

#include <box2d/box2d.h>

class PhysicsWorld {
public:
    PhysicsWorld();
    ~PhysicsWorld();

    void step(float dt);

    b2WorldId worldId() const { return world_id_; }

    // Constants
    static constexpr float GRAVITY = -20.0f;
    static constexpr int SUB_STEPS = 4;
    static constexpr float PIXELS_PER_METER = 30.0f; // Braille pixels per meter

private:
    b2WorldId world_id_;
};
