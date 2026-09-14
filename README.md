# nq

My small 2D engine in C, written on SDL3. Started 2026-09-14 as a long-term
side project — main focus stays on `nodepulse` (production monitoring), but
this is where I touch graphics, ECS, and game-engine plumbing at my own
pace.

## Status

Pre-alpha. Two example programs: `nq_hello_window` — opens a 640x480 SDL3
window, clears to dark blue, draws a single bouncing colored square that
bounces off the edges. SDL3 is wired through the public engine API; the
example never calls SDL3 directly.

Phase 1 (engine core primitives) is in progress.

## Roadmap

Phases are sequential — each delivers something runnable. Within a phase,
work is incremental via 30-min cron ticks.

### Phase 0 — scaffold ✅
- `examples/hello_window.c` — minimal SDL3 hello window
- `CMakeLists.txt` — `find_package(SDL3)`, `-Wall -Wextra -Wpedantic -Werror`
- `README.md`, `.gitignore`

### Phase 1 — engine core (in progress)
Pure-C geometric primitives and basic subsystems, no SDL3 dependency on the
math side so they're unit-testable in isolation. Each lands behind a public
header in `include/nq/`.

- `nq_vec2i` — 2D integer vector, add/sub/scale/eq
- `nq_vec2f` — same, float
- `nq_rect` — integer rect, contains/intersects/intersection/union/inflate
- `nq_color` — RGBA byte, already in `include/nq/graphics.h`
- `nq_log` — leveled logging (INFO/WARN/ERROR), thread-safe, stderr/file
- `nq_input` — keyboard + mouse state machine, debounced edges
- `nq_texture` — load BMP/PNG via SDL3_image, blit + region blit
- `nq_assets` — virtual filesystem (zip / embedded blobs), cache
- Test infra: `tests/test_main.c` runner + per-module `tests/test_*.c`,
  `assert.h` only (no external deps), CMake `nq_test` target + ctest

### Phase 2 — animation + timing
- `nq_clock` — high-res timer, delta-time, fixed-step / variable-step modes
- `nq_tween` — value lerpers (linear, ease-in/out, cubic, elastic)
- `nq_animation` — track-based, target object + duration
- `nq_atlas` — texture atlas, bin-packing or fixed-grid, runtime region by name

### Phase 3 — scene graph + scheduling
- `nq_node` — tree structure, transform2d, lifecycle (init/update/draw/destroy)
- `nq_scene` — root scene, layering, scene stack (push/pop for pauses)
- `nq_action` — scheduler with priorities. Tween / CallFunc / Repeat /
  Sequence / Spawn (cocos2d-x inspired)

### Phase 4 — 2D physics
- `nq_physics2d` — AABB + circles + oriented boxes (SAT if I get there)
- Integrator: semi-implicit Euler, fixed timestep 1/60s
- Broadphase: sweep-and-prune or spatial hash
- Collision callbacks: begin / overlap / end with mask filters

### Phase 5 — audio
- `nq_audio` over SDL3_audio or miniaudio
- Mix bus (master + SFX + music), per-source pitch/volume, sound pooling

### Phase 6 — tooling & examples
- Examples: animatedsprite, parallax scrolldemo, tiny arcanoid to demo the stack
- CI: GitHub Actions on ubuntu-latest + macos-latest + windows-latest
- Benchmarks via a hand-rolled `nq_bench.h`
- `tools/nq-conv` — sprite sheet → nq_atlas format converter

## Architectural principles

- **C11, zero warnings.** `-Wall -Wextra -Wpedantic -Werror` for own code. SDL3
  headers get `-Wno-pedantic` (GNU extensions upstream).
- **Static lib `nq_core` + public headers in `include/nq/`.** Consumers link to
  the core, never see SDL3 directly.
- **Opaque structs** (`NqRenderer`, `NqTexture`, `NqNode`) so a future
  backend swap (e.g. software renderer for tests) doesn't touch callers.
- **Memory: arenas per frame / per scene tick.** Allocations inside a tick are
  freed in bulk at the end — no malloc-spam in the hot loop. Borrowed from
  the Jai / Odin way of thinking.
- **Errors via return codes.** `<nq/common.h>` with `NQ_OK` / `NQ_ERR_*`. No
  exceptions, no longjmp.
- **No globals.** Engine context passed as `NqContext*` to all API.

## Out of scope (YAGNI)

- 3D rendering — different pipeline, separate project if I ever want it.
- Editor / IDE — competes with Godot / Defold, not my lane.
- Networking — if needed, separate `nq_net` lib on top of the event loop.
- Scripting (Lua / WASM) — adds API and build complexity, only if needed later.

## Build

```sh
cmake -S . -B build
cmake --build build
./build/nq_hello_window
```

Requires SDL3 dev headers. On Debian/Ubuntu:

```sh
sudo apt install libsdl3-dev cmake
```

## Tests

```sh
cmake -S . -B build -DNQ_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## Why C, why SDL3, why "nq"

- **C** — long-term foundation skill, no runtime overhead, builds
  everywhere.
- **SDL3** — current SDL line (released 2024), properties-based API, actual
  return values for errors.
- **"nq"** — my initials (NQAI). `#include <nq/graphics.h>` reads well;
  `github.com/NQAI-Dev/nq` is short enough to type.

## License

MIT.
