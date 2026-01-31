#include "rendering/camera.hpp"

#include <cmath>

Camera::Camera() = default;

void Camera::update(b2Vec2 target_pos, float dt) {
    // Keep ball centered on screen (reduce smoothing for tighter tracking)
    focus_.x = target_pos.x;  // No smoothing on X - keep ball centered horizontally
    focus_.y += (target_pos.y - focus_.y) * 0.3f;  // Slight smoothing on Y for stability
}

void Camera::setScreenSize(int width, int height) {
    screen_width_ = width;
    screen_height_ = height;
}

Camera::ScreenPos Camera::worldToScreen(b2Vec2 world_pos, float parallax_factor) const {
    // Apply parallax offset
    b2Vec2 parallax_focus = {
        focus_.x * parallax_factor,
        focus_.y * parallax_factor
    };

    // Camera center is at screen center
    float world_x_rel = world_pos.x - parallax_focus.x;
    float world_y_rel = world_pos.y - parallax_focus.y;

    // Convert to screen pixels (Y-down)
    int screen_x = static_cast<int>(screen_width_ / 2 + world_x_rel * pixels_per_meter_);
    int screen_y = static_cast<int>(screen_height_ / 2 - world_y_rel * pixels_per_meter_);

    return {screen_x, screen_y};
}

b2Vec2 Camera::screenToWorld(int screen_x, int screen_y) const {
    float world_x = focus_.x + (screen_x - screen_width_ / 2) / pixels_per_meter_;
    float world_y = focus_.y - (screen_y - screen_height_ / 2) / pixels_per_meter_;
    return {world_x, world_y};
}

float Camera::viewportLeft() const {
    return focus_.x - (screen_width_ / 2) / pixels_per_meter_;
}

float Camera::viewportRight() const {
    return focus_.x + (screen_width_ / 2) / pixels_per_meter_;
}

float Camera::viewportTop() const {
    return focus_.y + (screen_height_ / 2) / pixels_per_meter_;
}

float Camera::viewportBottom() const {
    return focus_.y - (screen_height_ / 2) / pixels_per_meter_;
}
