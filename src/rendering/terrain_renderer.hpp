#pragma once

#include "rendering/camera.hpp"
#include "level/level_segment.hpp"

#include <ftxui/dom/canvas.hpp>

#include <vector>

class TerrainRenderer {
public:
    TerrainRenderer() = default;

    // Draw terrain segments
    void draw(ftxui::Canvas& canvas,
              const Camera& camera,
              const std::vector<LevelSegment>& segments);

private:
    void drawSegment(ftxui::Canvas& canvas,
                     const Camera& camera,
                     const LevelSegment& segment);

    void drawGoal(ftxui::Canvas& canvas,
                  const Camera& camera,
                  const LevelSegment& segment);
};
