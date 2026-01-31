#pragma once

#include "level/level_segment.hpp"

#include <ftxui/dom/elements.hpp>

#include <vector>

class TextBar {
public:
    TextBar() = default;

    ftxui::Element render(const std::vector<LevelSegment>& segments,
                          float viewport_left,
                          float viewport_right,
                          int screen_width_chars,
                          int screen_width_braille_pixels);
};
