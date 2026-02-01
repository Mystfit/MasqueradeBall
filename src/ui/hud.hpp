#pragma once

#include "input/input_action.hpp"

#include <ftxui/dom/elements.hpp>

class HUD {
public:
    HUD() = default;

    ftxui::Element render(int score, float multiplier,
                          bool debug_enabled, const InputSnapshot& input);

private:
    std::string formatMultiplier(float mult);
    ftxui::Element renderDebugInput(const InputSnapshot& input);
};
