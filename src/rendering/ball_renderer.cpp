#include "rendering/ball_renderer.hpp"
#include "level/cubic_spline.hpp"

#include <cmath>
#include <algorithm>

void BallRenderer::draw(ftxui::Canvas& canvas,
                        const Camera& camera,
                        const std::vector<b2Vec2>& rim_positions) {
    if (rim_positions.empty()) {
        return;
    }

    // Sort rim positions by angle from center
    std::vector<b2Vec2> sorted_rims = sortByAngle(rim_positions);

    // Interpolate closed spline through rim positions
    std::vector<b2Vec2> spline_points = CubicSpline::interpolateClosed(sorted_rims, 0.05f);

    // Draw the spline using braille mode
    for (size_t i = 0; i < spline_points.size(); ++i) {
        size_t next = (i + 1) % spline_points.size();

        auto screen_a = camera.worldToScreen(spline_points[i]);
        auto screen_b = camera.worldToScreen(spline_points[next]);

        canvas.DrawPointLine(screen_a.x, screen_a.y, screen_b.x, screen_b.y);
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
        //canvas.DrawPointLine(core_screen.x, core_screen.y, rim_screen.x, rim_screen.y);
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
