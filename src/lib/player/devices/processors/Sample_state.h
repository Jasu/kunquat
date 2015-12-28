

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_SAMPLE_STATE_H
#define K_SAMPLE_STATE_H


#include <decl.h>

#include <stdlib.h>


size_t Sample_vstate_get_size(void);

void Sample_vstate_init(Voice_state* vstate, const Proc_state* proc_state);


#endif // K_SAMPLE_STATE_H


