#include "input/keyboard_provider.hpp"

bool KeyboardProvider::handleEvent(const ftxui::Event& event) {
    had_activity_ = true;

    if (event == ftxui::Event::ArrowLeft) {
        left_ = true;
        left_event_this_frame_ = true;
        left_frames_since_event_ = 0;
        return true;
    }
    if (event == ftxui::Event::ArrowRight) {
        right_ = true;
        right_event_this_frame_ = true;
        right_frames_since_event_ = 0;
        return true;
    }
    if (event == ftxui::Event::ArrowUp) {
        up_ = true;
        up_event_this_frame_ = true;
        up_frames_since_event_ = 0;
        return true;
    }
    if (event == ftxui::Event::ArrowDown) {
        down_ = true;
        down_event_this_frame_ = true;
        down_frames_since_event_ = 0;
        return true;
    }
    if (event == ftxui::Event::Character(' ')) {
        jump_ = true;
        jump_event_this_frame_ = true;
        jump_frames_since_event_ = 0;
        return true;
    }
    if (event == ftxui::Event::Return) {
        confirm_ = true;
        confirm_event_this_frame_ = true;
        confirm_frames_since_event_ = 0;
        return true;
    }
    if (event == ftxui::Event::Escape) {
        pause_ = true;
        pause_event_this_frame_ = true;
        pause_frames_since_event_ = 0;
        back_ = true;
        back_event_this_frame_ = true;
        back_frames_since_event_ = 0;
        return true;
    }

    return false;
}

void KeyboardProvider::beginFrame() {
    // Save previous states for edge detection
    prev_left_ = left_;
    prev_right_ = right_;
    prev_up_ = up_;
    prev_down_ = down_;
    prev_jump_ = jump_;
    prev_confirm_ = confirm_;
    prev_back_ = back_;
    prev_pause_ = pause_;

    // Clear event flags for this frame
    left_event_this_frame_ = false;
    right_event_this_frame_ = false;
    up_event_this_frame_ = false;
    down_event_this_frame_ = false;
    jump_event_this_frame_ = false;
    confirm_event_this_frame_ = false;
    back_event_this_frame_ = false;
    pause_event_this_frame_ = false;

    had_activity_ = false;
}

void KeyboardProvider::endFrame() {
    // Timeout-based release detection: increment frame counters for keys that didn't
    // receive an event this frame, and clear them after RELEASE_TIMEOUT_FRAMES.
    // This tolerates the OS key-repeat delay while still detecting releases.

    if (!left_event_this_frame_) {
        left_frames_since_event_++;
        if (left_frames_since_event_ >= RELEASE_TIMEOUT_FRAMES) {
            left_ = false;
        }
    }

    if (!right_event_this_frame_) {
        right_frames_since_event_++;
        if (right_frames_since_event_ >= RELEASE_TIMEOUT_FRAMES) {
            right_ = false;
        }
    }

    if (!up_event_this_frame_) {
        up_frames_since_event_++;
        if (up_frames_since_event_ >= RELEASE_TIMEOUT_FRAMES) {
            up_ = false;
        }
    }

    if (!down_event_this_frame_) {
        down_frames_since_event_++;
        if (down_frames_since_event_ >= RELEASE_TIMEOUT_FRAMES) {
            down_ = false;
        }
    }

    if (!jump_event_this_frame_) {
        jump_frames_since_event_++;
        if (jump_frames_since_event_ >= RELEASE_TIMEOUT_FRAMES) {
            jump_ = false;
        }
    }

    if (!confirm_event_this_frame_) {
        confirm_frames_since_event_++;
        if (confirm_frames_since_event_ >= RELEASE_TIMEOUT_FRAMES) {
            confirm_ = false;
        }
    }

    if (!back_event_this_frame_) {
        back_frames_since_event_++;
        if (back_frames_since_event_ >= RELEASE_TIMEOUT_FRAMES) {
            back_ = false;
        }
    }

    if (!pause_event_this_frame_) {
        pause_frames_since_event_++;
        if (pause_frames_since_event_ >= RELEASE_TIMEOUT_FRAMES) {
            pause_ = false;
        }
    }
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
