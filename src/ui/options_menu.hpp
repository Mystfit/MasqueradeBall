#pragma once

#include "game_state.hpp"

#include <ftxui/component/component.hpp>

#include <functional>

class OptionsMenu {
public:
    explicit OptionsMenu(std::function<void(GameState)> on_transition);

    ftxui::Component component();

private:
    std::function<void(GameState)> on_transition_;
    std::vector<std::string> entries_;
    int selected_ = 0;
    ftxui::Component menu_component_;
};
