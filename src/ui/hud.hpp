#pragma once

#include <ftxui/dom/elements.hpp>

class HUD {
public:
    HUD() = default;

    ftxui::Element render(int score, float multiplier);

private:
    std::string formatMultiplier(float mult);
};
