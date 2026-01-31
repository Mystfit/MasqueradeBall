#include "physics/physics_world.hpp"

PhysicsWorld::PhysicsWorld() {
    b2WorldDef world_def = b2DefaultWorldDef();
    world_def.gravity = {0.0f, GRAVITY};
    world_id_ = b2CreateWorld(&world_def);
}

PhysicsWorld::~PhysicsWorld() {
    if (B2_IS_NON_NULL(world_id_)) {
        b2DestroyWorld(world_id_);
    }
}

void PhysicsWorld::step(float dt) {
    b2World_Step(world_id_, dt, SUB_STEPS);
}
