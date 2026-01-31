#pragma once

#include "input/input_action.hpp"

#include <ftxui/component/event.hpp>

class KeyboardProvider {
public:
    // Process an FTXUI event. Returns true if the event was consumed.
    bool handleEvent(const ftxui::Event& event);

    // Call once per frame before reading snapshot to detect releases.
    void beginFrame();

    // Call once per frame after processing all events.
    void endFrame();

    InputSnapshot snapshot() const;
    bool hadActivity() const { return had_activity_; }

private:
    // Current frame key states
    bool left_ = false;
    bool right_ = false;
    bool up_ = false;
    bool down_ = false;
    bool jump_ = false;
    bool confirm_ = false;
    bool back_ = false;
    bool pause_ = false;

    // Previous frame state for edge detection
    bool prev_jump_ = false;

    bool had_activity_ = false;
};
