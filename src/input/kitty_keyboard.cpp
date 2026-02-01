#include "input/kitty_keyboard.hpp"

#include <cstdio>
#include <vector>

namespace kitty {

void enable(int flags) {
    // CSI > flags u — push keyboard mode with given progressive enhancement flags
    std::printf("\x1b[>%du", flags);
    std::fflush(stdout);
}

void disable() {
    // CSI < u — pop keyboard mode (restore previous state)
    std::printf("\x1b[<u");
    std::fflush(stdout);
}

// Parse a semicolon/colon-separated parameter field from a CSI sequence.
// Returns the numeric values split by ':' sub-parameters within a single
// ';'-separated field.
struct CsiParam {
    uint32_t value = 0;       // Main parameter value
    uint32_t sub_value = 0;   // Sub-parameter after ':'
    bool has_sub = false;
};

static auto parseCsiParams(const std::string& raw, size_t start, size_t end)
    -> std::vector<CsiParam> {
    std::vector<CsiParam> params;
    CsiParam current;
    bool in_sub = false;

    for (size_t i = start; i < end; ++i) {
        char c = raw[i];
        if (c >= '0' && c <= '9') {
            uint32_t digit = static_cast<uint32_t>(c - '0');
            if (in_sub) {
                current.sub_value = current.sub_value * 10 + digit;
            } else {
                current.value = current.value * 10 + digit;
            }
        } else if (c == ':') {
            current.has_sub = true;
            in_sub = true;
        } else if (c == ';') {
            params.push_back(current);
            current = {};
            in_sub = false;
        }
    }
    params.push_back(current);
    return params;
}

auto parse(const std::string& raw) -> std::optional<KeyEvent> {
    // Minimum valid kitty sequence: ESC [ digit u = 4 bytes
    if (raw.size() < 4) {
        return std::nullopt;
    }

    // Must start with CSI (ESC [)
    if (raw[0] != '\x1b' || raw[1] != '[') {
        return std::nullopt;
    }

    // Find the terminating byte (last character)
    char terminator = raw.back();

    // Determine if this is a CSI u sequence or a functional key sequence
    bool is_csi_u = (terminator == 'u');
    bool is_functional = (terminator >= 'A' && terminator <= 'H') ||
                         (terminator >= 'P' && terminator <= 'S');

    if (!is_csi_u && !is_functional) {
        return std::nullopt;
    }

    // Parse the parameter bytes between CSI and the terminator
    auto params = parseCsiParams(raw, 2, raw.size() - 1);

    if (params.empty()) {
        return std::nullopt;
    }

    KeyEvent event;

    if (is_csi_u) {
        // CSI keycode [; modifiers[:event_type] [; text_codepoints]] u
        event.keycode = params[0].value;

        if (params.size() >= 2) {
            // Modifiers are encoded as 1 + actual_modifiers
            uint32_t mod_field = params[1].value;
            event.modifiers = (mod_field > 0) ? static_cast<uint8_t>(mod_field - 1) : 0;

            // Event type is a sub-parameter of the modifiers field
            if (params[1].has_sub) {
                uint8_t et = static_cast<uint8_t>(params[1].sub_value);
                if (et >= 1 && et <= 3) {
                    event.event_type = static_cast<EventType>(et);
                } else {
                    event.event_type = EventType::Press;
                }
            } else {
                event.event_type = EventType::Press;
            }
        } else {
            event.modifiers = 0;
            event.event_type = EventType::Press;
        }
    } else {
        // Functional key form: CSI 1 ; modifiers[:event_type] [ABCDEFHPQS]
        // The first parameter is typically 1 for functional keys
        // Map terminator to keycode
        switch (terminator) {
            case 'A': event.keycode = 57417; break;  // Up
            case 'B': event.keycode = 57420; break;  // Down
            case 'C': event.keycode = 57421; break;  // Right (note: spec uses this)
            case 'D': event.keycode = 57419; break;  // Left
            case 'H': event.keycode = 57423; break;  // Home
            case 'F': event.keycode = 57424; break;  // End
            case 'P': event.keycode = 57364; break;  // F1
            case 'Q': event.keycode = 57365; break;  // F2
            case 'R': event.keycode = 57366; break;  // F3
            case 'S': event.keycode = 57367; break;  // F4
            default:  event.keycode = 0; break;
        }

        if (params.size() >= 2) {
            uint32_t mod_field = params[1].value;
            event.modifiers = (mod_field > 0) ? static_cast<uint8_t>(mod_field - 1) : 0;

            if (params[1].has_sub) {
                uint8_t et = static_cast<uint8_t>(params[1].sub_value);
                if (et >= 1 && et <= 3) {
                    event.event_type = static_cast<EventType>(et);
                } else {
                    event.event_type = EventType::Press;
                }
            } else {
                event.event_type = EventType::Press;
            }
        } else {
            event.modifiers = 0;
            event.event_type = EventType::Press;
        }
    }

    return event;
}

auto toGameKey(uint32_t keycode) -> GameKey {
    switch (keycode) {
        case 57419: return GameKey::Left;    // KP_LEFT / Arrow Left
        case 57421: return GameKey::Right;   // KP_RIGHT / Arrow Right
        case 57417: return GameKey::Up;      // KP_UP / Arrow Up
        case 57420: return GameKey::Down;    // KP_DOWN / Arrow Down
        case 32:    return GameKey::Space;
        case 13:    return GameKey::Enter;
        case 27:    return GameKey::Escape;
        default:    return GameKey::Unknown;
    }
}

auto functionalTerminatorToGameKey(char terminator) -> GameKey {
    switch (terminator) {
        case 'A': return GameKey::Up;
        case 'B': return GameKey::Down;
        case 'C': return GameKey::Right;
        case 'D': return GameKey::Left;
        default:  return GameKey::Unknown;
    }
}

}  // namespace kitty
