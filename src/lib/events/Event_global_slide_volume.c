

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


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>

#include <Event_common.h>
#include <Event_global_slide_volume.h>
#include <Reltime.h>

#include <xmemory.h>


static Event_field_desc slide_volume_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .min.field.double_type = -INFINITY,
        .max.field.double_type = 0
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


Event_create_set_primitive_and_get(Event_global_slide_volume,
                                   EVENT_GLOBAL_SLIDE_VOLUME,
                                   double, target_volume_dB)


static void Event_global_slide_volume_process(Event_global* event, Playdata* play);


Event_create_constructor(Event_global_slide_volume,
                         EVENT_GLOBAL_SLIDE_VOLUME,
                         slide_volume_desc,
                         event->target_volume_dB = 0)


bool Event_global_slide_volume_handle(Playdata* global_state, char* fields)
{
    assert(global_state != NULL);
    if (fields == NULL)
    {
        return false;
    }
    Event_field data[1];
    Read_state* state = READ_STATE_AUTO;
    Event_type_get_fields(fields, slide_volume_desc, data, state);
    if (state->error)
    {
        return false;
    }
    global_state->volume_slide_target = exp2(data[0].field.double_type / 6);
    global_state->volume_slide_frames =
            Reltime_toframes(&global_state->volume_slide_length,
                             global_state->tempo,
                             global_state->freq);
    double volume_dB = log2(global_state->volume) * 6;
    double dB_step = (data[0].field.double_type - volume_dB) /
                     global_state->volume_slide_frames;
    global_state->volume_slide_update = exp2(dB_step / 6);
    if (dB_step > 0)
    {
        global_state->volume_slide = 1;
    }
    else if (dB_step < 0)
    {
        global_state->volume_slide = -1;
    }
    else
    {
        global_state->volume = global_state->volume_slide_target;
    }
    return true;
}


static void Event_global_slide_volume_process(Event_global* event, Playdata* play)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_GLOBAL_SLIDE_VOLUME);
    assert(play != NULL);
    Event_global_slide_volume* slide_volume = (Event_global_slide_volume*)event;
    play->volume_slide_target = exp2(slide_volume->target_volume_dB / 6);
    play->volume_slide_frames = Reltime_toframes(&play->volume_slide_length,
                                                 play->tempo,
                                                 play->freq);
    double volume_dB = log2(play->volume) * 6;
    double dB_step = (slide_volume->target_volume_dB - volume_dB) / play->volume_slide_frames;
    play->volume_slide_update = exp2(dB_step / 6);
    if (dB_step > 0)
    {
        play->volume_slide = 1;
    }
    else if (dB_step < 0)
    {
        play->volume_slide = -1;
    }
    else
    {
        play->volume = play->volume_slide_target;
    }
    return;
}


