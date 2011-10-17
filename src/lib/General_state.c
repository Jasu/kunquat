

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2011
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>

#include <General_state.h>
#include <xassert.h>


General_state* General_state_init(General_state* state, bool global)
{
    assert(state != NULL);
    state->global = global;
    state->pause = false;
    state->cond_exec_enabled = false;
    state->cond_for_exec = false;
    state->evaluated_cond = false;
    return state;
}


bool General_state_events_enabled(General_state* state)
{
    assert(state != NULL);
    return !state->cond_exec_enabled ||
           (state->cond_for_exec == state->evaluated_cond);
}


