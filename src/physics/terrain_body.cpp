#include "physics/terrain_body.hpp"

TerrainBody::TerrainBody(b2WorldId world_id)
    : world_id_(world_id) {}

TerrainBody::~TerrainBody() {
    clear();
}

void TerrainBody::addSegment(const std::vector<b2Vec2>& points) {
    if (points.size() < 2) {
        return;
    }

    // Create a static body for this segment
    b2BodyDef body_def = b2DefaultBodyDef();
    body_def.type = b2_staticBody;
    b2BodyId body_id = b2CreateBody(world_id_, &body_def);

    // Create chain shape from points
    b2ChainDef chain_def = b2DefaultChainDef();
    chain_def.points = points.data();
    chain_def.count = static_cast<int>(points.size());
    chain_def.isLoop = false;

    // Set material properties for the chain
    b2SurfaceMaterial material = {1.0f, 0.1f, 0.0f}; // friction, restitution, rollingResistance
    chain_def.materials = &material;
    chain_def.materialCount = 1;

    b2CreateChain(body_id, &chain_def);

    segment_bodies_.push_back(body_id);
}

void TerrainBody::clear() {
    for (auto body_id : segment_bodies_) {
        if (B2_IS_NON_NULL(body_id)) {
            b2DestroyBody(body_id);
        }
    }
    segment_bodies_.clear();
}
