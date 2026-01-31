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

    // Call once per frame before reading input
    void beginFrame();

    // Poll gamepad and finalize frame input
    void endFrame();

    // Get the current merged input state
    InputSnapshot snapshot() const;

private:
    KeyboardProvider keyboard_;
    GamepadProvider gamepad_;

    enum class ActiveSource { Keyboard, Gamepad };
    ActiveSource active_source_ = ActiveSource::Keyboard;

    InputSnapshot current_;
};
