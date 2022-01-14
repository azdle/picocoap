/* Generated Modality mutator header file for 'SEND_SKIP' */

#ifndef SEND_SKIP_H
#define SEND_SKIP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const uint64_t HASH_SEND_SKIP_send_skip;

extern void * const g_generated_state_SEND_SKIP;

void mutate_SEND_SKIP(
        modality_probe * const probe,
        void * const opaque_state,
        const uint64_t instrumentation_hash,
        void * const data,
        const size_t data_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SEND_SKIP_H */
