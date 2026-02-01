#include "rendering/ball_renderer.hpp"

#include <cmath>
#include <algorithm>

void BallRenderer::draw(ftxui::Canvas& canvas,
                        const Camera& camera,
                        b2Vec2 core_position,
                        const std::vector<b2Vec2>& rim_positions) {
    if (rim_positions.empty()) {
        return;
    }

    // Sort rim positions by angle from center
    std::vector<b2Vec2> sorted_rims = sortByAngle(rim_positions);

    // Convert core to screen coords
    auto core_screen = camera.worldToScreen(core_position);

    // Convert all rim positions to screen coords
    std::vector<Camera::ScreenPos> rim_screen;
    rim_screen.reserve(sorted_rims.size());
    for (const auto& rim : sorted_rims) {
        rim_screen.push_back(camera.worldToScreen(rim));
    }

    // Draw alternating filled / outline-only triangles
    for (size_t i = 0; i < rim_screen.size(); ++i) {
        size_t next = (i + 1) % rim_screen.size();

        int cx = core_screen.x, cy = core_screen.y;
        int ax = rim_screen[i].x, ay = rim_screen[i].y;
        int bx = rim_screen[next].x, by = rim_screen[next].y;

        if (i % 2 == 0) {
            // Filled triangle
            fillTriangle(canvas, cx, cy, ax, ay, bx, by);
        }

        // Always draw the outline edges so every triangle has visible borders
        canvas.DrawPointLine(cx, cy, ax, ay);
        canvas.DrawPointLine(ax, ay, bx, by);
        canvas.DrawPointLine(bx, by, cx, cy);
    }
}

void BallRenderer::drawDebug(ftxui::Canvas& canvas,
                             const Camera& camera,
                             b2Vec2 core_position,
                             const std::vector<b2Vec2>& rim_positions,
                             float core_radius,
                             float rim_radius) {
    if (rim_positions.empty()) {
        return;
    }

    // Draw core circle
    drawCircle(canvas, camera, core_position, core_radius);

    // Draw rim circles
    for (const auto& rim_pos : rim_positions) {
        drawCircle(canvas, camera, rim_pos, rim_radius);

        // Draw constraint line from core to rim (spokes)
        auto core_screen = camera.worldToScreen(core_position);
        auto rim_screen = camera.worldToScreen(rim_pos);
        canvas.DrawPointLine(core_screen.x, core_screen.y, rim_screen.x, rim_screen.y);
    }

    // Draw ring constraints between adjacent rims
    std::vector<b2Vec2> sorted_rims = sortByAngle(rim_positions);
    for (size_t i = 0; i < sorted_rims.size(); ++i) {
        size_t next = (i + 1) % sorted_rims.size();
        auto screen_a = camera.worldToScreen(sorted_rims[i]);
        auto screen_b = camera.worldToScreen(sorted_rims[next]);
        canvas.DrawPointLine(screen_a.x, screen_a.y, screen_b.x, screen_b.y);
    }
}

void BallRenderer::drawCircle(ftxui::Canvas& canvas,
                              const Camera& camera,
                              b2Vec2 center,
                              float radius) {
    // Draw circle as octagon (8 points)
    constexpr int num_points = 16;
    for (int i = 0; i < num_points; ++i) {
        float angle1 = (2.0f * M_PI * i) / num_points;
        float angle2 = (2.0f * M_PI * (i + 1)) / num_points;

        b2Vec2 p1 = {center.x + radius * cosf(angle1), center.y + radius * sinf(angle1)};
        b2Vec2 p2 = {center.x + radius * cosf(angle2), center.y + radius * sinf(angle2)};

        auto screen1 = camera.worldToScreen(p1);
        auto screen2 = camera.worldToScreen(p2);

        canvas.DrawPointLine(screen1.x, screen1.y, screen2.x, screen2.y);
    }
}

void BallRenderer::fillTriangle(ftxui::Canvas& canvas,
                                int x0, int y0,
                                int x1, int y1,
                                int x2, int y2) {
    // Sort vertices by Y coordinate (top to bottom in screen space)
    if (y0 > y1) { std::swap(x0, x1); std::swap(y0, y1); }
    if (y0 > y2) { std::swap(x0, x2); std::swap(y0, y2); }
    if (y1 > y2) { std::swap(x1, x2); std::swap(y1, y2); }

    // Degenerate triangle
    if (y0 == y2) return;

    // Scanline fill
    for (int y = y0; y <= y2; ++y) {
        // Compute x range for this scanline by interpolating along edges
        // Edge 0-2 always spans the full height
        float t_long = static_cast<float>(y - y0) / (y2 - y0);
        float x_long = x0 + t_long * (x2 - x0);

        float x_short;
        if (y < y1) {
            // Upper half: edge 0-1
            if (y1 == y0) {
                x_short = static_cast<float>(x1);
            } else {
                float t = static_cast<float>(y - y0) / (y1 - y0);
                x_short = x0 + t * (x1 - x0);
            }
        } else {
            // Lower half: edge 1-2
            if (y2 == y1) {
                x_short = static_cast<float>(x1);
            } else {
                float t = static_cast<float>(y - y1) / (y2 - y1);
                x_short = x1 + t * (x2 - x1);
            }
        }

        int xa = static_cast<int>(x_long);
        int xb = static_cast<int>(x_short);
        if (xa > xb) std::swap(xa, xb);

        canvas.DrawPointLine(xa, y, xb, y);
    }
}

std::vector<b2Vec2> BallRenderer::sortByAngle(const std::vector<b2Vec2>& rim_positions) {
    if (rim_positions.empty()) {
        return {};
    }

    // Find center
    b2Vec2 center = {0, 0};
    for (const auto& pos : rim_positions) {
        center.x += pos.x;
        center.y += pos.y;
    }
    center.x /= rim_positions.size();
    center.y /= rim_positions.size();

    // Sort by angle
    std::vector<b2Vec2> sorted = rim_positions;
    std::sort(sorted.begin(), sorted.end(), [&center](const b2Vec2& a, const b2Vec2& b) {
        float angle_a = atan2f(a.y - center.y, a.x - center.x);
        float angle_b = atan2f(b.y - center.y, b.x - center.x);
        return angle_a < angle_b;
    });

    return sorted;
}
