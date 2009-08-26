

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>

#include <Event_common.h>
#include <Event_voice_tremolo_speed.h>
#include <Reltime.h>
#include <Voice.h>
#include <math_common.h>

#include <xmemory.h>


static Event_field_desc tremolo_speed_desc[] =
{
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { 0, INFINITY }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


static bool Event_voice_tremolo_speed_set(Event* event, int index, void* data);

static void* Event_voice_tremolo_speed_get(Event* event, int index);

static void Event_voice_tremolo_speed_process(Event_voice* event, Voice* voice);


Event* new_Event_voice_tremolo_speed(Reltime* pos)
{
    assert(pos != NULL);
    Event_voice_tremolo_speed* event = xalloc(Event_voice_tremolo_speed);
    if (event == NULL)
    {
        return NULL;
    }
    Event_init(&event->parent.parent,
               pos,
               EVENT_VOICE_TREMOLO_SPEED,
               tremolo_speed_desc,
               Event_voice_tremolo_speed_set,
               Event_voice_tremolo_speed_get);
    event->parent.process = Event_voice_tremolo_speed_process;
    event->speed = 0;
    return (Event*)event;
}


static bool Event_voice_tremolo_speed_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_VOICE_TREMOLO_SPEED);
    assert(data != NULL);
    Event_voice_tremolo_speed* tremolo_speed = (Event_voice_tremolo_speed*)event;
    if (index != 0)
    {
        return false;
    }
    double speed = *(double*)data;
    Event_check_double_range(speed, event->field_types[0]);
    tremolo_speed->speed = speed;
    return true;
}


static void* Event_voice_tremolo_speed_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_VOICE_TREMOLO_SPEED);
    Event_voice_tremolo_speed* tremolo_speed = (Event_voice_tremolo_speed*)event;
    if (index != 0)
    {
        return NULL;
    }
    return &tremolo_speed->speed;
}


static void Event_voice_tremolo_speed_process(Event_voice* event, Voice* voice)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_VOICE_TREMOLO_SPEED);
    assert(voice != NULL);
    Event_voice_tremolo_speed* tremolo_speed = (Event_voice_tremolo_speed*)event;
    if (tremolo_speed->speed > 0 && voice->state.generic.tremolo_depth_target > 0)
    {
        voice->state.generic.tremolo = true;
    }
    double unit_len = Reltime_toframes(Reltime_set(RELTIME_AUTO, 1, 0),
                                       voice->state.generic.tempo,
                                       voice->state.generic.freq);
    voice->state.generic.tremolo_length = unit_len / tremolo_speed->speed;
    voice->state.generic.tremolo_update = (2 * PI) / voice->state.generic.tremolo_length;
    Channel_state* ch_state = voice->state.generic.new_ch_state;
    ch_state->tremolo_length = voice->state.generic.tremolo_length;
    ch_state->tremolo_update = voice->state.generic.tremolo_update;
    if (!voice->state.generic.tremolo)
    {
        voice->state.generic.tremolo_delay_pos = 0;
    }
    return;
}


