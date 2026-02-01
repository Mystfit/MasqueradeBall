#pragma once

#include "input/input_action.hpp"
#include "input/keyboard_provider.hpp"
#include "input/gamepad_provider.hpp"

#include <ftxui/component/event.hpp>

class InputManager {
public:
    InputManager();

    // Called from FTXUI CatchEvent. Returns true if consumed.
    bool handleFtxuiEvent(const ftxui::Event& event);

    // Try to parse the event as a kitty keyboard protocol sequence.
    // Returns true if the event was a valid kitty sequence and was consumed.
    // Falls back to handleFtxuiEvent() if not a kitty sequence.
    bool handleRawEvent(const ftxui::Event& event);

    // Call once per frame before reading input
    void beginFrame();

    // Poll gamepad and finalize frame input
    void endFrame();

    // Get the current merged input state
    InputSnapshot snapshot() const;

    // Enable/disable kitty keyboard protocol mode
    void setKittyMode(bool enabled);
    bool kittyMode() const;

    // Returns true if the kitty protocol was confirmed working (received
    // at least one valid kitty sequence). Returns false if we're still
    // waiting for detection or if detection has failed.
    bool kittyConfirmed() const { return kitty_confirmed_; }

private:
    KeyboardProvider keyboard_;
    GamepadProvider gamepad_;

    enum class ActiveSource { Keyboard, Gamepad };
    ActiveSource active_source_ = ActiveSource::Keyboard;

    InputSnapshot current_;

    // Auto-detection: track whether we've actually received kitty events
    bool kitty_confirmed_ = false;
    int kitty_detection_frames_ = 0;
    static constexpr int KITTY_DETECTION_TIMEOUT = 120;  // ~2 seconds at 60fps
};
