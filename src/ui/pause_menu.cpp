#include "ui/pause_menu.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

PauseMenu::PauseMenu(std::function<void(GameState)> on_transition,
                     std::function<void()> on_restart)
    : on_transition_(std::move(on_transition)),
      on_restart_(std::move(on_restart)),
      entries_({"Resume", "Restart", "Main Menu"}) {

    using namespace ftxui;

    auto option = MenuOption::Vertical();
    option.on_enter = [this] {
        if (selected_ == 0) {
            on_transition_(GameState::Playing);
        } else if (selected_ == 1) {
            on_restart_();
            on_transition_(GameState::Playing);
        } else if (selected_ == 2) {
            on_transition_(GameState::StartMenu);
        }
    };
    option.entries_option.transform = [](const EntryState& state) {
        auto label = text(state.label);
        if (state.focused) {
            label = label | bold | inverted;
        }
        return label | center;
    };

    menu_component_ = Menu(&entries_, &selected_, option);
}

ftxui::Component PauseMenu::component() {
    return menu_component_;
}

ftxui::Element PauseMenu::render() {
    using namespace ftxui;

    return vbox({
        text("PAUSED") | bold | center,
        text(""),
        separator(),
        text(""),
        menu_component_->Render(),
        text(""),
        text("Press Escape to resume") | dim | center,
    }) | border | size(WIDTH, EQUAL, 40);
}
