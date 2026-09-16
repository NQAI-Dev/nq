# Changelog

All notable changes to `nq` are documented here. The format is based
on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased] — 2026-09-15 → 2026-09-16

### Phase 6 closed: animation, action/tween, collision sets fully symmetric

#### Animation primitives — every primitive type now has a parallel animator

* `NqAnimRect`  — per-frame NqRect animator (closes the anim set
  with NqAnimVec2 in `0e8c51c`).
* `NqAnimVec2`  — per-frame NqVec2f animator in `c35e695`.
* `nq_action_tween_vec2` — action bridge for NqAnimVec2 in `25e6170`.
* `nq_action_tween_rect` — action bridge for NqAnimRect in `6c2f864`.

#### Collision API — closure + int/float variants for every primitive

* `nq_circle_overlap`        — circle-vs-circle intersection check
  (`fd0014b`). Int version.
* `nq_circle_overlap_f`      — float version (`bee1ecd`).
* `nq_rect_contains_circle`  — rect-vs-circle overlap check (existing).
* `nq_rect_contains_circle_f`— float version (`70b4132`).
* `nq_rect_intersect_circle` — rect-circle overlap rect (`ad1cfd5`).
* `nq_rect_intersect_circle_f` — float version, with header-order fix
  following (`ab4711c` + ordering fix `8d50106`).
* `nq_circle_penetration_vector_f` — circle-circle separation vector
  (the missing primitive for persistent collision response,
  `21e36e8` + math.h include fix `181fcfa`).
* `nq_rect_penetration_vector_f` — rect-as-obstacle separation vector
  (closes the penetration API on the rect side, `01cffa7` with test
  fix `5ca35ed`).

#### Action repeats and the reset API

* `nq_action_repeat_total` — getter for the original iteration count
  (companion to `nq_action_repeat_remaining`) for "iteration N of M"
  UIs, `0be921a`.

#### Input + utility

* `nq_input_clear` — wipes keyboard + mouse state on scene swaps
  (window-focus loss recovery, `3c8b61e` after circuit-round fixes).
* `nq_log_set_level_by_name` — runtime level setter for CLI args / env
  vars (`a72b59c`).

#### Examples

* `examples/action_pipeline.c` — sixth example. First demo of the
  full animation-to-action pipeline: NqAnimVec2 +
  nq_action_tween_vec2 + NqActionManager moving a sprite across the
  screen under the action framework (`cee46bd` + CMakeLists fix
  `4de3913` + README sync `f63c742`).

#### Docs

* `README.md` Status section kept in sync each block of additions:
  helper primitives listed as they shipped (`8b24fcc`, `2ceb632`,
  `0b495d1`).
* `LICENSE` — MIT.
* `CHANGELOG.md` — this file.

### Build / CI

Phase 6 work was verified locally via `gcc -fsyntax-only -I include`.
CI matrix runs on `ubuntu-latest` / `macos-latest` / `windows-latest`
via `.github/workflows/build.yml`.

### Fixed during Phase 6 shipping chain

The day's long commit chain surfaced several latent test bugs that
were fixed in shipping:

* `e467766` — forward-decl `test_text_width_*` so the
  NQ_TEST_REGISTER block can see them.
* `4891b23` — `nq_text_width` impl + pre-existing `void == (void)0` bug
  in test_text.c.
* `b7db133` / `15863e8` / `181fcfa` / `5ca35ed` — function-ordering
  fixes when test bodies referenced functions declared later in the
  file, plus missing `<math.h>` for sqrtf.

---

## Phase 6 definition

Phase 6 = "tooling and examples". Five traditional examples were
shipped before this release cycle:

* `nq_hello_window`       — bouncing colored square
* `nq_animated_square`    — kitchen-sink of Phase 2 + Phase 3
* `nq_controlled_square`  — input-driven with friction
* `nq_parallax`           — three-layer parallax scroll
* `nq_animatedsprite`     — `NqAnimFloat` cycling sprite

Tooling:

* `tools/nq-conv/`         — sprite-sheet → `NqAtlas` C header
  generator (`714f8ea` + CMakeLists in `3b700c9` + CI fixture
  path via `NQ_CONV_BIN` macro in `3b700c9`, with broken
  path-fix `c544a27`).

Phases 1, 2, 3 closed earlier — see README's Status section.
