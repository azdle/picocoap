/* Generated Modality mutator implementation for 'SEND_SKIP' */

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "modality/probe.h"
#include "modality/mutation_interface.h"

void *memcpy(void *dest, const void *src, size_t n);

typedef struct g_generated_state_s
{
    bool is_registered;
    bool is_injected;
    bool send_skip;
} g_generated_state_s;

static g_generated_state_s g_generated_state =
{
    .is_registered = false,
    .is_injected = false,
    .send_skip = false,
};

void * const g_generated_state_SEND_SKIP = &g_generated_state;

static const modality_mutation_parameter_definition PARAM_DEFS_SEND_SKIP[1] =
{
    [0] =
    {
        .param_type =
        {
            .tag = MUTATION_PARAM_TYPE_BOOLEAN,
            .body.boolean =
            {
                .minimum_effect_value = false,
            },
        },
        .name = "send-skip",
    },
};

const uint64_t HASH_SEND_SKIP_send_skip = 3774990707475861396ULL;

static const modality_mutation_definition MUTATOR_DEF_SEND_SKIP =
{
    .name = "send-skip",
    .params = PARAM_DEFS_SEND_SKIP,
    .params_length = 1,
    .tags = NULL,
    .tags_length = 0,
};

static size_t clear_mutations(
        void * const fi)
{
    assert(fi != NULL);
    g_generated_state_s * const state = (g_generated_state_s*) fi;
    state->is_injected = false;
    return 0;
}

static size_t get_definition(
        void * const fi,
        const modality_mutation_definition ** const definition)
{
    assert(fi != NULL);
    (*definition) = &MUTATOR_DEF_SEND_SKIP;
    return 0;
}

static size_t inject_mutation(
        void * const fi,
        const modality_mutation_param * const params,
        const size_t params_length)
{
    assert(fi != NULL);
    g_generated_state_s * const state = (g_generated_state_s*) fi;
    assert(params != NULL);
    assert(params_length == 1);

    assert(params[0].tag == MUTATION_PARAM_BOOLEAN);
    state->send_skip = params[0].body.boolean.value;

    state->is_injected = true;
    return 0;
}

static const modality_mutation_interface SEND_SKIP =
{
    .state = (void*) &g_generated_state,
    .inject_mutation = &inject_mutation,
    .get_definition = &get_definition,
    .clear_mutations = &clear_mutations,
};

void mutate_SEND_SKIP(
        modality_probe * const probe,
        void * const opaque_state,
        const uint64_t instrumentation_hash,
        void * const data,
        const size_t data_size)
{
    assert(probe != NULL);
    assert(opaque_state != NULL);
    assert(instrumentation_hash != 0);
    assert(data != NULL);
    assert(data_size != 0);

    g_generated_state_s * const state = (g_generated_state_s*) opaque_state;
    assert(state == &g_generated_state);

    if(state->is_registered == false)
    {
        state->is_registered = true;

        const size_t err = modality_probe_register_mutator(
                probe,
                &SEND_SKIP);
        assert(err == MODALITY_PROBE_ERROR_OK);
    }

    if(state->is_injected == true)
    {
        if(instrumentation_hash == HASH_SEND_SKIP_send_skip)
        {
            assert(data_size == sizeof(state->send_skip));
            (void) memcpy(data, &state->send_skip, data_size);
        }
        else
        {
            assert(false); /* Got an unrecognized hash */
        }
    }
}
