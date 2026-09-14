/*
 * nq — leveled logging.
 *
 * Thread-safe via a single internal mutex. Output goes to stderr only for
 * now; file sink is deferred until something actually needs rotated logs.
 *
 * Usage:
 *   nq_log_set_level(NQ_LOG_INFO);
 *   nq_log_set_file("/tmp/nq.log");   // optional, NULL = stderr
 *   NQ_LOG_INFO("hello %s", "world");
 *
 * Each macro captures __FILE__ and __LINE__ automatically.
 */
#ifndef NQ_LOG_H
#define NQ_LOG_H

typedef enum {
    NQ_LOG_DEBUG = 0,
    NQ_LOG_INFO  = 1,
    NQ_LOG_WARN  = 2,
    NQ_LOG_ERROR = 3,
    NQ_LOG_FATAL = 4,
} NqLogLevel;

void nq_log_set_level(NqLogLevel min_level);
NqLogLevel nq_log_get_level(void);
void nq_log_set_file(const char *path);  /* NULL or "-" = stderr (default) */

void nq_log(NqLogLevel level,
            const char *file, int line,
            const char *fmt, ...);

/* Convenience macros — auto-capture __FILE__ and __LINE__. */
#define NQ_LOG_DEBUG(...) nq_log(NQ_LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define NQ_LOG_INFO(...)  nq_log(NQ_LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define NQ_LOG_WARN(...)  nq_log(NQ_LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define NQ_LOG_ERROR(...) nq_log(NQ_LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define NQ_LOG_FATAL(...) nq_log(NQ_LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#endif /* NQ_LOG_H */
