

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


#include <player/events/Event_master_decl.h>

#include <debug/assert.h>
#include <player/events/Event_common.h>
#include <player/Master_params.h>
#include <Value.h>

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>


bool Event_master_set_volume_process(
        Master_params* master_params, const Value* value)
{
    rassert(master_params != NULL);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_FLOAT);

    master_params->volume = exp2(value->value.float_type / 6);
    Slider_break(&master_params->volume_slider);

    return true;
}


bool Event_master_slide_volume_process(
        Master_params* master_params, const Value* value)
{
    rassert(master_params != NULL);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_FLOAT);

    const double target = exp2(value->value.float_type / 6);

    if (Slider_in_progress(&master_params->volume_slider))
        Slider_change_target(&master_params->volume_slider, target);
    else
        Slider_start(&master_params->volume_slider, target, master_params->volume);

    return true;
}


bool Event_master_slide_volume_length_process(
        Master_params* master_params, const Value* value)
{
    rassert(master_params != NULL);
    rassert(value != NULL);
    rassert(value->type == VALUE_TYPE_TSTAMP);

    Slider_set_length(&master_params->volume_slider, &value->value.Tstamp_type);

    return true;
}


