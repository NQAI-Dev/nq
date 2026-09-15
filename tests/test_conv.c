/*
 * Integration tests for tools/nq-conv. We invoke the nq-conv CLI as a
 * subprocess (via popen) and validate the generated header against
 * patterns we expect. nq-conv is SDL3-free so the test executable
 * doesn't need to link any rendering library.
 *
 * Skip the test if nq-conv isn't available at the standard path — for
 * example when the binary hasn't been built yet. The ctest target's
 * dependency on the nq-conv target guarantees this in CI.
 */
#include "test_main.c"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* CMake injects the absolute path to the built nq-conv binary so the
 * test works regardless of which directory ctest runs from (CI typically
 * runs tests from build/, not the source root). */
#ifndef NQ_CONV_BIN
#define NQ_CONV_BIN "tools/nq-conv/nq-conv"
#endif

/* Run `nq-conv <out> <name> <cols> <rows> [tex_w] [tex_h]`, capture stdout
 * (which the tool prints on success), return it in a malloc'd buffer.
 * Caller frees. On failure returns NULL. */
static char *nq_conv_run(int cols, int rows, int tex_w, int tex_h,
                        const char *atlas_name) {
    char cmd[1024];
    if (tex_w > 0 && tex_h > 0) {
        snprintf(cmd, sizeof(cmd),
                 "%s /tmp/nq_conv_test_out.h %s %d %d %d %d",
                 NQ_CONV_BIN, atlas_name, cols, rows, tex_w, tex_h);
    } else {
        snprintf(cmd, sizeof(cmd),
                 "%s /tmp/nq_conv_test_out.h %s %d %d",
                 NQ_CONV_BIN, atlas_name, cols, rows);
    }
    /* The tool writes the generated header to the path passed as argv[1];
     * /dev/null suppresses that file. We capture stdout instead. */
    /* (nq-conv prints "wrote X cells to <path>" on success.) */
    FILE *p = popen(cmd, "r");
    if (!p) return NULL;
    char *out = calloc(1, 4096);
    if (!out) { pclose(p); return NULL; }
    size_t n = fread(out, 1, 4095, p);
    out[n] = '\0';
    pclose(p);
    return out;
}

static int contains(const char *hay, const char *needle) {
    return strstr(hay, needle) != NULL;
}

static void test_conv_basic_output(void) {
    /* 2x2 sprite sheet, named TEST, tex 64x64. Expect 4 cells. */
    char *out = nq_conv_run(2, 2, 64, 64, "TEST");
    NQ_ASSERT(out != NULL);
    /* Sanity-check the header structure. */
    NQ_ASSERT(contains(out, "NQ_ATLAS_TEST_H"));
    NQ_ASSERT(contains(out, "NQ_ATLAS_TEST_TEX_W 64"));
    NQ_ASSERT(contains(out, "NQ_ATLAS_TEST_TEX_H 64"));
    NQ_ASSERT(contains(out, "NQ_ATLAS_TEST_COLS 2"));
    NQ_ASSERT(contains(out, "NQ_ATLAS_TEST_ROWS 2"));
    /* 4 cells, 0..3, each with _X _Y _W _H */
    NQ_ASSERT(contains(out, "NQ_ATLAS_TEST_REGION_0_X"));
    NQ_ASSERT(contains(out, "NQ_ATLAS_TEST_REGION_0_Y"));
    NQ_ASSERT(contains(out, "NQ_ATLAS_TEST_REGION_0_W"));
    NQ_ASSERT(contains(out, "NQ_ATLAS_TEST_REGION_0_H"));
    NQ_ASSERT(contains(out, "NQ_ATLAS_TEST_REGION_3_H"));
    free(out);
}

