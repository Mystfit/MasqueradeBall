#include "rendering/renderer.hpp"
#include "physics/softbody_ball.hpp"

#include <ftxui/dom/canvas.hpp>

Renderer::Renderer() = default;

ftxui::Element Renderer::render(const SoftbodyBall& ball,
                                 int screen_width,
                                 int screen_height) {
    using namespace ftxui;

    camera_.setScreenSize(screen_width, screen_height);

    return canvas(screen_width / 2, screen_height / 4, [&](Canvas& c) {
        // Draw the ball
        ball_renderer_.draw(c, camera_, ball.getCenterPosition(), ball.getRimPositions());
    });
}
