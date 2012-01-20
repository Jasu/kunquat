

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


#ifndef K_EVENT_CHANNEL_SET_GEN_INT_NAME_H
#define K_EVENT_CHANNEL_SET_GEN_INT_NAME_H


#include <Event_channel.h>
#include <Reltime.h>
#include <Value.h>


Event* new_Event_channel_set_gen_int_name(Reltime* pos);


bool Event_channel_set_gen_int_name_process(Channel_state* ch_state,
                                            Value* value);


#endif // K_EVENT_CHANNEL_SET_GEN_INT_NAME_H



