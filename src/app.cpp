#include "app.hpp"
#include "input/kitty_keyboard.hpp"

#include <ftxui/component/loop.hpp>
#include <ftxui/dom/elements.hpp>

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <thread>
#include <unistd.h>

// Global flag for atexit/signal cleanup of kitty protocol
static volatile sig_atomic_t g_kitty_enabled = 0;

static void cleanupKittyProtocol() {
    if (g_kitty_enabled) {
        kitty::disable();
        g_kitty_enabled = 0;
    }
}

static void signalHandler(int signum) {
    // Use write() instead of printf - it's async-signal-safe
    if (g_kitty_enabled) {
        const char seq[] = "\x1b[<u";
        write(STDOUT_FILENO, seq, sizeof(seq) - 1);
        g_kitty_enabled = 0;
    }
    // Re-raise with default handler to get proper exit behavior
    std::signal(signum, SIG_DFL);
    std::raise(signum);
}

App::App(std::unique_ptr<StdinReader> stdin_reader)
    : screen_(ftxui::ScreenInteractive::Fullscreen()),
      stdin_reader_(std::move(stdin_reader)) {

    input_manager_ = std::make_unique<InputManager>();
    game_session_ = std::make_unique<GameSession>(*stdin_reader_);
    renderer_ = std::make_unique<Renderer>();
    mask_renderer_ = std::make_unique<MaskRenderer>("assets/mask.png");
    hud_ = std::make_unique<HUD>();

    // Enable stdin
    screen_.HandlePipedInput(true);

    // Register cleanup handlers for kitty protocol
    std::atexit(cleanupKittyProtocol);
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    // Initialize camera at ball starting position
    renderer_->camera().update({5.0f, 10.0f}, 1.0f);
}

App::~App() {
    disableKittyProtocol();
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

        // Sync kitty_active_ with InputManager's auto-detection.
        // If the InputManager disabled kitty mode (unsupported terminal),
        // update our flag so CatchEvent falls back to normal FTXUI handling.
        if (kitty_active_ && !input_manager_->kittyMode()) {
            kitty_active_ = false;
            g_kitty_enabled = 0;
            kitty::disable();
        }

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

    // Ensure kitty protocol is disabled before exiting
    disableKittyProtocol();
}

