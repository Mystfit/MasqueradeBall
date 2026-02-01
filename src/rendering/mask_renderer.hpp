#pragma once

#include "rendering/camera.hpp"

#include <ftxui/dom/canvas.hpp>
#include <box2d/box2d.h>

#include <string>
#include <vector>

class MaskRenderer {
public:
    explicit MaskRenderer(const std::string& image_path, float world_width = 1.2f);

    void draw(ftxui::Canvas& canvas, const Camera& camera, b2Vec2 mask_position);

    bool isLoaded() const { return loaded_; }

    // Target world width in meters (used to compute mask_half_width for physics)
    float worldWidth() const { return world_width_; }

private:
    struct BraillePixel {
        int dx; // braille pixel offset from center
        int dy;
    };

    std::vector<BraillePixel> pixels_;
    int braille_width_ = 0;  // image width in braille pixels
    int braille_height_ = 0; // image height in braille pixels
    float world_width_ = 0.6f;
    bool loaded_ = false;
};
