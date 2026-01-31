#pragma once

#include <box2d/box2d.h>

class Camera {
public:
    Camera();

    // Update camera to track target position
    void update(b2Vec2 target_pos, float dt);

    // Set screen dimensions (in braille pixels)
    void setScreenSize(int width, int height);

    // Convert world coords to screen pixel coords
    struct ScreenPos {
        int x;
        int y;
    };
    ScreenPos worldToScreen(b2Vec2 world_pos, float parallax_factor = 1.0f) const;

    // Convert screen coords to world coords
    b2Vec2 screenToWorld(int screen_x, int screen_y) const;

    // Viewport bounds in world coordinates
    float viewportLeft() const;
    float viewportRight() const;
    float viewportTop() const;
    float viewportBottom() const;

    b2Vec2 focus() const { return focus_; }

private:
    b2Vec2 focus_ = {0, 0};
    int screen_width_ = 0;   // In braille pixels
    int screen_height_ = 0;
    float smoothing_ = 0.1f; // Camera lerp factor
    float pixels_per_meter_ = 30.0f; // From PhysicsWorld::PIXELS_PER_METER
};
