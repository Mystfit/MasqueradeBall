#pragma once

#include "game_state.hpp"
#include "ui/start_menu.hpp"
#include "ui/options_menu.hpp"
#include "ui/hud.hpp"
#include "ui/game_over_overlay.hpp"
#include "input/input_manager.hpp"
#include "game/game_session.hpp"
#include "rendering/renderer.hpp"
#include "rendering/terrain_renderer.hpp"
#include "rendering/ball_renderer.hpp"
#include "level/stdin_reader.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include <memory>

class App {
public:
    explicit App(std::unique_ptr<StdinReader> stdin_reader);
    void run();

private:
    ftxui::ScreenInteractive screen_;
    GameState current_state_ = GameState::StartMenu;
    int tab_index_ = 0;

    std::unique_ptr<StdinReader> stdin_reader_;
    std::unique_ptr<StartMenu> start_menu_;
    std::unique_ptr<OptionsMenu> options_menu_;
    std::unique_ptr<InputManager> input_manager_;
    std::unique_ptr<GameSession> game_session_;
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<HUD> hud_;
    std::unique_ptr<GameOverOverlay> game_over_overlay_;

    ftxui::Component buildUI();
    ftxui::Component buildGameComponent();
    void transitionTo(GameState new_state);
};
