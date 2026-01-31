#include "ui/game_over_overlay.hpp"

#include <ftxui/dom/elements.hpp>

ftxui::Element GameOverOverlay::render() {
    using namespace ftxui;

    return dbox({
        filler(),
        vbox({
            text("GAME OVER") | bold | center,
            text("Press Enter to restart") | center | dim,
        }) | center | border | bgcolor(Color::Black),
    });
}
