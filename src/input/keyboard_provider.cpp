#include "input/keyboard_provider.hpp"

bool KeyboardProvider::handleEvent(const ftxui::Event& event) {
    had_activity_ = true;

    if (event == ftxui::Event::ArrowLeft) {
        left_ = true;
        return true;
    }
    if (event == ftxui::Event::ArrowRight) {
        right_ = true;
        return true;
    }
    if (event == ftxui::Event::ArrowUp) {
        up_ = true;
        return true;
    }
    if (event == ftxui::Event::ArrowDown) {
        down_ = true;
        return true;
    }
    if (event == ftxui::Event::Character(' ')) {
        jump_ = true;
        return true;
    }
    if (event == ftxui::Event::Return) {
        confirm_ = true;
        return true;
    }
    if (event == ftxui::Event::Escape) {
        pause_ = true;
        back_ = true;
        return true;
    }

    return false;
}

void KeyboardProvider::beginFrame() {
    // Save previous jump state for edge detection
    prev_jump_ = jump_;

    // Clear per-frame states
    left_ = false;
    right_ = false;
    up_ = false;
    down_ = false;
    jump_ = false;
    confirm_ = false;
    back_ = false;
    pause_ = false;
    had_activity_ = false;
}

void KeyboardProvider::endFrame() {
    // Nothing needed for now; edge detection is handled in snapshot()
}

InputSnapshot KeyboardProvider::snapshot() const {
    InputSnapshot snap;
    snap.move_left = left_;
    snap.move_right = right_;
    snap.move_up = up_;
    snap.move_down = down_;
    snap.jump_held = jump_;
    snap.jump_just_pressed = jump_ && !prev_jump_;
    snap.jump_just_released = !jump_ && prev_jump_;
    snap.confirm = confirm_;
    snap.back = back_;
    snap.pause = pause_;
    snap.horizontal_axis = (left_ ? -1.0f : 0.0f) + (right_ ? 1.0f : 0.0f);
    return snap;
}
