

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


#ifndef K_EVENT_GLOBAL_SET_SCALE_FIXED_POINT_H
#define K_EVENT_GLOBAL_SET_SCALE_FIXED_POINT_H


#include <stdbool.h>

#include <Playdata.h>
#include <Value.h>


bool Event_global_set_scale_fixed_point_process(
        Playdata* global_state,
        Value* value);


#endif // K_EVENT_GLOBAL_SET_SCALE_FIXED_POINT_H


