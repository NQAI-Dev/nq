#include "nq/path.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

int nq_path_heuristic_manhattan(NqVec2i a, NqVec2i b) {
    int dx = a.x - b.x;
    int dy = a.y - b.y;
    return (dx > 0 ? dx : -dx) + (dy > 0 ? dy : -dy);
}

static bool is_valid_pos(const NqPathGrid* grid, NqVec2i pos) {
    if (pos.x < 0 || pos.y < 0 || pos.x >= grid->width || pos.y >= grid->height) {
        return false;
    }
    return grid->walkable[pos.y * grid->width + pos.x];
}

static NqPathNode* get_node(NqPathNode* nodes, int width, NqVec2i pos) {
    return &nodes[pos.y * width + pos.x];
}

NqPathResult nq_path_find_astar(const NqPathGrid* grid, NqVec2i start, NqVec2i goal) {
    NqPathResult result = {0};
    result.found = false;

    if (!grid || !grid->walkable) {
        return result;
    }

    if (!is_valid_pos(grid, start) || !is_valid_pos(grid, goal)) {
        return result;
    }

    if (start.x == goal.x && start.y == goal.y) {
        result.found = true;
        result.length = 1;
        result.path[0] = start;
        return result;
    }

    int node_count = grid->width * grid->height;
    if (node_count > NQ_PATH_MAX_NODES) {
        return result; // Grid too large for fixed scaffold buffer
    }

    NqPathNode nodes[NQ_PATH_MAX_NODES];
    memset(nodes, 0, sizeof(NqPathNode) * (size_t)node_count);

    for (int y = 0; y < grid->height; ++y) {
        for (int x = 0; x < grid->width; ++x) {
            int idx = y * grid->width + x;
            nodes[idx].pos.x = x;
            nodes[idx].pos.y = y;
            nodes[idx].f_score = INT_MAX;
            nodes[idx].g_score = INT_MAX;
        }
    }

    NqPathNode* open_list[NQ_PATH_MAX_NODES];
    int open_count = 0;

    NqPathNode* start_node = get_node(nodes, grid->width, start);
    start_node->g_score = 0;
    start_node->h_score = nq_path_heuristic_manhattan(start, goal);
    start_node->f_score = start_node->h_score;
    start_node->open = true;

    open_list[open_count++] = start_node;

    NqVec2i dirs[4] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};

    while (open_count > 0) {
        int best_idx = 0;
        for (int i = 1; i < open_count; ++i) {
            if (open_list[i]->f_score < open_list[best_idx]->f_score) {
                best_idx = i;
            }
        }

        NqPathNode* current = open_list[best_idx];

        if (current->pos.x == goal.x && current->pos.y == goal.y) {
            // Reconstruct path
            NqPathNode* curr = current;
            int len = 0;
            while (curr != NULL) {
                len++;
                curr = curr->parent;
            }

            if (len > NQ_PATH_MAX_RESULT) {
                return result; // Path too long
            }

            result.found = true;
            result.length = len;
            curr = current;
            for (int i = len - 1; i >= 0; --i) {
                result.path[i] = curr->pos;
                curr = curr->parent;
            }
            return result;
        }

        current->open = false;
        current->closed = true;
        open_list[best_idx] = open_list[--open_count];

        for (int i = 0; i < 4; ++i) {
            NqVec2i neighbor_pos = {current->pos.x + dirs[i].x, current->pos.y + dirs[i].y};

            if (!is_valid_pos(grid, neighbor_pos)) {
                continue;
            }

            NqPathNode* neighbor = get_node(nodes, grid->width, neighbor_pos);
            if (neighbor->closed) {
                continue;
            }

            int tentative_g_score = current->g_score + 1; // uniform cost for 4-way movement

            if (!neighbor->open) {
                neighbor->open = true;
                open_list[open_count++] = neighbor;
            } else if (tentative_g_score >= neighbor->g_score) {
                continue;
            }

            neighbor->parent = current;
            neighbor->g_score = tentative_g_score;
            neighbor->h_score = nq_path_heuristic_manhattan(neighbor->pos, goal);
            neighbor->f_score = neighbor->g_score + neighbor->h_score;
        }
    }

    return result; // No path found
}
