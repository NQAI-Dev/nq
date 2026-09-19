#include "nq/path.h"
#include <stdbool.h>
#include "test_main.c"

static void test_heuristic() {
    NQ_ASSERT_EQ(nq_path_heuristic_manhattan((NqVec2i){0,0}, (NqVec2i){5,5}), 10);
    NQ_ASSERT_EQ(nq_path_heuristic_manhattan((NqVec2i){2,3}, (NqVec2i){-1,-2}), 8);
    NQ_ASSERT_EQ(nq_path_heuristic_manhattan((NqVec2i){10,10}, (NqVec2i){10,10}), 0);
}

static void test_astar_straight() {
    bool walkable[25] = {
        true, true, true, true, true,
        true, true, true, true, true,
        true, true, true, true, true,
        true, true, true, true, true,
        true, true, true, true, true
    };
    NqPathGrid grid = {5, 5, walkable};
    
    NqPathResult res = nq_path_find_astar(&grid, (NqVec2i){0,0}, (NqVec2i){4,0});
    NQ_ASSERT(res.found);
    NQ_ASSERT_EQ(res.length, 5);
    NQ_ASSERT_EQ(res.path[0].x, 0);
    NQ_ASSERT_EQ(res.path[0].y, 0);
    NQ_ASSERT_EQ(res.path[4].x, 4);
    NQ_ASSERT_EQ(res.path[4].y, 0);
}

static void test_astar_obstacle() {
    bool walkable[25] = {
        true, true,  true, true, true,
        true, false, true, true, true,
        true, false, true, true, true,
        true, false, true, true, true,
        true, true,  true, true, true
    };
    NqPathGrid grid = {5, 5, walkable};
    
    NqPathResult res = nq_path_find_astar(&grid, (NqVec2i){0,2}, (NqVec2i){2,2});
    NQ_ASSERT(res.found);
    NQ_ASSERT(res.length > 3); 
    NQ_ASSERT_EQ(res.path[0].x, 0);
    NQ_ASSERT_EQ(res.path[0].y, 2);
    NQ_ASSERT_EQ(res.path[res.length - 1].x, 2);
    NQ_ASSERT_EQ(res.path[res.length - 1].y, 2);
}

static void test_astar_no_path() {
    bool walkable[25] = {
        true, false, true, true, true,
        true, false, true, true, true,
        true, false, true, true, true,
        true, false, true, true, true,
        true, false, true, true, true
    };
    NqPathGrid grid = {5, 5, walkable};
    
    NqPathResult res = nq_path_find_astar(&grid, (NqVec2i){0,2}, (NqVec2i){4,2});
    NQ_ASSERT(!res.found);
    NQ_ASSERT_EQ(res.length, 0);
}

static void test_astar_invalid_bounds() {
    bool walkable[4] = {true, true, true, true};
    NqPathGrid grid = {2, 2, walkable};

    NqPathResult res = nq_path_find_astar(&grid, (NqVec2i){-1,0}, (NqVec2i){0,0});
    NQ_ASSERT(!res.found);
    
    res = nq_path_find_astar(&grid, (NqVec2i){0,0}, (NqVec2i){2,2});
    NQ_ASSERT(!res.found);
}

static void test_astar_same_start_goal() {
    bool walkable[4] = {true, true, true, true};
    NqPathGrid grid = {2, 2, walkable};

    NqPathResult res = nq_path_find_astar(&grid, (NqVec2i){1,1}, (NqVec2i){1,1});
    NQ_ASSERT(res.found);
    NQ_ASSERT_EQ(res.length, 1);
    NQ_ASSERT_EQ(res.path[0].x, 1);
    NQ_ASSERT_EQ(res.path[0].y, 1);
}

NQ_TEST_REGISTER("path_heuristic", test_heuristic)
NQ_TEST_REGISTER("path_astar_straight", test_astar_straight)
NQ_TEST_REGISTER("path_astar_obstacle", test_astar_obstacle)
NQ_TEST_REGISTER("path_astar_no_path", test_astar_no_path)
NQ_TEST_REGISTER("path_astar_invalid_bounds", test_astar_invalid_bounds)
NQ_TEST_REGISTER("path_astar_same_start_goal", test_astar_same_start_goal)
