#include "app.hpp"

#include <ftxui/component/loop.hpp>
#include <ftxui/dom/elements.hpp>

#include <chrono>
#include <thread>

App::App(std::unique_ptr<StdinReader> stdin_reader)
    : screen_(ftxui::ScreenInteractive::Fullscreen()),
      stdin_reader_(std::move(stdin_reader)) {

    input_manager_ = std::make_unique<InputManager>();
    game_session_ = std::make_unique<GameSession>(*stdin_reader_);
    renderer_ = std::make_unique<Renderer>();
    hud_ = std::make_unique<HUD>();
    game_over_overlay_ = std::make_unique<GameOverOverlay>();

    // Initialize camera at ball starting position
    renderer_->camera().update({5.0f, 10.0f}, 1.0f);
}

void App::run() {
    auto ui = buildUI();
    ftxui::Loop loop(&screen_, ui);

    auto last_time = std::chrono::steady_clock::now();
    constexpr auto frame_duration = std::chrono::milliseconds(1000 / 60);

    while (!loop.HasQuitted()) {
        auto now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(now - last_time).count();
        last_time = now;

        input_manager_->beginFrame();
        loop.RunOnce();
        input_manager_->endFrame();

        if (current_state_ == GameState::Playing) {
            // Update game
            game_session_->update(dt, input_manager_->snapshot());

            // Update camera to track ball
            renderer_->camera().update(game_session_->ball().getCenterPosition(), dt);

            // Check for game state transitions
            if (game_session_->isGameOver()) {
                transitionTo(GameState::GameOver);
            } else if (game_session_->isLevelComplete()) {
                transitionTo(GameState::LevelComplete);
            }
        }

        screen_.RequestAnimationFrame();
        std::this_thread::sleep_until(now + frame_duration);
    }
}

ftxui::Component App::buildUI() {
    using namespace ftxui;

    auto transition = [this](GameState state) { transitionTo(state); };

    start_menu_ = std::make_unique<StartMenu>(transition);
    options_menu_ = std::make_unique<OptionsMenu>(transition);

    auto game_component = buildGameComponent();

    // Placeholder screens
    auto paused_placeholder = ftxui::Renderer([] {
        return text("PAUSED") | bold | center | border | flex;
    });

    auto level_complete_screen = ftxui::Renderer([this] {
        return vbox({
                   filler(),
                   text("LEVEL COMPLETE!") | bold | center,
                   text("Final Score: " + std::to_string(game_session_->score())) |
                       center,
                   text("Press Escape to return to menu") | center | dim,
                   filler(),
               }) |
               border | flex;
    });

    auto tab = Container::Tab(
        {
            start_menu_->component(),  // 0: StartMenu
            options_menu_->component(), // 1: OptionsMenu
            game_component,            // 2: Playing
            paused_placeholder,        // 3: Paused
            ftxui::Renderer([]{return text("GAME OVER");}) | flex, // 4: GameOver (handled in game_component)
            level_complete_screen,     // 5: LevelComplete
        },
        &tab_index_);

    // Wrap with global event handlers
    return CatchEvent(tab, [this](ftxui::Event event) {
        // Pass events to input manager when in game
        if (current_state_ == GameState::Playing || current_state_ == GameState::GameOver) {
            input_manager_->handleFtxuiEvent(event);
        }

        // Global escape handler
        if (event == ftxui::Event::Escape) {
            if (current_state_ == GameState::StartMenu) {
                screen_.Exit();
                return true;
            } else if (current_state_ == GameState::OptionsMenu) {
                transitionTo(GameState::StartMenu);
                return true;
            } else if (current_state_ == GameState::LevelComplete) {
                transitionTo(GameState::StartMenu);
                return true;
            }
        }

        // Restart on Enter from game over
        if (event == ftxui::Event::Return && current_state_ == GameState::GameOver) {
            game_session_->restart();
            transitionTo(GameState::Playing);
            return true;
        }

        return false;
    });
}

ftxui::Component App::buildGameComponent() {
    using namespace ftxui;

    return ftxui::Renderer([this] {
        // Get terminal dimensions
        int screen_width = screen_.dimx() * 2;   // Braille: 2 pixels per column
        int screen_height = screen_.dimy() * 4;  // Braille: 4 pixels per row

        // Render game world with both terrain and ball (DEBUG MODE)
        auto game_canvas = canvas(screen_width / 2, screen_height / 2, [this, screen_width, screen_height](Canvas& c) {
            auto& camera = renderer_->camera();

            // Update camera screen size for proper centering
            camera.setScreenSize(screen_width / 2, screen_height / 2);

            // Draw terrain
            TerrainRenderer terrain_renderer;
            terrain_renderer.draw(c, camera, game_session_->segments());

            // Draw ball in DEBUG mode - shows physics bodies and constraints
            BallRenderer ball_renderer;
            //ball_renderer.draw(c, camera, game_session_->ball().getRimPositions());
            ball_renderer.drawDebug(c, camera,
                                   game_session_->ball().getCenterPosition(),
                                   game_session_->ball().getRimPositions(),
                                   0.15f,  // core radius
                                   0.05f); // rim radius
        });

        // Build UI layers
        auto hud_element = hud_->render(game_session_->score(),
                                        game_session_->speedMultiplier());

        auto game_view = vbox({
            hud_element,
            game_canvas | flex,
        });

        // Overlay game over if needed
        if (current_state_ == GameState::GameOver) {
            return dbox({
                game_view,
                game_over_overlay_->render(),
            });
        }

        return game_view;
    });
}

void App::transitionTo(GameState new_state) {
    current_state_ = new_state;
    tab_index_ = static_cast<int>(new_state);
}
