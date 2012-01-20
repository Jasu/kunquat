

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2012
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_EVENT_CONTROL_SET_GOTO_SUBSONG_H
#define K_EVENT_CONTROL_SET_GOTO_SUBSONG_H


#include <stdbool.h>

#include <General_state.h>
#include <Reltime.h>
#include <Value.h>


Event* new_Event_control_set_goto_subsong(Reltime* pos);


bool Event_control_set_goto_subsong_process(General_state* gstate,
                                            Value* value);


#endif // K_EVENT_CONTROL_SET_GOTO_SUBSONG_H


