#include "ui/options_menu.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

OptionsMenu::OptionsMenu(std::function<void(GameState)> on_transition)
    : on_transition_(std::move(on_transition)),
      entries_({"Option 1", "Option 2", "Option 3", "Back"}) {

    using namespace ftxui;

    auto option = MenuOption::Vertical();
    option.on_enter = [this] {
        if (selected_ == static_cast<int>(entries_.size()) - 1) {
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

    auto menu = Menu(&entries_, &selected_, option);

    menu_component_ = Renderer(menu, [menu] {
        return vbox({
                   filler(),
                   text("Options") | bold | center,
                   text("") | center,
                   separator(),
                   text("") | center,
                   menu->Render() | center | size(WIDTH, EQUAL, 20),
                   filler(),
               }) |
               border | flex;
    });
}

ftxui::Component OptionsMenu::component() {
    return menu_component_;
}
