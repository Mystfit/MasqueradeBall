#include "rendering/terrain_renderer.hpp"

void TerrainRenderer::draw(ftxui::Canvas& canvas,
                           const Camera& camera,
                           const std::vector<LevelSegment>& segments) {
    for (const auto& segment : segments) {
        // Only draw segments visible in viewport
        if (segment.end_x < camera.viewportLeft() ||
            segment.start_x > camera.viewportRight()) {
            continue;
        }

        if (segment.is_goal) {
            drawGoal(canvas, camera, segment);
        } else {
            drawSegment(canvas, camera, segment);
        }
    }
}

void TerrainRenderer::drawSegment(ftxui::Canvas& canvas,
                                  const Camera& camera,
                                  const LevelSegment& segment) {
    // Draw the terrain curve with thicker lines
    for (size_t i = 0; i + 1 < segment.sampled_points.size(); ++i) {
        auto screen_a = camera.worldToScreen(segment.sampled_points[i]);
        auto screen_b = camera.worldToScreen(segment.sampled_points[i + 1]);

        // Draw the line
        canvas.DrawPointLine(screen_a.x, screen_a.y, screen_b.x, screen_b.y);
        // Draw slightly below for thickness
        canvas.DrawPointLine(screen_a.x, screen_a.y + 1, screen_b.x, screen_b.y + 1);
    }

    // Draw text characters along the terrain
    if (!segment.source_text.empty() && !segment.sampled_points.empty()) {
        // Distribute text across the segment
        float segment_width = segment.end_x - segment.start_x;
        float char_spacing = segment_width / segment.source_text.length();

        for (size_t i = 0; i < segment.source_text.length(); ++i) {
            float x = segment.start_x + i * char_spacing;

            // Find closest sampled point
            b2Vec2 closest = segment.sampled_points[0];
            float min_dist = fabsf(closest.x - x);
            for (const auto& pt : segment.sampled_points) {
                float dist = fabsf(pt.x - x);
                if (dist < min_dist) {
                    min_dist = dist;
                    closest = pt;
                }
            }

            auto screen_pos = camera.worldToScreen(closest);
            std::string ch(1, segment.source_text[i]);
            canvas.DrawText(screen_pos.x, screen_pos.y, ch);
        }
    }
}

void TerrainRenderer::drawGoal(ftxui::Canvas& canvas,
                               const Camera& camera,
                               const LevelSegment& segment) {
    // Draw terrain
    drawSegment(canvas, camera, segment);

    // Draw goal posts (two vertical lines)
    float goal_x = segment.end_x;
    float goal_height = 5.0f;

    auto bottom_left = camera.worldToScreen({goal_x - 0.5f, 0.0f});
    auto top_left = camera.worldToScreen({goal_x - 0.5f, goal_height});
    auto bottom_right = camera.worldToScreen({goal_x + 0.5f, 0.0f});
    auto top_right = camera.worldToScreen({goal_x + 0.5f, goal_height});

    canvas.DrawPointLine(bottom_left.x, bottom_left.y, top_left.x, top_left.y);
    canvas.DrawPointLine(bottom_right.x, bottom_right.y, top_right.x, top_right.y);
    canvas.DrawPointLine(top_left.x, top_left.y, top_right.x, top_right.y);
}
