#ifndef NQ_PATH_H
#define NQ_PATH_H

#include <stdbool.h>
#include <stddef.h>

#include "nq/vec.h"
#include "nq/color.h"
#include "nq/rect.h"
#include "nq/graphics.h"

//
// A* Pathfinding scaffold.
// Uses an opaque grid for testing. Grid size and walkability are defined externally.
//

#define NQ_PATH_MAX_NODES 1024
#define NQ_PATH_MAX_RESULT 256

typedef struct NqPathNode {
    NqVec2i pos;
    int g_score;
    int h_score;
    int f_score;
    struct NqPathNode* parent;
    bool closed;
    bool open;
} NqPathNode;

typedef struct NqPathGrid {
    int width;
    int height;
    bool* walkable;
} NqPathGrid;

typedef struct NqPathResult {
    NqVec2i path[NQ_PATH_MAX_RESULT];
    int length;
    bool found;
} NqPathResult;

// Returns Manhattan distance between a and b.
int nq_path_heuristic_manhattan(NqVec2i a, NqVec2i b);

// Finds a path from start to goal in the given grid.
// Returns a NqPathResult containing the path points in order (start to goal)
// or found=false if no path exists or bounds are invalid.
NqPathResult nq_path_find_astar(const NqPathGrid* grid, NqVec2i start, NqVec2i goal);

#endif
