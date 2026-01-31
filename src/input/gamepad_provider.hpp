#pragma once

#include "input/input_action.hpp"

class GamepadProvider {
public:
    GamepadProvider();
    ~GamepadProvider();

    void poll();
    InputSnapshot snapshot() const;
    bool hadActivity() const { return had_activity_; }
    bool isAvailable() const { return available_; }

private:
    bool available_ = false;
    bool had_activity_ = false;
    InputSnapshot current_;

#ifdef HAS_GAMEPAD
    // libgamepad hook handle would go here
    // std::shared_ptr<gamepad::hook> hook_;
#endif
};
