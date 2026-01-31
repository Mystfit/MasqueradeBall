# Masquerade Ball

A terminal-based side-scrolling physics platformer game written in C++20.

## Features

- **Softbody Physics Ball**: Deformable ball using Box2D v3 distance joints
- **Procedural Level Generation**: Levels generated from text piped via STDIN
- **Dynamic Terrain**: Cubic spline-based terrain rendered in braille mode
- **Score System**: Distance-based scoring with speed multiplier
- **Full Game Loop**: Start menu, gameplay, game over, and level complete screens
- **Keyboard Controls**: Arrow keys to move, spacebar to jump
- **Terminal Rendering**: Uses FTXUI for high-performance terminal graphics

## Building

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

## Running

### With piped input (Vib-Ribbon style):
```bash
echo "Hello World" | ./build/masquerade_ball
cat some_text_file.txt | ./build/masquerade_ball
```

### Without piped input (uses lorem ipsum):
```bash
./build/masquerade_ball
```

## Controls

**Menu Navigation:**
- Arrow Up/Down: Navigate menu
- Enter: Select option
- Escape: Quit from start menu, back from options

**Gameplay:**
- Arrow Left/Right: Roll ball left/right
- Spacebar (tap): Small jump
- Spacebar (hold then release): Charged jump (compress ball for higher bounce)
- Enter (on game over): Restart from last position

## Game Mechanics

- **Ball Movement**: Physics-based rolling with torque
- **Terrain**: Generated from input text using cubic splines
- **Gaps**: Newlines in input create gaps to jump over
- **Falling**: Falling through gaps causes game over
- **Goal**: Reaching EOF creates a goal to cross for level complete
- **Scoring**: Distance traveled × speed multiplier

## Dependencies

- FTXUI v6.1.9 - Terminal UI
- Box2D v3.1.0 - Physics engine
- C++20 compiler (GCC/Clang)

All dependencies are fetched automatically via CMake FetchContent.

## Architecture

Phases 1-7 implemented (playable foundation):
1. Build system & skeleton
2. Menu system & state machine
3. Input system (keyboard + gamepad support structure)
4. Physics world & softbody ball
5. Ball rendering & camera
6. Level generation & terrain rendering
7. Game session & scoring

Deferred to future iterations:
- Phase 8: Parallax layers (stars, background curves, speed lines)
- Phase 9: Pause menu & polish
- Phase 10: Audio (libmikmod music, STK sound effects)

## Project Structure

```
src/
├── main.cpp
├── app.{hpp,cpp}              # Application shell & game loop
├── game_state.hpp             # State machine
├── input/                     # Unified input abstraction
├── physics/                   # Box2D wrappers
├── rendering/                 # Camera & renderers
├── level/                     # Level generation & STDIN reader
├── ui/                        # Menus, HUD, overlays
└── game/                      # Game session & scoring
```

## License

Built with Claude Code
