#include "input/input_manager.hpp"
#include "input/kitty_keyboard.hpp"

InputManager::InputManager() = default;

bool InputManager::handleFtxuiEvent(const ftxui::Event& event) {
    bool consumed = keyboard_.handleEvent(event);
    if (consumed) {
        active_source_ = ActiveSource::Keyboard;
    }
    return consumed;
}

bool InputManager::handleRawEvent(const ftxui::Event& event) {
    // Try to parse as a kitty keyboard protocol sequence
    auto kitty_event = kitty::parse(event.input());
    if (kitty_event) {
        // Mark kitty protocol as confirmed working
        kitty_confirmed_ = true;

        auto game_key = kitty::toGameKey(kitty_event->keycode);

        switch (kitty_event->event_type) {
            case kitty::EventType::Press:
            case kitty::EventType::Repeat:
                keyboard_.handlePress(game_key);
                break;
            case kitty::EventType::Release:
                keyboard_.handleRelease(game_key);
                break;
        }

        active_source_ = ActiveSource::Keyboard;
        return true;
    }

    // Not a kitty sequence - fall back to regular FTXUI event handling
    return handleFtxuiEvent(event);
}

void InputManager::setKittyMode(bool enabled) {
    keyboard_.setKittyMode(enabled);
    if (enabled) {
        kitty_detection_frames_ = 0;
        // Don't reset kitty_confirmed_ - once confirmed, stays confirmed
    }
}

bool InputManager::kittyMode() const {
    return keyboard_.kittyMode();
}

void InputManager::beginFrame() {
    keyboard_.beginFrame();
}

void InputManager::endFrame() {
    // Auto-detect kitty protocol support: if we've been in kitty mode
    // for KITTY_DETECTION_TIMEOUT frames without receiving any kitty
    // events, the terminal likely doesn't support the protocol.
    // Fall back to timeout-based release detection.
    if (keyboard_.kittyMode() && !kitty_confirmed_) {
        kitty_detection_frames_++;
        if (kitty_detection_frames_ >= KITTY_DETECTION_TIMEOUT) {
            keyboard_.setKittyMode(false);
        }
    }

    keyboard_.endFrame();

    // Poll gamepad

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
