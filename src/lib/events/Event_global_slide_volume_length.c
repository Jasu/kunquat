

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
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
#include <limits.h>
#include <math.h>

#include <Event_common.h>
#include <Event_global_slide_volume_length.h>
#include <kunquat/limits.h>
#include <Value.h>
#include <xassert.h>


bool Event_global_slide_volume_length_process(
        Playdata* global_state,
        Value* value)
{
    assert(global_state != NULL);
    assert(value != NULL);
    if (value->type != VALUE_TYPE_TSTAMP)
    {
        return false;
    }
    Slider_set_mix_rate(&global_state->volume_slider, global_state->freq);
    Slider_set_tempo(&global_state->volume_slider, global_state->tempo);
    Slider_set_length(&global_state->volume_slider,
                      &value->value.Tstamp_type);
    return true;
}


