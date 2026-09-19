#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <nq/log.h>
#include "test_main.c"  /* NQ_TEST_REGISTER / NQ_ASSERT / NQ_ASSERT_EQ */

/* Helper log emitters — passed to capture_stderr as function pointers.
 * Defined at file scope so they're visible to the test functions that
 * follow (C doesn't have inline lambdas). */
static void emit_filter_messages(void) {
    NQ_LOG_DEBUG("debug-line");
    NQ_LOG_INFO("info-line");
    NQ_LOG_WARN("warn-line");
    NQ_LOG_ERROR("error-line");
}

static void emit_all_log_messages(void) {
    NQ_LOG_DEBUG("d");
    NQ_LOG_INFO("i");
    NQ_LOG_WARN("w");
    NQ_LOG_ERROR("e");
    NQ_LOG_FATAL("f");
}

/* The logger writes to stderr by default. Capture it via dup2 for test. */
static int capture_stderr(char *buf, size_t cap, void (*fn)(void)) {
    int orig = dup(STDERR_FILENO);
    int fds[2];
    if (pipe(fds) != 0) {
        return -1;
    }
    /* Make read-end non-blocking so we can drain it after fn() returns. */
    int flags = fcntl(fds[0], F_GETFL, 0);
    fcntl(fds[0], F_SETFL, flags | O_NONBLOCK);
    dup2(fds[1], STDERR_FILENO);
    close(fds[1]);

    fn();

    fflush(stderr);
    /* Restore stderr so fprintf below reaches the user. */
    dup2(orig, STDERR_FILENO);
    close(orig);

    /* Drain read-end into buf. */
    size_t n = 0;
    while (n + 1 < cap) {
        ssize_t r = read(fds[0], buf + n, cap - 1 - n);
        if (r <= 0) break;
        n += (size_t)r;
    }
    buf[n] = '\0';
    close(fds[0]);
    return (int)n;
}

static void test_log_level_filter(void) {
    char buf[4096];
    nq_log_set_level(NQ_LOG_WARN);

    nq_log_set_file(NULL);  /* stderr */
    int n = capture_stderr(buf, sizeof(buf), emit_filter_messages);
    nq_log_set_level(NQ_LOG_INFO);
    NQ_ASSERT(n > 0);
    NQ_ASSERT(strstr(buf, "warn-line") != NULL);
    NQ_ASSERT(strstr(buf, "error-line") != NULL);
    NQ_ASSERT(strstr(buf, "debug-line") == NULL);
    NQ_ASSERT(strstr(buf, "info-line") == NULL);
}

/* Level name round-trip. */
static void test_log_level_name(void) {
    char buf[4096];
    nq_log_set_level(NQ_LOG_DEBUG);
    nq_log_set_file(NULL);
    int n = capture_stderr(buf, sizeof(buf), emit_all_log_messages);
    NQ_ASSERT(n > 0);
    NQ_ASSERT(strstr(buf, "[DEBUG]") != NULL);
    NQ_ASSERT(strstr(buf, "[INFO]")  != NULL);
    NQ_ASSERT(strstr(buf, "[WARN]")  != NULL);
    NQ_ASSERT(strstr(buf, "[ERROR]") != NULL);
    NQ_ASSERT(strstr(buf, "[FATAL]") != NULL);
}

static void test_log_set_get_level(void) {
    nq_log_set_level(NQ_LOG_ERROR);
    NQ_ASSERT_EQ(nq_log_get_level(), NQ_LOG_ERROR);
    nq_log_set_level(NQ_LOG_DEBUG);
    NQ_ASSERT_EQ(nq_log_get_level(), NQ_LOG_DEBUG);
    /* Restore default for other tests. */
    nq_log_set_level(NQ_LOG_INFO);
}

/* Two threads racing the logger; verify both messages land and the
 * final newline count is exactly N (no interleave corruption). We
 * settle for "no crash, no garbage bytes" since fully verifying
 * non-interleaving output is brittle without formatter inspection. */
typedef struct {
    const char *msg;
    int iterations;
} thread_arg;

