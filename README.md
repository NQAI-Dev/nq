# nq

My small 2D engine in C, written on SDL3. Started 2026-09-14 as a long-term
side project — main focus stays on `nodepulse` (production monitoring), but
this is where I touch graphics, ECS, and game-engine plumbing at my own
pace.

## Status

Pre-alpha. One example program: `nq_hello_window` — opens a 640x480 SDL3
window, clears to cornflower blue, exits on ESC. Roadmap (incremental, no
deadlines):

1. Render loop + sprite blit
2. Input + animation system
3. Scene graph (or ECS — TBD)
4. Audio
5. Asset pipeline + tooling

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

## Why C, why SDL3

- **C** — long-term foundation skill, no runtime overhead, builds
  everywhere. Matches the spirit of projects I want to learn from (Quake,
  DOOM, sdl1.2-era tutorials).
- **SDL3** — current SDL line (released 2024), properties-based API, actual
  return values for errors. SDL2 is legacy and the API differences are
  too wide to dual-target.

## Why "nq"

My initials (NQAI). Short, lowercase, mine. `#include <nq/graphics.h>`
reads well; `github.com/NQAI-Dev/nq` is short enough to type.

## License

TBD — leaning MIT.
