/*
 * nq — shared types and error codes.
 *
 * Included by every public header. Anything that needs an error code pulls
 * NQ_OK / NQ_ERR_* from here so we have one source of truth.
 */
#ifndef NQ_COMMON_H
#define NQ_COMMON_H

#define NQ_OK     0
#define NQ_ERR   -1
/* Specific failure modes can be added as NQ_ERR_<MODULE>_<REASON>
 * once a module has more than one failure mode worth distinguishing. */

#endif /* NQ_COMMON_H */