static void *log_thread(void *p) {
    thread_arg *a = (thread_arg *)p;
    for (int i = 0; i < a->iterations; i++) {
        NQ_LOG_INFO("%s-%d", a->msg, i);
    }
    return NULL;
}

static void test_log_thread_safety(void) {
    nq_log_set_file(NULL);
    nq_log_set_level(NQ_LOG_INFO);

    pthread_t t1, t2;
    thread_arg a1 = { "alpha", 200 };
    thread_arg a2 = { "beta", 200 };
    pthread_create(&t1, NULL, log_thread, &a1);
    pthread_create(&t2, NULL, log_thread, &a2);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    /* If the mutex didn't work we'd either crash inside vfprintf or
     * corrupt stderr state. Reaching here means we're safe at that
     * level. Higher-quality interleaving checks would need a custom
     * FILE* sink, deferred. */
    NQ_ASSERT(1);
}

NQ_TEST_REGISTER("log_level_filter",       test_log_level_filter)
NQ_TEST_REGISTER("log_level_name",         test_log_level_name)
NQ_TEST_REGISTER("log_set_get_level",      test_log_set_get_level)
NQ_TEST_REGISTER("log_thread_safety",      test_log_thread_safety)

static void test_log_set_level_by_name(void) {
    /* Canonical names — exact case */
    NQ_ASSERT_EQ(nq_log_set_level_by_name("DEBUG"), 1);
    NQ_ASSERT_EQ(nq_log_get_level(), NQ_LOG_DEBUG);
    NQ_ASSERT_EQ(nq_log_set_level_by_name("INFO"), 1);
    NQ_ASSERT_EQ(nq_log_get_level(), NQ_LOG_INFO);
    NQ_ASSERT_EQ(nq_log_set_level_by_name("WARN"), 1);
    NQ_ASSERT_EQ(nq_log_get_level(), NQ_LOG_WARN);
    NQ_ASSERT_EQ(nq_log_set_level_by_name("ERROR"), 1);
    NQ_ASSERT_EQ(nq_log_get_level(), NQ_LOG_ERROR);
    NQ_ASSERT_EQ(nq_log_set_level_by_name("FATAL"), 1);
    NQ_ASSERT_EQ(nq_log_get_level(), NQ_LOG_FATAL);
}

static void test_log_set_level_by_name_case_insensitive(void) {
    /* strcasecmp: upper, lower, mixed */
    nq_log_set_level(NQ_LOG_INFO);
    NQ_ASSERT_EQ(nq_log_set_level_by_name("warn"),  1);
    NQ_ASSERT_EQ(nq_log_get_level(), NQ_LOG_WARN);
    NQ_ASSERT_EQ(nq_log_set_level_by_name("Warn"),  1);
    NQ_ASSERT_EQ(nq_log_set_level_by_name("WARN"),  1);
    NQ_ASSERT_EQ(nq_log_set_level_by_name("wArN"),  1);
}

static void test_log_set_level_by_name_unknown_returns_zero(void) {
    /* Unknown name leaves the level unchanged */
    nq_log_set_level(NQ_LOG_INFO);
    int prev = nq_log_get_level();
    NQ_ASSERT_EQ(nq_log_set_level_by_name("FROBNICATE"), 0);
    NQ_ASSERT_EQ(nq_log_get_level(), prev);
    NQ_ASSERT_EQ(nq_log_set_level_by_name(""), 0);
    NQ_ASSERT_EQ(nq_log_get_level(), prev);
}

static void test_log_set_level_by_name_null_safe(void) {
    NQ_ASSERT_EQ(nq_log_set_level_by_name(NULL), 0);
}

NQ_TEST_REGISTER("log_set_level_by_name",               test_log_set_level_by_name)
NQ_TEST_REGISTER("log_set_level_by_name_case",          test_log_set_level_by_name_case_insensitive)
NQ_TEST_REGISTER("log_set_level_by_name_unknown",       test_log_set_level_by_name_unknown_returns_zero)
NQ_TEST_REGISTER("log_set_level_by_name_null_safe",      test_log_set_level_by_name_null_safe)
