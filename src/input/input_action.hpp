#pragma once

struct InputSnapshot {
    bool move_left = false;
    bool move_right = false;
    bool move_up = false;
    bool move_down = false;
    bool jump_held = false;
    bool jump_just_pressed = false;
    bool jump_just_released = false;
    bool confirm = false;
    bool back = false;
    bool pause = false;
    float horizontal_axis = 0.0f; // -1.0 (left) to 1.0 (right)
};
