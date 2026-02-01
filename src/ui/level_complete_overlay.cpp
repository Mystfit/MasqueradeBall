#include "ui/level_complete_overlay.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

LevelCompleteOverlay::LevelCompleteOverlay(std::function<void(GameState)> on_transition)
    : on_transition_(std::move(on_transition)),
      entries_({"Main Menu"}) {

    using namespace ftxui;

    auto option = MenuOption::Vertical();
    option.on_enter = [this] {
        if (selected_ == 0) {
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

ftxui::Component LevelCompleteOverlay::component() {
    return menu_component_;
}

ftxui::Element LevelCompleteOverlay::render(int final_score) {
    using namespace ftxui;

    return vbox({
        text("LEVEL COMPLETE!") | bold | center,
        text(""),
        text("Final Score: " + std::to_string(final_score)) | center,
        text(""),
        separator(),
        text(""),
        menu_component_->Render(),
        text(""),
        text("Press Escape or Enter") | dim | center,
    }) | border | size(WIDTH, EQUAL, 40);
}
