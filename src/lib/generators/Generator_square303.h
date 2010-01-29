

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_GENERATOR_SQUARE303_H
#define K_GENERATOR_SQUARE303_H


#include <Generator.h>
#include <Voice_state.h>


typedef struct Generator_square303
{
    Generator parent;
} Generator_square303;


/**
 * Creates a new Square303 Generator.
 *
 * \param ins_params   The Instrument parameters -- must not be \c NULL.
 *
 * \return   The new Square303 Generator if successful, or \c NULL if memory
 *           allocation failed.
 */
Generator* new_Generator_square303(Instrument_params* ins_params);


uint32_t Generator_square303_mix(Generator* gen,
                                 Voice_state* state,
                                 uint32_t nframes,
                                 uint32_t offset,
                                 uint32_t freq,
                                 double tempo,
                                 int buf_count,
                                 kqt_frame** bufs);


/**
 * Destroys an existing Square303 Generator.
 *
 * \param gen   The Square303 Generator -- must not be \c NULL.
 */
void del_Generator_square303(Generator* gen);


#endif // K_GENERATOR_SQUARE303_H

