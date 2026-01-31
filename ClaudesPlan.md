Claude’s Plan
Masquerade Ball - Implementation Plan
Terminal-based side-scrolling physics platformer in C++20 using FTXUI, Box2D v3, and libgamepad.

Scope
Phases 1-7: Build system, menus, input, physics, ball rendering, level generation, and core game loop. Parallax layers, UI overlays, and audio are deferred to a later iteration.

Decisions
C++20 with traditional headers (.hpp/.cpp, no C++20 modules)
Box2D v3.1.0 (C API, 2x faster than v2.4, distance joints for softbody)
FTXUI v6.1.9 (Canvas braille rendering, custom game loop via ftxui::Loop)
Gamepad via libgamepad (optional, compile-guarded)
CMake with FetchContent for all dependencies
Directory Structure

MasqueradeBall/
├── CMakeLists.txt
├── cmake/
│   └── Dependencies.cmake
├── src/
│   ├── main.cpp
│   ├── app.hpp / app.cpp
│   ├── game_state.hpp
│   ├── input/
│   │   ├── input_action.hpp
│   │   ├── input_manager.hpp / .cpp
│   │   ├── keyboard_provider.hpp / .cpp
│   │   └── gamepad_provider.hpp / .cpp
│   ├── physics/
│   │   ├── physics_world.hpp / .cpp
│   │   ├── softbody_ball.hpp / .cpp
│   │   └── terrain_body.hpp / .cpp
│   ├── rendering/
│   │   ├── renderer.hpp / .cpp
│   │   ├── ball_renderer.hpp / .cpp
│   │   ├── terrain_renderer.hpp / .cpp
│   │   └── camera.hpp / .cpp
│   ├── level/
│   │   ├── level_generator.hpp / .cpp
│   │   ├── level_segment.hpp
│   │   ├── stdin_reader.hpp / .cpp
│   │   └── cubic_spline.hpp / .cpp
│   ├── ui/
│   │   ├── start_menu.hpp / .cpp
│   │   ├── options_menu.hpp / .cpp
│   │   ├── hud.hpp / .cpp
│   │   └── game_over_overlay.hpp / .cpp
│   └── game/
│       ├── game_session.hpp / .cpp
│       └── scoring.hpp / .cpp
└── assets/
    └── lorem_ipsum.txt
Phase 1: Build System & Skeleton
Files: CMakeLists.txt, cmake/Dependencies.cmake, src/main.cpp, src/app.hpp, src/app.cpp
CMakeLists.txt - C++20, FetchContent deps, single executable target collecting all src/**/*.cpp.

cmake/Dependencies.cmake - FetchContent for:

FTXUI v6.1.9 → link ftxui::component, ftxui::dom, ftxui::screen
Box2D v3.1.0 → link box2d
libgamepad (master) → guarded by ENABLE_GAMEPAD option, link gamepad
libmikmod and STK deferred (not needed yet)
src/main.cpp - Entry point. Detects pipe via isatty(STDIN_FILENO). Creates StdinReader if piped, then creates App and calls app.run().

src/app.hpp/cpp - App class:

Owns ScreenInteractive::Fullscreen()
Runs custom loop: ftxui::Loop with RunOnce() at ~60 FPS
Manages GameState enum and active component switching via Container::Tab
Holds subsystems: InputManager, GameSession, StdinReader
Verification: Build compiles, links, and displays "Masquerade Ball" text in terminal.

Phase 2: Start Menu & State Machine
Files: src/game_state.hpp, src/ui/start_menu.hpp/.cpp, src/ui/options_menu.hpp/.cpp
GameState enum: StartMenu, OptionsMenu, Playing, Paused, GameOver, LevelComplete

StartMenuComponent: FTXUI Menu with entries ["Start", "Options"]. Full-terminal window with border. Title "Masquerade Ball" centered above menu. on_enter callback triggers state transitions.

OptionsMenuComponent: FTXUI Menu with ["Option 1", "Option 2", "Option 3", "Back"]. Back returns to StartMenu.

State transitions: Container::Tab indexed by GameState. Start → Playing, Options → OptionsMenu, Back → StartMenu.

