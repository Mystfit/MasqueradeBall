#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "rendering/mask_renderer.hpp"

#include <algorithm>
#include <cmath>

MaskRenderer::MaskRenderer(const std::string& image_path, float world_width)
    : world_width_(world_width) {

    int img_w, img_h, channels;
    unsigned char* data = stbi_load(image_path.c_str(), &img_w, &img_h, &channels, 4);
    if (!data) {
        return;
    }

    // Target size: world_width in meters * 30 pixels/meter = braille pixels wide
    // Now we work directly in braille pixel space (1x1 resolution)
    constexpr float pixels_per_meter = 30.0f;
    int target_braille_width = static_cast<int>(world_width * pixels_per_meter);
    if (target_braille_width < 1) target_braille_width = 1;

    // Maintain aspect ratio
    float aspect = static_cast<float>(img_h) / static_cast<float>(img_w);
    int target_braille_height = static_cast<int>(target_braille_width * aspect);
    if (target_braille_height < 1) target_braille_height = 1;

    braille_width_ = target_braille_width;
    braille_height_ = target_braille_height;

    // Sample the source image at braille resolution using nearest-neighbor
    auto sample = [&](int bx, int by) -> std::array<unsigned char, 4> {
        // Map braille coords to source image coords
        int src_x = std::clamp(bx * img_w / target_braille_width, 0, img_w - 1);
        int src_y = std::clamp(by * img_h / target_braille_height, 0, img_h - 1);
        int idx = (src_y * img_w + src_x) * 4;
        return {data[idx], data[idx + 1], data[idx + 2], data[idx + 3]};
    };

    constexpr unsigned char alpha_threshold = 128;

    // Build braille pixel data - store only opaque pixels
    for (int by = 0; by < target_braille_height; ++by) {
        for (int bx = 0; bx < target_braille_width; ++bx) {
            auto rgba = sample(bx, by);
            if (rgba[3] >= alpha_threshold) {
                BraillePixel bp;
                bp.dx = bx;
                bp.dy = by;
                pixels_.push_back(bp);
            }
        }
    }

    stbi_image_free(data);
    loaded_ = true;
}

void MaskRenderer::draw(ftxui::Canvas& canvas, const Camera& camera, b2Vec2 mask_position) {
    if (!loaded_ || pixels_.empty()) {
        return;
    }

    auto screen_center = camera.worldToScreen(mask_position);

    // Offset to center the image on the mask position
    int origin_x = screen_center.x - braille_width_ / 2 - 10;
    int origin_y = screen_center.y - braille_height_ / 2 - 15;

    for (const auto& bp : pixels_) {
        int px = origin_x + bp.dx;
        int py = origin_y + bp.dy;

        // Bounds check
        if (px < 0 || py < 0) continue;

        canvas.DrawPoint(px, py, true, ftxui::Color::Orange1);
    }
}
