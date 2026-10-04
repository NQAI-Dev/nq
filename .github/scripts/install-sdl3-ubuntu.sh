#!/usr/bin/env bash
set -euo pipefail

SDL_VERSION=3.4.18
SDL_SOURCE="${RUNNER_TEMP:-/tmp}/SDL"
SDL_BUILD="${RUNNER_TEMP:-/tmp}/SDL-build"

sudo apt-get update
sudo apt-get install -y build-essential cmake ninja-build
git clone --depth 1 --branch "release-${SDL_VERSION}" \
    https://github.com/libsdl-org/SDL.git "$SDL_SOURCE"
cmake -S "$SDL_SOURCE" -B "$SDL_BUILD" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release -DSDL_TESTS=OFF -DSDL_EXAMPLES=OFF
cmake --build "$SDL_BUILD" --parallel 2
sudo cmake --install "$SDL_BUILD"
sudo ldconfig
