# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Run

```bash
# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Run with piped text input (generates levels from text)
echo "Hello World" | ./build/masquerade_ball
cat some_file.txt | ./build/masquerade_ball

# Run without pipe (uses assets/lorem_ipsum.txt as fallback)
./build/masquerade_ball

# Enable optional gamepad support
cmake -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_GAMEPAD=ON
```

There are no tests or linting configured.

## Project Overview

Terminal-based side-scrolling physics platformer in C++20, inspired by PS1's Vib-Ribbon. A deformable softbody ball rolls along procedurally-generated terrain created from text piped via STDIN. Renders entirely in the terminal using FTXUI braille graphics.

## Architecture

The codebase follows a layered architecture with clear separation between subsystems. `App` (src/app.cpp) is the top-level coordinator that owns all subsystems and runs the game loop.

### Game Loop (in App::run)
FTXUI's `Loop::RunOnce()` processes terminal events at ~60 FPS. Each frame: process input → update game session → step physics → generate terrain ahead of camera → render to braille canvas → request next animation frame.

### Subsystem Layers

- **Input** (`src/input/`): `InputManager` produces an `InputSnapshot` struct each frame, abstracting keyboard and optional gamepad. Keyboard provider uses frame-based edge detection for key releases (FTXUI only provides press events).

- **Physics** (`src/physics/`): Box2D v3 (C API) wrapped in C++ classes. The rest of the codebase never touches `b2*` types directly. `SoftbodyBall` is 16 rim circles + 1 core circle connected by distance joints with spring-damper parameters. Movement is torque-based; jumping uses impulses with optional compression (hold spacebar to shorten spoke joints, release for bigger impulse).

- **Level** (`src/level/`): `StdinReader` runs a background thread reading STDIN lines into a mutex-protected queue. `LevelGenerator` converts text lines into `LevelSegment`s with cubic spline terrain. Blank lines create gaps. EOF triggers a goal.

- **Rendering** (`src/rendering/`): `Camera` tracks the ball with lerp smoothing and converts Box2D world coords (meters, Y-up) to braille screen coords (pixels, Y-down) at 30 braille pixels per meter. `BallRenderer` draws a closed cubic spline through rim positions. `TerrainRenderer` draws terrain curves with source text overlaid.

- **Game** (`src/game/`): `GameSession` ties physics, level, and scoring together. Handles fall-through-gap detection (game over) and goal crossing (level complete). Scoring is distance × speed multiplier (up to 5x).

- **UI** (`src/ui/`): FTXUI-based menus, HUD, and overlays. State machine in `game_state.hpp` with 6 states: StartMenu, OptionsMenu, Playing, Paused, GameOver, LevelComplete.

### Key Constraints

- **Threading**: Only two threads — main thread (FTXUI loop + physics + rendering) and STDIN reader. Box2D is not thread-safe and must stay on the main thread.
- **Coordinate system**: Box2D uses meters (Y-up), screen uses braille pixels (Y-down). Camera handles the flip. Scale: 30 braille pixels/meter.
- **FTXUI key-release workaround**: Key state cleared each frame; missing key event after held state triggers `just_released` for one frame (adds ≤16ms latency).

## Dependencies

All fetched via CMake FetchContent (in `cmake/Dependencies.cmake`):

| Dependency | Version | Purpose |
|---|---|---|
| FTXUI | v6.1.9 (main branch in progress) | Terminal UI, canvas braille rendering |
| Box2D | v3.1.0 | Physics engine (C API) |
| libgamepad | master | Optional gamepad support (compile-guarded with `HAS_GAMEPAD`) |

## Deferred Features

Phases 8-10 are not yet implemented: parallax background layers, pause menu polish, and audio (libmikmod + STK).