Verification: Can navigate menus with arrow keys and Enter. Start transitions to (empty) game view.

Phase 3: Input System
Files: src/input/input_action.hpp, src/input/input_manager.hpp/.cpp, src/input/keyboard_provider.hpp/.cpp, src/input/gamepad_provider.hpp/.cpp
InputSnapshot struct: move_left, move_right, jump_held, jump_just_pressed, jump_just_released, confirm, back, pause, horizontal_axis (-1.0 to 1.0).

KeyboardProvider: Translates FTXUI events → InputSnapshot. Arrow keys for movement, spacebar for jump, Escape for pause, Enter for confirm. Uses frame-based hold detection: key state cleared each frame, key event sets it. Missing key event after held → jump_just_released for one frame.

GamepadProvider: Wraps libgamepad hook. A button → jump, Start → pause, left stick/dpad → movement. Compile-guarded with #ifdef HAS_GAMEPAD.

InputManager: Owns both providers. Auto-detection: gamepad activity switches to gamepad mode, keyboard activity switches back. handleFtxuiEvent() called from CatchEvent, poll() called each frame for gamepad. Returns merged InputSnapshot.

Verification: Print input state to screen. Arrow keys and spacebar produce correct InputSnapshot values.

Phase 4: Physics World & Softbody Ball
Files: src/physics/physics_world.hpp/.cpp, src/physics/softbody_ball.hpp/.cpp
PhysicsWorld: Wraps b2WorldId. Creates world with gravity (0, -20). Steps with b2World_Step(worldId, dt, 4). Scale: 30 braille pixels per meter.

SoftbodyBall: Constructs a deformable ball from Box2D v3 bodies + distance joints:

1 central core dynamic body (circle, radius 0.3m, density 2.0)
12 rim dynamic bodies arranged in a circle (radius 0.15m each, at 1.5m from center)
12 ring joints connecting adjacent rim bodies (distance joints, spring hertz=8, damping=0.7)
12 spoke joints connecting each rim body to core (same spring params)
Movement API:

applyMovement(float dir): Applies torque to core body to spin the ball
applyJumpImpulse(float magnitude): Applies upward linear impulse to core
compressDownward(float amount): During held jump, shortens spoke joint rest lengths to compress ball, slows horizontal damping
releaseJump(): Restores spoke lengths, applies large impulse scaled by hold duration
Accessors: getCenterPosition(), getRimPositions(), getSpeed()

Verification: Create ball and ground plane, step physics, print positions to verify ball falls and rests on ground.

Phase 5: Ball Rendering & Camera
Files: src/rendering/ball_renderer.hpp/.cpp, src/rendering/camera.hpp/.cpp, src/level/cubic_spline.hpp/.cpp
CubicSpline: Natural cubic spline interpolation. Takes sorted (x,y) control points, returns densely sampled points. Also supports closed splines (for ball outline) by duplicating endpoints.

BallRenderer: Gets 12 rim positions from SoftbodyBall, sorts by angle from center, computes closed cubic spline through them, draws pairs of sampled points with canvas.DrawPointLine() in braille mode.

Camera: Tracks ball position with smoothing (lerp). Converts world coordinates to screen braille pixel coordinates. worldToScreen(b2Vec2 pos, float parallax_factor). Viewport bounds for culling.

Rendering integration: Game component's Render() creates a canvas(width, height, [&](Canvas& c) { ... }) element. BallRenderer draws into it. Camera provides the coordinate transform.

Verification: Ball renders on screen as a deformable circle. Moving left/right shows ball rolling. Jump shows ball leaving ground and deforming on landing.

Phase 6: Level Generation & Terrain
Files: src/level/stdin_reader.hpp/.cpp, src/level/level_generator.hpp/.cpp, src/level/level_segment.hpp, src/physics/terrain_body.hpp/.cpp, src/rendering/terrain_renderer.hpp/.cpp
StdinReader: Separate thread reads std::getline() into mutex-protected queue. popNextLine() returns next line if available. isEof() signals pipe closed. If isatty() was true, loads assets/lorem_ipsum.txt and feeds it line by line instead.

LevelSegment struct: source_text, spline_points (control), sampled_points (dense), start_x, end_x, gap_after, is_goal.

