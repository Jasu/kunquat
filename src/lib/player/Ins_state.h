

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_INS_STATE_H
#define K_INS_STATE_H


#include <player/Device_state.h>


typedef struct Ins_state
{
    Device_state parent;

    double sustain; // 0 = no sustain, 1.0 = full sustain
} Ins_state;


#endif // K_INS_STATE_H

