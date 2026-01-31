#pragma once

#include <box2d/box2d.h>

#include <vector>

class TerrainBody {
public:
    explicit TerrainBody(b2WorldId world_id);
    ~TerrainBody();

    // Add a terrain segment as a chain of edge shapes
    void addSegment(const std::vector<b2Vec2>& points);

    // Remove all terrain segments (for restart)
    void clear();

private:
    b2WorldId world_id_;
    std::vector<b2BodyId> segment_bodies_;
};
