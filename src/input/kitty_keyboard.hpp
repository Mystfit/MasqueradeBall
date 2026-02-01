#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace kitty {

enum class EventType : uint8_t {
    Press = 1,
    Repeat = 2,
    Release = 3,
};

struct KeyEvent {
    uint32_t keycode;
    uint8_t modifiers;
    EventType event_type;
};

enum class GameKey {
    Left,
    Right,
    Up,
    Down,
    Space,
    Enter,
    Escape,
    Unknown,
};

// Enable the kitty keyboard protocol with the given progressive enhancement flags.
// Default flags: 0b11 = disambiguate escape codes + report event types.
// Writes the escape sequence to stdout.
void enable(int flags = 0b11);

// Disable the kitty keyboard protocol by popping the previously pushed mode.
// Writes the escape sequence to stdout.
void disable();

// Parse raw terminal input bytes (from FTXUI event.input()) as a kitty keyboard
// protocol sequence. Returns nullopt if the input is not a valid kitty sequence.
// Handles both CSI u form and functional key form (CSI 1;... [ABCDEFHPQS]).
auto parse(const std::string& raw) -> std::optional<KeyEvent>;

// Map a kitty protocol keycode to a game-relevant key action.
auto toGameKey(uint32_t keycode) -> GameKey;

// Map a CSI functional key terminator character (A-H, P-S) to a GameKey.
// Returns GameKey::Unknown for unrecognized terminators.
auto functionalTerminatorToGameKey(char terminator) -> GameKey;

}  // namespace kitty
