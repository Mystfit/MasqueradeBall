#pragma once

#include "rendering/camera.hpp"
#include "rendering/ball_renderer.hpp"

#include <ftxui/dom/elements.hpp>

#include <box2d/box2d.h>
#include <vector>

// Forward declarations
class SoftbodyBall;

class Renderer {
public:
    Renderer();

    // Render the game world to an FTXUI element
    ftxui::Element render(const SoftbodyBall& ball,
                          int screen_width,
                          int screen_height);

    Camera& camera() { return camera_; }

private:
    Camera camera_;
    BallRenderer ball_renderer_;
};