LevelGenerator: Consumes lines from StdinReader. For each line:

Segment width = text length in world units
Place N control points horizontally spaced
Each point's Y = previous Y + random offset scaled by difficulty_
Cubic spline through control points → sampled_points
Blank lines between text lines → gap_after (3-5 world units)
EOF → final segment with is_goal = true
difficulty_ increases by 0.1 per segment
TerrainBody: Creates Box2D chain shapes from sampled spline points. Each LevelSegment becomes a static body with b2ChainDef. addSegment() and clear() methods.

TerrainRenderer: Draws terrain curve with canvas.DrawPointLine() between sampled points. Overlays source text characters positioned along the spline using canvas.DrawText().

Level streaming: GameSession calls generateAheadOfCamera() each frame. Generates new segments when camera approaches the end of existing terrain.

Verification: Pipe text in (echo "Hello World" | ./masquerade_ball), see terrain curve appear with text. Ball rolls along terrain. Gaps appear between lines.

Phase 7: Game Session & Core Loop
Files: src/game/game_session.hpp/.cpp, src/game/scoring.hpp/.cpp, src/ui/hud.hpp/.cpp, src/ui/game_over_overlay.hpp/.cpp
GameSession: Ties everything together:

Owns PhysicsWorld, SoftbodyBall, TerrainBody, LevelGenerator
update(float dt, InputSnapshot input): processes input → applies forces → steps physics → generates terrain ahead → checks fall/goal
processInput(): movement forces from horizontal input, jump mechanics from jump state
checkFallThroughGap(): if ball center Y drops below a threshold during a gap → game_over
checkGoalReached(): if ball center X passes goal X → level_complete
restart(): recreate ball at last line position, regenerate terrain from last_line_index
Scoring: raw_distance accumulates ball X progress in world units. speed_multiplier = clamp(current_speed / base_speed, 1.0, 5.0). Score = sum of (distance_delta * multiplier) each frame, displayed as integer.

HUD: Simple FTXUI element overlay: score left-aligned, multiplier right-aligned, rendered above the canvas.

GameOverOverlay: "GAME OVER" centered with "Press Enter to restart" below. Rendered via dbox on top of game canvas.

Goal: Two vertical block lines drawn at goal X position. Crossing triggers LevelComplete state showing final score.

Verification: Full playable loop - start menu → game with ball rolling on terrain → score accumulates → fall through gap shows GAME OVER → Enter restarts → reach goal shows final score.

Key Technical Notes
Game Loop Pattern

ftxui::Loop loop(&screen_, component);
while (!loop.HasQuitted()) {
    auto now = steady_clock::now();
    float dt = duration<float>(now - last).count();
    last = now;
    loop.RunOnce();           // Process FTXUI events
    if (playing) {
        input_->poll();
        game_session_->update(dt, input_->snapshot());
    }
    screen_.RequestAnimationFrame();
    sleep_until(now + 16ms);  // ~60 FPS
}
Box2D v3 C API Pattern
All Box2D interaction is behind C++ wrapper classes in physics/. The rest of the codebase never touches b2* types directly. Key functions: b2CreateWorld, b2CreateBody, b2CreateCircleShape, b2CreateDistanceJoint, b2World_Step, b2Body_ApplyTorque, b2Body_ApplyLinearImpulseToCenter.

FTXUI Key-Release Workaround
FTXUI only provides key-press events, not key-release. The keyboard provider uses frame-based detection: key state is cleared each frame. If a key event arrives, it's "held." If it was held last frame but no event arrived this frame, it's "just released." This adds at most 16ms latency.

Threading
Two threads only: main (FTXUI loop + physics + rendering) and STDIN reader. Box2D is not thread-safe, so physics stays on main thread. Gamepad polling uses libgamepad's internal thread.

Coordinate System
Box2D world: meters, Y-up. Screen: braille pixels, Y-down (row 0 = top). Camera handles the flip. Scale: 30 braille pixels per meter.

Deferred (Future Phases)
Phase 8: Parallax layers (stars, background curves, speed lines)
Phase 9: Pause menu, polish
Phase 10: Audio (libmikmod for music, STK for sound effects)