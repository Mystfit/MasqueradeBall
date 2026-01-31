#include "input/gamepad_provider.hpp"

GamepadProvider::GamepadProvider() {
#ifdef HAS_GAMEPAD
    // TODO: Initialize libgamepad hook
    // hook_ = gamepad::hook::make();
    // hook_->start();
    // hook_->set_plug_and_play(500);
    available_ = true;
#else
    available_ = false;
#endif
}

GamepadProvider::~GamepadProvider() {
#ifdef HAS_GAMEPAD
    // TODO: Clean up libgamepad hook
#endif
}

void GamepadProvider::poll() {
#ifdef HAS_GAMEPAD
    // TODO: Poll gamepad state and update current_
    // Check buttons: A = jump, Start = pause, DPad/Stick = movement
    had_activity_ = false;
#else
    had_activity_ = false;
#endif
}

InputSnapshot GamepadProvider::snapshot() const {
    return current_;
}
