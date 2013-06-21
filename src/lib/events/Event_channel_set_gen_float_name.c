

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdbool.h>

#include <Active_names.h>
#include <Event.h>
#include <Event_common.h>
#include <Event_channel_set_gen_float_name.h>
#include <set_active_name.h>
#include <Value.h>
#include <xassert.h>


bool Event_channel_set_gen_float_name_process(
        Channel_state* ch_state,
        Value* value)
{
    assert(ch_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_STRING)
    {
        return false;
    }
    return set_active_name(
            &ch_state->parent,
            ACTIVE_CAT_CH_GEN,
            ACTIVE_TYPE_FLOAT,
            value);
}


