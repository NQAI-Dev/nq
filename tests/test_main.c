/*
 * nq — minimal unit-test runner.
 *
 * Pure C, no external deps beyond <assert.h>-style macros defined inline.
 * Each test_*.c registers tests via NQ_TEST_REGISTER, which uses
 * __attribute__((constructor)) so registration happens before main().
 *
 * Run all:                ./nq_test
 * Run filtered by name:   ./nq_test vec2i_add
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NQ_TEST_MAX 256

typedef void (*nq_test_fn)(void);

typedef struct {
    const char *name;
    nq_test_fn  fn;
} nq_test_entry;

static nq_test_entry nq_test_table[NQ_TEST_MAX];
static int nq_test_count = 0;

#define NQ_TEST_REGISTER(name, fn)                                          \
    __attribute__((constructor)) static void nq_test_reg_##fn(void) {       \
        if (nq_test_count < NQ_TEST_MAX) {                                 \
            nq_test_table[nq_test_count].name = (name);                    \
            nq_test_table[nq_test_count].fn   = (fn);                      \
            nq_test_count++;                                                \
        }                                                                   \
    }

#define NQ_ASSERT(cond) do {                                               \
    if (!(cond)) {                                                          \
        fprintf(stderr, "    ASSERT FAIL %s:%d: %s\n",                     \
                __FILE__, __LINE__, #cond);                                \
        return;                                                             \
    }                                                                       \
} while (0)

#define NQ_ASSERT_EQ(a, b) do {                                            \
    long long _a = (long long)(a);                                         \
    long long _b = (long long)(b);                                         \
    if (_a != _b) {                                                         \
        fprintf(stderr, "    ASSERT FAIL %s:%d: %s (%lld) != %s (%lld)\n", \
                __FILE__, __LINE__, #a, _a, #b, _b);                       \
        return;                                                             \
    }                                                                       \
} while (0)

static int nq_test_run(const char *name_filter) {
    int passed = 0;
    int failed = 0;
    for (int i = 0; i < nq_test_count; i++) {
        if (name_filter && strstr(nq_test_table[i].name, name_filter) == NULL) {
            continue;
        }
        printf("  RUN  %-32s ... ", nq_test_table[i].name);
        fflush(stdout);
        int failures_before = failed;
        nq_test_table[i].fn();
        if (failed == failures_before) {
            printf("OK\n");
            passed++;
        } else {
            printf("FAIL\n");
            failed++;
        }
    }
    printf("\n%d passed, %d failed\n", passed, failed);
    return failed > 0 ? 1 : 0;
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    /* Filter by name substring if first arg looks like a filter (no leading
     * dash, not "all"). Default: run everything. */
    const char *filter = NULL;
    if (argc > 1 && argv[1][0] != '-') {
        filter = argv[1];
    }
    return nq_test_run(filter);
}