ftxui::Component App::buildUI() {
    using namespace ftxui;

    auto transition = [this](GameState state) { transitionTo(state); };
    auto restart = [this]() { game_session_->restart(); };

    start_menu_ = std::make_unique<StartMenu>(transition);
    options_menu_ = std::make_unique<OptionsMenu>(transition, &debug_enabled_);
    pause_menu_ = std::make_unique<PauseMenu>(transition, restart);
    game_over_overlay_ = std::make_unique<GameOverOverlay>(transition, restart);
    level_complete_overlay_ = std::make_unique<LevelCompleteOverlay>(transition);

    auto game_component = buildGameComponent();

    // Wrap each overlay's menu component in a Renderer that calls its render()
    // method, so Modal gets both event handling (from menu) and full dialog rendering
    auto pause_modal = ftxui::Renderer(pause_menu_->component(), [this] {
        return pause_menu_->render();
    });

    auto game_over_modal = ftxui::Renderer(game_over_overlay_->component(), [this] {
        return game_over_overlay_->render();
    });

    auto level_complete_modal = ftxui::Renderer(level_complete_overlay_->component(), [this] {
        return level_complete_overlay_->render(game_session_->score());
    });

    // Chain Modal decorators onto the game component.
    // Each Modal overlays a centered dialog over the game view when its bool is true.
    game_component |= Modal(pause_modal, &show_pause_modal_);
    game_component |= Modal(game_over_modal, &show_game_over_modal_);
    game_component |= Modal(level_complete_modal, &show_level_complete_modal_);

    // Tab only needs menu states and the game-with-modals component
    auto tab = Container::Tab(
        {
            start_menu_->component(),   // 0: StartMenu
            options_menu_->component(), // 1: OptionsMenu
            game_component,             // 2: Playing / Paused / GameOver / LevelComplete
        },
        &tab_index_);

    // Wrap with global event handlers
    return CatchEvent(tab, [this](ftxui::Event event) {
        // When kitty protocol is active during gameplay, parse events through the kitty parser
        if (kitty_active_ && current_state_ == GameState::Playing) {

            // Try to parse as kitty sequence for game input
            auto kitty_event = kitty::parse(event.input());
            if (kitty_event) {
                // Route to input manager for game controls
                input_manager_->handleRawEvent(event);

                // Handle escape key for pause (only on press)
                if (kitty_event->event_type == kitty::EventType::Press) {
                    auto game_key = kitty::toGameKey(kitty_event->keycode);

                    if (game_key == kitty::GameKey::Escape) {
                        transitionTo(GameState::Paused);
                        return true;
                    }
                }

                return true;  // Consume all kitty events during gameplay
            }

            // Not a kitty sequence - pass through to regular handling
            // (fallback for terminals that don't support kitty protocol)
            input_manager_->handleFtxuiEvent(event);
            return false;
        }

        // Non-kitty mode: pass events to input manager when in game
        if (current_state_ == GameState::Playing) {
            input_manager_->handleFtxuiEvent(event);
        }

        // Global escape handler (for menu states and non-kitty fallback)
        if (event == ftxui::Event::Escape) {
            if (current_state_ == GameState::StartMenu) {
                screen_.Exit();
                return true;
            } else if (current_state_ == GameState::OptionsMenu) {
                transitionTo(GameState::StartMenu);
                return true;
            } else if (current_state_ == GameState::Paused) {
                transitionTo(GameState::Playing);
                return true;
            } else if (current_state_ == GameState::LevelComplete) {
                transitionTo(GameState::StartMenu);
                return true;
            } else if (current_state_ == GameState::Playing) {
                // Non-kitty fallback: pause on Escape
                transitionTo(GameState::Paused);
                return true;
            }
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

            // Update camera screen size in braille pixels for proper centering
            camera.setScreenSize(screen_width, screen_height);

            // Draw terrain
            TerrainRenderer terrain_renderer;
            terrain_renderer.draw(c, camera, game_session_->segments());

            // Draw ball
            BallRenderer ball_renderer;
            if (debug_enabled_) {
                ball_renderer.drawDebug(c, camera,
                                       game_session_->ball().getCenterPosition(),
                                       game_session_->ball().getRimPositions(),
                                       SoftbodyBall::CORE_RADIUS,
                                       SoftbodyBall::RIM_CIRCLE_RADIUS);
            } else {
                ball_renderer.draw(c, camera,
                                  game_session_->ball().getCenterPosition(),
                                  game_session_->ball().getRimPositions());
            }

            // Draw mask overlay
            mask_renderer_->draw(c, camera,
                                 game_session_->mask().getPosition());
        });

        // Build UI layers
        auto hud_element = hud_->render(game_session_->score(),
                                        game_session_->speedMultiplier(),
                                        debug_enabled_,
                                        input_manager_->snapshot());

        // Build text bar overlay at bottom third of screen
        auto& camera = renderer_->camera();
        TextBar text_bar;
        auto text_bar_element = text_bar.render(
            game_session_->segments(),
            camera.viewportLeft(),
            camera.viewportRight(),
            screen_.dimx(),
            screen_width  // Pass braille pixel width for proper scaling
        );

        auto game_view = vbox({
            hud_element,
            dbox({
                game_canvas | flex,
                vbox({
                    filler() | flex_grow,
                    text_bar_element,
                    filler() | size(HEIGHT, EQUAL, 2),
                }),
            }) | flex,
        });

        return game_view;
    });
}

void App::transitionTo(GameState new_state) {
    // Toggle kitty keyboard protocol based on game state
    // Enable only for Playing (fast input needed)
    // Disable for all overlay/menu states so FTXUI can handle menu navigation
    if (new_state == GameState::Playing) {
        enableKittyProtocol();
    } else {
        disableKittyProtocol();
    }

    current_state_ = new_state;

    // Reset all modal visibility
    show_pause_modal_ = false;
    show_game_over_modal_ = false;
    show_level_complete_modal_ = false;

    // Map game state to tab index and modal flags
    switch (new_state) {
        case GameState::StartMenu:
            tab_index_ = 0;
            break;
        case GameState::OptionsMenu:
            tab_index_ = 1;
            break;
        case GameState::Playing:
            tab_index_ = 2;
            break;
        case GameState::Paused:
            tab_index_ = 2;
            show_pause_modal_ = true;
            break;
        case GameState::GameOver:
            tab_index_ = 2;
            show_game_over_modal_ = true;
            break;
        case GameState::LevelComplete:
            tab_index_ = 2;
            show_level_complete_modal_ = true;
            break;
    }
}

void App::enableKittyProtocol() {
    if (!kitty_active_) {
        kitty::enable(0b11);  // disambiguate + report event types
        kitty_active_ = true;
        g_kitty_enabled = 1;
        input_manager_->setKittyMode(true);
    }
}

void App::disableKittyProtocol() {
    if (kitty_active_) {
        kitty::disable();
        kitty_active_ = false;
        g_kitty_enabled = 0;
        input_manager_->setKittyMode(false);
    }
}
