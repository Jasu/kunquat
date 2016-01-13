

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


#ifndef K_PITCH_STATE_H
#define K_PITCH_STATE_H


#include <decl.h>
#include <kunquat/limits.h>
#include <player/devices/Proc_state.h>
#include <player/devices/Voice_state.h>
#include <player/Pitch_controls.h>


Voice_state_get_size_func Pitch_vstate_get_size;

void Pitch_vstate_init(Voice_state* vstate, const Proc_state* proc_state);

void Pitch_vstate_set_controls(Voice_state* vstate, const Pitch_controls* controls);

void Pitch_vstate_arpeggio_on(
        Voice_state* vstate,
        double speed,
        double ref_pitch,
        double tones[KQT_ARPEGGIO_TONES_MAX]);

void Pitch_vstate_arpeggio_off(Voice_state* vstate);

void Pitch_vstate_update_arpeggio_tones(
        Voice_state* vstate, double tones[KQT_ARPEGGIO_TONES_MAX]);

void Pitch_vstate_update_arpeggio_speed(Voice_state* vstate, double speed);

void Pitch_vstate_reset_arpeggio(Voice_state* vstate);


#endif // K_PITCH_STATE_H


