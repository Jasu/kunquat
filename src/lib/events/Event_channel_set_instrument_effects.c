

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011
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

#include <Channel.h>
#include <Channel_state.h>
#include <Event_common.h>
#include <Event_channel_set_instrument_effects.h>
#include <File_base.h>
#include <kunquat/limits.h>
#include <xassert.h>
#include <xmemory.h>


static Event_field_desc set_instrument_effects_desc[] =
{
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_constructor(Event_channel,
                         EVENT_CHANNEL_SET_INSTRUMENT_EFFECTS,
                         set_instrument_effects);


bool Event_channel_set_instrument_effects_process(Channel_state* ch_state,
                                                  char* fields)
{
    assert(ch_state != NULL);
    (void)fields;
    ch_state->inst_effects = true;
    return true;
}

