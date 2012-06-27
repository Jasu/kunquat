

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2012
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_GENERATOR_DEBUG_H
#define K_GENERATOR_DEBUG_H


#include <stdint.h>

#include <Generator.h>
#include <Voice_state.h>


typedef struct Generator_debug
{
    Generator parent;
    bool single_pulse;
} Generator_debug;


/**
 * Creates a new Debug Generator.
 *
 * The Debug Generator generates a narrow pulse wave (with one sample value 1,
 * the rest are 0.5) that lasts no more than 10 phase cycles. Note off lasts
 * no more than two phase cycles with all sample values negated.
 *
 * \param buffer_size   The mixing buffer size -- must be > \c 0 and
 *                      <= \c KQT_BUFFER_SIZE_MAX.
 * \param mix_rate      The mixing rate -- must be > \c 0.
 *
 * \return   The new Debug Generator if successful, or \c NULL if memory
 *           allocation failed.
 */
Generator* new_Generator_debug(uint32_t buffer_size,
                               uint32_t mix_rate);


uint32_t Generator_debug_mix(Generator* gen,
                             Voice_state* state,
                             uint32_t nframes,
                             uint32_t offset,
                             uint32_t freq,
                             double tempo);


/**
 * Destroys an existing Debug Generator.
 *
 * \param gen   The Debug Generator, or \c NULL.
 */
void del_Generator_debug(Generator* gen);


#endif // K_GENERATOR_DEBUG_H


