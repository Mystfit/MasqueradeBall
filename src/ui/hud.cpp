#include "ui/hud.hpp"

#include <ftxui/dom/elements.hpp>

#include <sstream>
#include <iomanip>

ftxui::Element HUD::render(int score, float multiplier,
                           bool debug_enabled, const InputSnapshot& input) {
    using namespace ftxui;

    auto hud_line = hbox({
        text("Score: " + std::to_string(score)) | bold,
        filler(),
        text("x" + formatMultiplier(multiplier)),
    }) | size(HEIGHT, EQUAL, 1);

    if (debug_enabled) {
        return vbox({
            hud_line,
            renderDebugInput(input),
        });
    }

    return vbox({ hud_line });
}

ftxui::Element HUD::renderDebugInput(const InputSnapshot& input) {
    using namespace ftxui;

    auto key = [](const std::string& label, bool active) -> Element {
        auto t = text(label);
        return active ? (t | bold | inverted) : (t | dim);
    };

    std::string jump_label = "JUMP";
    bool jump_active = false;
    if (input.jump_held) {
        jump_label = "JUMP[hold]";
        jump_active = true;
    } else if (input.jump_just_pressed) {
        jump_label = "JUMP[press]";
        jump_active = true;
    } else if (input.jump_just_released) {
        jump_label = "JUMP[rel]";
        jump_active = true;
    }

    return hbox({
        filler(),
        text("Input: ") | dim,
        key("LEFT", input.move_left),
        text(" "),
        key("RIGHT", input.move_right),
        text(" "),
        key("UP", input.move_up),
        text(" "),
        key("DOWN", input.move_down),
        text(" "),
        key(jump_label, jump_active),
    }) | size(HEIGHT, EQUAL, 1);
}

std::string HUD::formatMultiplier(float mult) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << mult;
    return oss.str();
}
