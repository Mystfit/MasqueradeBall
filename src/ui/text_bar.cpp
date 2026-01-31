#include "ui/text_bar.hpp"

#include <ftxui/dom/elements.hpp>

#include <cmath>
#include <sstream>

ftxui::Element TextBar::render(const std::vector<LevelSegment>& segments,
                                float viewport_left,
                                float viewport_right,
                                int screen_width_chars,
                                int screen_width_braille_pixels) {
    using namespace ftxui;

    // Available width inside the border (left + right border chars)
    int text_width = screen_width_chars - 2;
    if (text_width <= 0) {
        return border(text(""));
    }

    // Each display character occupies 0.15 world units (matches level_generator.cpp)
    constexpr float chars_to_world = 0.15f;
    constexpr float pixels_per_meter = 30.0f;

    // Viewport width in world units
    float viewport_width_world = screen_width_braille_pixels / pixels_per_meter;

    // Build a buffer of spaces, then place words from visible segments
    std::string buffer(text_width, ' ');

    for (const auto& segment : segments) {
        // Quick cull: skip segments entirely outside viewport
        if (segment.end_x < viewport_left || segment.start_x > viewport_right) {
            continue;
        }

        if (segment.display_text.empty()) {
            continue;
        }

        // Parse display_text into words and their positions
        std::istringstream iss(segment.display_text);
        std::string word;
        size_t char_index = 0;

        while (iss >> word) {
            // Find the position of this word in display_text
            size_t word_start = segment.display_text.find(word, char_index);
            if (word_start == std::string::npos) {
                break;
            }
            char_index = word_start + word.length();

            // World position of the first character of this word
            float word_world_x = segment.start_x + word_start * chars_to_world;

            // Check if word is within viewport
            float word_end_world_x = word_world_x + word.length() * chars_to_world;
            if (word_end_world_x < viewport_left || word_world_x > viewport_right) {
                continue;
            }

            // Map word's world position to screen column
            float normalized = (word_world_x - viewport_left) / viewport_width_world;
            int start_col = static_cast<int>(normalized * text_width);

            // Draw each character of the word consecutively
            for (size_t i = 0; i < word.length(); ++i) {
                int col = start_col + static_cast<int>(i);
                if (col >= 0 && col < text_width) {
                    buffer[col] = word[i];
                }
            }
        }
    }

    return text(buffer) | border;
}
