

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/events/Event_channel_decl.h>

#include <debug/assert.h>
#include <kunquat/limits.h>
#include <player/Channel.h>
#include <player/events/Event_common.h>
#include <player/events/Event_params.h>
#include <Value.h>

#include <stdbool.h>
#include <stdlib.h>


bool Event_channel_set_au_input_process(
        Channel* ch,
        Device_states* dstates,
        const Master_params* master_params,
        const Event_params* params)
{
    rassert(ch != NULL);
    rassert(dstates != NULL);
    rassert(master_params != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_INT);

    ch->au_input = (int)params->arg->value.int_type;

    return true;
}


