#pragma once

#include "rendering/camera.hpp"

#include <ftxui/dom/canvas.hpp>
#include <box2d/box2d.h>

#include <vector>

class BallRenderer {
public:
    BallRenderer() = default;

    // Draw ball as alternating filled/unfilled triangles from core to rim pairs
    void draw(ftxui::Canvas& canvas,
              const Camera& camera,
              b2Vec2 core_position,
              const std::vector<b2Vec2>& rim_positions);

    // Debug draw mode showing physics bodies
    void drawDebug(ftxui::Canvas& canvas,
                   const Camera& camera,
                   b2Vec2 core_position,
                   const std::vector<b2Vec2>& rim_positions,
                   float core_radius,
                   float rim_radius);

private:
    // Sort rim positions by angle from center for proper spline ordering
    std::vector<b2Vec2> sortByAngle(const std::vector<b2Vec2>& rim_positions);

    void drawCircle(ftxui::Canvas& canvas,
                    const Camera& camera,
                    b2Vec2 center,
                    float radius);

    // Scanline-fill a triangle in screen (braille) coordinates
    void fillTriangle(ftxui::Canvas& canvas,
                      int x0, int y0,
                      int x1, int y1,
                      int x2, int y2);
};
