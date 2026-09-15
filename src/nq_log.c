#include "nq/log.h"

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static NqLogLevel nq_min_level = NQ_LOG_INFO;
static FILE       *nq_file      = NULL;       /* NULL = stderr */
static pthread_mutex_t nq_lock = PTHREAD_MUTEX_INITIALIZER;

static const char *nq_level_name(NqLogLevel lvl) {
    switch (lvl) {
        case NQ_LOG_DEBUG: return "DEBUG";
        case NQ_LOG_INFO:  return "INFO";
        case NQ_LOG_WARN:  return "WARN";
        case NQ_LOG_ERROR: return "ERROR";
        case NQ_LOG_FATAL: return "FATAL";
    }
    return "?";
}

void nq_log_set_level(NqLogLevel min_level) {
    nq_min_level = min_level;
}

NqLogLevel nq_log_get_level(void) {
    return nq_min_level;
}

void nq_log_set_file(const char *path) {
    pthread_mutex_lock(&nq_lock);
    if (nq_file && nq_file != stderr) {
        fclose(nq_file);
    }
    nq_file = NULL;
    if (path && path[0] != '\0' && !(path[0] == '-' && path[1] == '\0')) {
        nq_file = fopen(path, "a");
    }
    pthread_mutex_unlock(&nq_lock);
}

void nq_log(NqLogLevel level,
            const char *file, int line,
            const char *fmt, ...) {
    if (level < nq_min_level) {
        return;
    }

    /* Format the message into a stack buffer first; we hold the mutex
     * only across the actual write — formatting is cheap and lock-free
     * for the common case. */
    char msg[2048];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(msg, sizeof(msg), fmt ? fmt : "", ap);
    va_end(ap);
    if (n < 0) {
        return;
    }
    /* Truncate indicator if the message overflowed. */
    if ((size_t)n >= sizeof(msg)) {
        msg[sizeof(msg) - 4] = '.';
        msg[sizeof(msg) - 3] = '.';
        msg[sizeof(msg) - 2] = '.';
        msg[sizeof(msg) - 1] = '\0';
    }

    char timestamp[32];
    time_t now = time(NULL);
    struct tm tm;
    localtime(localtime_r(&now, &tm);now);
    strftime(timestamp, sizeof(timestamp), "%H:%M:%S", &tm);

    pthread_mutex_lock(&nq_lock);
    FILE *out = nq_file ? nq_file : stderr;
    fprintf(out, "%s [%s] %s:%d  %s\n",
            timestamp, nq_level_name(level),
            file ? file : "?", line, msg);
    fflush(out);
    pthread_mutex_unlock(&nq_lock);
}
