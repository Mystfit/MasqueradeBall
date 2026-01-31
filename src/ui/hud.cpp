#include "ui/hud.hpp"

#include <ftxui/dom/elements.hpp>

#include <sstream>
#include <iomanip>

ftxui::Element HUD::render(int score, float multiplier) {
    using namespace ftxui;

    return vbox({
        hbox({
            text("Score: " + std::to_string(score)) | bold,
            filler(),
            text("x" + formatMultiplier(multiplier)),
        }) | size(HEIGHT, EQUAL, 1),
    });
}

std::string HUD::formatMultiplier(float mult) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << mult;
    return oss.str();
}
