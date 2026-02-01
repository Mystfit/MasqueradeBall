#pragma once

#include "game_state.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include <functional>

class PauseMenu {
public:
    explicit PauseMenu(std::function<void(GameState)> on_transition,
                       std::function<void()> on_restart);

    // Returns the menu component for event handling
    ftxui::Component component();

    // Renders the pause menu as an overlay (centered dialog)
    ftxui::Element render();

private:
    std::function<void(GameState)> on_transition_;
    std::function<void()> on_restart_;
    std::vector<std::string> entries_;
    int selected_ = 0;
    ftxui::Component menu_component_;
};
