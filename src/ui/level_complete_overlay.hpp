#pragma once

#include "game_state.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include <functional>
#include <string>

class LevelCompleteOverlay {
public:
    explicit LevelCompleteOverlay(std::function<void(GameState)> on_transition);

    // Returns the menu component for event handling
    ftxui::Component component();

    // Renders the level complete overlay as a centered dialog
    // Pass the final score to display
    ftxui::Element render(int final_score);

private:
    std::function<void(GameState)> on_transition_;
    std::vector<std::string> entries_;
    int selected_ = 0;
    ftxui::Component menu_component_;
};
