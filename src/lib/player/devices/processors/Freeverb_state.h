

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_FREEVERB_STATE_H
#define K_FREEVERB_STATE_H


#include <decl.h>
#include <devices/Device.h>
#include <player/devices/Device_state.h>

#include <stdint.h>


Device_state* new_Freeverb_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size);


#endif // K_FREEVERB_STATE_H


