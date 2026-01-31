#include "ui/start_menu.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

StartMenu::StartMenu(std::function<void(GameState)> on_transition)
    : on_transition_(std::move(on_transition)),
      entries_({"Start", "Options"}) {

    using namespace ftxui;

    auto option = MenuOption::Vertical();
    option.on_enter = [this] {
        if (selected_ == 0) {
            on_transition_(GameState::Playing);
        } else if (selected_ == 1) {
            on_transition_(GameState::OptionsMenu);
        }
    };
    option.entries_option.transform = [](const EntryState& state) {
        auto label = text(state.label);
        if (state.focused) {
            label = label | bold | inverted;
        }
        return label | center;
    };

    auto menu = Menu(&entries_, &selected_, option);

    menu_component_ = Renderer(menu, [this, menu] {
        return vbox({
                   filler(),
                   text("Masquerade Ball") | bold | center,
                   text("") | center,
                   separator(),
                   text("") | center,
                   menu->Render() | center | size(WIDTH, EQUAL, 20),
                   filler(),
               }) |
               border | flex;
    });
}

ftxui::Component StartMenu::component() {
    return menu_component_;
}
