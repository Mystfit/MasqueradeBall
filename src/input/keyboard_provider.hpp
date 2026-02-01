#pragma once

#include "input/input_action.hpp"
#include "input/kitty_keyboard.hpp"

#include <ftxui/component/event.hpp>

class KeyboardProvider {
public:
    // Process an FTXUI event. Returns true if the event was consumed.
    // Used when kitty protocol is not active (menu states).
    bool handleEvent(const ftxui::Event& event);

    // Process a kitty keyboard press event. Sets key state immediately.
    void handlePress(kitty::GameKey key);

    // Process a kitty keyboard release event. Clears key state immediately.
    void handleRelease(kitty::GameKey key);

    // Call once per frame before reading snapshot to detect releases.
    void beginFrame();

    // Call once per frame after processing all events.
    void endFrame();

    InputSnapshot snapshot() const;
    bool hadActivity() const { return had_activity_; }

    // Enable/disable kitty mode. When enabled, timeout-based release
    // detection is skipped (we get real release events instead).
    void setKittyMode(bool enabled);
    bool kittyMode() const { return kitty_mode_; }

private:
    // Set or clear the boolean for a given game key
    void setKeyState(kitty::GameKey key, bool pressed);

    bool kitty_mode_ = false;

    // Current frame key states (persistent until release detected)
    bool left_ = false;
    bool right_ = false;
    bool up_ = false;
    bool down_ = false;
    bool jump_ = false;
    bool confirm_ = false;
    bool back_ = false;
    bool pause_ = false;

    // Previous frame state for edge detection
    bool prev_left_ = false;
    bool prev_right_ = false;
    bool prev_up_ = false;
    bool prev_down_ = false;
    bool prev_jump_ = false;
    bool prev_confirm_ = false;
    bool prev_back_ = false;
    bool prev_pause_ = false;

    // Track if key received event this frame (for release detection)
    bool left_event_this_frame_ = false;
    bool right_event_this_frame_ = false;
    bool up_event_this_frame_ = false;
    bool down_event_this_frame_ = false;
    bool jump_event_this_frame_ = false;
    bool confirm_event_this_frame_ = false;
    bool back_event_this_frame_ = false;
    bool pause_event_this_frame_ = false;

    // Frame counters since last event (for timeout-based release detection)
    int left_frames_since_event_ = 0;
    int right_frames_since_event_ = 0;
    int up_frames_since_event_ = 0;
    int down_frames_since_event_ = 0;
    int jump_frames_since_event_ = 0;
    int confirm_frames_since_event_ = 0;
    int back_frames_since_event_ = 0;
    int pause_frames_since_event_ = 0;

    // Release timeout: clear key after this many frames without an event
    // At 60 FPS, 10 frames = ~166ms (tolerates OS key-repeat delay)
    static constexpr int RELEASE_TIMEOUT_FRAMES = 10;

    bool had_activity_ = false;
};