static void test_conv_64x64_layout(void) {
    /* 8x8 atlas in a 64x64 texture → cells of 8x8. Region (0,0) should
     * have x=0, y=0, w=8, h=8; region (1,0) is column 1 row 0 → x=8, y=0. */
    char *out = nq_conv_run(8, 8, 64, 64, "LAYOUT");
    NQ_ASSERT(out != NULL);
    /* Cell (col=1, row=0) → x=8, y=0 */
    NQ_ASSERT(contains(out, "NQ_ATLAS_LAYOUT_REGION_1_X 8"));
    NQ_ASSERT(contains(out, "NQ_ATLAS_LAYOUT_REGION_1_Y 0"));
    /* Cell (col=0, row=1) → x=0, y=8 */
    NQ_ASSERT(contains(out, "NQ_ATLAS_LAYOUT_REGION_8_X 0"));
    NQ_ASSERT(contains(out, "NQ_ATLAS_LAYOUT_REGION_8_Y 8"));
    /* Last cell (col=7, row=7) → x=56, y=56, w=8, h=8 */
    NQ_ASSERT(contains(out, "NQ_ATLAS_LAYOUT_REGION_63_X 56"));
    NQ_ASSERT(contains(out, "NQ_ATLAS_LAYOUT_REGION_63_Y 56"));
    free(out);
}

static void test_conv_sanitizes_name(void) {
    /* Names with non-alphanumeric chars should be uppercased + underscored.
     * "player-run" → "PLAYER_RUN" / "player.run" → "PLAYER_RUN" */
    char *out = nq_conv_run(1, 1, 32, 32, "player-run");
    NQ_ASSERT(out != NULL);
    NQ_ASSERT(contains(out, "NQ_ATLAS_PLAYER_RUN_H"));
    free(out);

    out = nq_conv_run(1, 1, 32, 32, "player.run");
    NQ_ASSERT(out != NULL);
    NQ_ASSERT(contains(out, "NQ_ATLAS_PLAYER_RUN_H"));
    free(out);
}

static void test_conv_unspecified_tex_size(void) {
    /* If tex_w/tex_h are 0 (or omitted), the tool emits them as 0 and
     * doesn't pre-compute region rects. The output should still be a
     * well-formed header. */
    char *out = nq_conv_run(4, 4, 0, 0, "NOTEX");
    NQ_ASSERT(out != NULL);
    NQ_ASSERT(contains(out, "NQ_ATLAS_NOTEX_TEX_W 0"));
    NQ_ASSERT(contains(out, "NQ_ATLAS_NOTEX_TEX_H 0"));
    free(out);
}

static void test_conv_one_cell_at_origin(void) {
    /* 1x1 atlas always has cell (0,0) at origin regardless of texture
     * size — useful sanity check that the formula handles small grids. */
    char *out = nq_conv_run(1, 1, 16, 32, "ONE");
    NQ_ASSERT(out != NULL);
    NQ_ASSERT(contains(out, "NQ_ATLAS_ONE_REGION_0_X 0"));
    NQ_ASSERT(contains(out, "NQ_ATLAS_ONE_REGION_0_Y 0"));
    NQ_ASSERT(contains(out, "NQ_ATLAS_ONE_REGION_0_W 16"));
    NQ_ASSERT(contains(out, "NQ_ATLAS_ONE_REGION_0_H 32"));
    free(out);
}

/* Always register — if nq-conv binary is missing, popen fails and the
 * tests below NQ_ASSERT on NULL output. CI builds nq-conv as a test
 * dependency via add_subdirectory(tools/nq-conv), so it's always there. */
NQ_TEST_REGISTER("conv_basic_output",       test_conv_basic_output);
NQ_TEST_REGISTER("conv_64x64_layout",        test_conv_64x64_layout);
NQ_TEST_REGISTER("conv_sanitizes_name",     test_conv_sanitizes_name);
NQ_TEST_REGISTER("conv_unspecified_tex_size", test_conv_unspecified_tex_size);
NQ_TEST_REGISTER("conv_one_cell_at_origin", test_conv_one_cell_at_origin);
