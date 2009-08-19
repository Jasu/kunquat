

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

#include <Event_common.h>
#include <Event_ins_set_pedal.h>
#include <Instrument_params.h>

#include <xmemory.h>


static Event_field_desc set_pedal_desc[] =
{
    {
        .type = EVENT_FIELD_INT,
        .range.integral_type = { 1, KQT_INSTRUMENTS_MAX }
    },
    {
        .type = EVENT_FIELD_DOUBLE,
        .range.double_type = { 0, 1 }
    },
    {
        .type = EVENT_FIELD_NONE
    }
};


static bool Event_ins_set_pedal_set(Event* event, int index, void* data);

static void* Event_ins_set_pedal_get(Event* event, int index);

static void Event_ins_set_pedal_process(Event_ins* event, Instrument_params* ins_params);


Event* new_Event_ins_set_pedal(Reltime* pos)
{
    assert(pos != NULL);
    Event_ins_set_pedal* event = xalloc(Event_ins_set_pedal);
    if (event == NULL)
    {
        return NULL;
    }
    Event_init(&event->parent.parent,
               pos,
               EVENT_INS_SET_PEDAL,
               set_pedal_desc,
               Event_ins_set_pedal_set,
               Event_ins_set_pedal_get);
    event->parent.process = Event_ins_set_pedal_process;
    event->parent.ins_index = 0;
    event->pedal = 0;
    return (Event*)event;
}


static bool Event_ins_set_pedal_set(Event* event, int index, void* data)
{
    assert(event != NULL);
    assert(event->type == EVENT_INS_SET_PEDAL);
    assert(data != NULL);
    Event_ins_set_pedal* set_pedal = (Event_ins_set_pedal*)event;
    if (index == 0)
    {
        int64_t index = *(int64_t*)data;
        Event_check_integral_range(index, event->field_types[0]);
        set_pedal->parent.ins_index = index;
        return true;
    }
    else if (index == 1)
    {
        double pedal = *(double*)data;
        Event_check_double_range(pedal, event->field_types[1]);
        set_pedal->pedal = pedal;
        return true;
    }
    return false;
}


static void* Event_ins_set_pedal_get(Event* event, int index)
{
    assert(event != NULL);
    assert(event->type == EVENT_INS_SET_PEDAL);
    Event_ins_set_pedal* set_pedal = (Event_ins_set_pedal*)event;
    if (index == 0)
    {
        return &set_pedal->parent.ins_index;
    }
    else if (index == 1)
    {
        return &set_pedal->pedal;
    }
    return NULL;
}


static void Event_ins_set_pedal_process(Event_ins* event, Instrument_params* ins_params)
{
    assert(event != NULL);
    assert(event->parent.type == EVENT_INS_SET_PEDAL);
    assert(ins_params != NULL);
    Event_ins_set_pedal* set_pedal = (Event_ins_set_pedal*)event;
    ins_params->pedal = set_pedal->pedal;
    return;
}


