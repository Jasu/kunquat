

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_COMPRESS_STATE_H
#define KQT_COMPRESS_STATE_H


#include <player/devices/Device_state.h>
#include <player/devices/Voice_state.h>


Device_state_create_func new_Compress_pstate;

Voice_state_get_size_func Compress_vstate_get_size;

Voice_state_init_func Compress_vstate_init;


#endif // KQT_COMPRESS_STATE_H


