#include "input/input_manager.hpp"

InputManager::InputManager() = default;

bool InputManager::handleFtxuiEvent(const ftxui::Event& event) {
    bool consumed = keyboard_.handleEvent(event);
    if (consumed) {
        active_source_ = ActiveSource::Keyboard;
    }
    return consumed;
}

void InputManager::beginFrame() {
    keyboard_.beginFrame();
}

void InputManager::endFrame() {
    keyboard_.endFrame();

    // Poll gamepad
    gamepad_.poll();
    if (gamepad_.hadActivity()) {
        active_source_ = ActiveSource::Gamepad;
    }

    // Merge from active source
    if (active_source_ == ActiveSource::Gamepad && gamepad_.isAvailable()) {
        current_ = gamepad_.snapshot();
    } else {
        current_ = keyboard_.snapshot();
    }
}

InputSnapshot InputManager::snapshot() const {
    return current_;
}
