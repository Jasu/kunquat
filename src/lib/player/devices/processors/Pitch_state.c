

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


#include <player/devices/processors/Pitch_state.h>

#include <debug/assert.h>
#include <init/devices/processors/Proc_pitch.h>
#include <mathnum/conversions.h>
#include <player/Pitch_controls.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct Pitch_vstate
{
    Voice_state parent;

    Pitch_controls controls;
    double orig_pitch_param;
    double actual_pitch;
    double prev_actual_pitch;

    bool is_arpeggio_enabled;
    double arpeggio_ref_pitch;
    double arpeggio_speed;
    double arpeggio_tone_progress;
    int arpeggio_tone_index;
    double arpeggio_tones[KQT_ARPEGGIO_TONES_MAX];
} Pitch_vstate;


size_t Pitch_vstate_get_size(void)
{
    return sizeof(Pitch_vstate);
}


static int32_t Pitch_vstate_render_voice(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);
    assert(au_state != NULL);
    assert(wbs != NULL);
    assert(buf_start >= 0);
    assert(buf_stop >= 0);
    assert(isfinite(tempo));
    assert(tempo > 0);

    // Get output
    float* out_buf =
        Proc_state_get_voice_buffer_contents_mut(proc_state, DEVICE_PORT_TYPE_SEND, 0);
    if (out_buf == NULL)
    {
        vstate->active = false;
        return buf_start;
    }

    Pitch_vstate* pvstate = (Pitch_vstate*)vstate;

    Pitch_controls* pc = &pvstate->controls;
    Pitch_controls_set_tempo(pc, tempo);

    out_buf[buf_start - 1] = pc->pitch;

    // Apply pitch slide
    if (Slider_in_progress(&pc->slider))
    {
        float new_pitch = pc->pitch;
        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            new_pitch = Slider_step(&pc->slider);
            out_buf[i] = new_pitch;
        }
        pc->pitch = new_pitch;
    }
    else
    {
        for (int32_t i = buf_start; i < buf_stop; ++i)
            out_buf[i] = pc->pitch;
    }

    // Adjust carried pitch
    if (pc->freq_mul != 1)
    {
        for (int32_t i = buf_start; i < buf_stop; ++i)
            out_buf[i] *= pc->freq_mul;
    }

    out_buf[buf_start - 1] = pvstate->actual_pitch;

    // Apply vibrato
    if (LFO_active(&pc->vibrato))
    {
        for (int32_t i = buf_start; i < buf_stop; ++i)
            out_buf[i] *= LFO_step(&pc->vibrato);
    }

    // Apply arpeggio
    if (pvstate->is_arpeggio_enabled)
    {
        const Device_state* dstate = &proc_state->parent;
        const double progress_update =
            (pvstate->arpeggio_speed / dstate->audio_rate) * (tempo / 60.0);

        for (int32_t i = buf_start; i < buf_stop; ++i)
        {
            // Adjust actual pitch according to the current arpeggio state
            assert(!isnan(pvstate->arpeggio_tones[0]));
            double diff = exp2(
                    (pvstate->arpeggio_tones[pvstate->arpeggio_tone_index] -
                        pvstate->arpeggio_ref_pitch) / 1200.0);
            out_buf[i] *= diff;

            // Update arpeggio state
            pvstate->arpeggio_tone_progress += progress_update;
            while (pvstate->arpeggio_tone_progress > 1.0)
            {
                pvstate->arpeggio_tone_progress -= 1.0;
                ++pvstate->arpeggio_tone_index;
                if (pvstate->arpeggio_tone_index >= KQT_ARPEGGIO_TONES_MAX ||
                        isnan(pvstate->arpeggio_tones[pvstate->arpeggio_tone_index]))
                    pvstate->arpeggio_tone_index = 0;
            }
        }
    }

    // Update actual pitch for next iteration
    pvstate->actual_pitch = out_buf[buf_stop - 1];
    pvstate->prev_actual_pitch = out_buf[buf_stop - 2];

    // Mark state as started, TODO: fix this mess
    vstate->pos = 1;

    return buf_stop;
}


void Pitch_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);

    vstate->render_voice = Pitch_vstate_render_voice;

    Pitch_vstate* pvstate = (Pitch_vstate*)vstate;

    // Initialise common pitch state
    Pitch_controls_init(&pvstate->controls, proc_state->parent.audio_rate, 120);

    pvstate->orig_pitch_param = NAN;
    pvstate->actual_pitch = 0;
    pvstate->prev_actual_pitch = 0;

    // Initialise arpeggio
    pvstate->is_arpeggio_enabled = false;
    pvstate->arpeggio_ref_pitch = NAN;
    pvstate->arpeggio_speed = 24;
    pvstate->arpeggio_tone_progress = 0;
    pvstate->arpeggio_tone_index = 0;
    pvstate->arpeggio_tones[0] = pvstate->arpeggio_tones[1] = NAN;

    vstate->is_pitch_state = true;

    return;
}


void Pitch_vstate_set_controls(Voice_state* vstate, const Pitch_controls* controls)
{
    assert(vstate != NULL);
    assert(controls != NULL);

    Pitch_vstate* pvstate = (Pitch_vstate*)vstate;

    Pitch_controls_copy(&pvstate->controls, controls);

    if (isnan(pvstate->orig_pitch_param))
        pvstate->orig_pitch_param = pvstate->controls.orig_carried_pitch;

    pvstate->actual_pitch = pvstate->controls.pitch;

    return;
}


void Pitch_vstate_arpeggio_on(
        Voice_state* vstate,
        double speed,
        double ref_pitch,
        double tones[KQT_ARPEGGIO_TONES_MAX])
{
    assert(vstate != NULL);
    assert(isfinite(speed));
    assert(speed > 0);
    assert(isfinite(ref_pitch));
    assert(tones != NULL);

    Pitch_vstate* pvstate = (Pitch_vstate*)vstate;

    if (pvstate->is_arpeggio_enabled)
        return;

    pvstate->arpeggio_speed = speed;
    pvstate->arpeggio_ref_pitch = ref_pitch;
    memcpy(pvstate->arpeggio_tones, tones, KQT_ARPEGGIO_TONES_MAX * sizeof(double));
    pvstate->arpeggio_tone_index = 0;
    pvstate->is_arpeggio_enabled = true;

    return;
}


void Pitch_vstate_arpeggio_off(Voice_state* vstate)
{
    assert(vstate != NULL);

    Pitch_vstate* pvstate = (Pitch_vstate*)vstate;
    pvstate->is_arpeggio_enabled = false;

    return;
}


void Pitch_vstate_update_arpeggio_tones(
        Voice_state* vstate, double tones[KQT_ARPEGGIO_TONES_MAX])
{
    assert(vstate != NULL);

    Pitch_vstate* pvstate = (Pitch_vstate*)vstate;
    memcpy(pvstate->arpeggio_tones, tones, KQT_ARPEGGIO_TONES_MAX * sizeof(double));

    return;
}


void Pitch_vstate_update_arpeggio_speed(Voice_state* vstate, double speed)
{
    assert(vstate != NULL);
    assert(isfinite(speed));
    assert(speed > 0);

    Pitch_vstate* pvstate = (Pitch_vstate*)vstate;
    pvstate->arpeggio_speed = speed;

    return;
}


void Pitch_vstate_reset_arpeggio(Voice_state* vstate)
{
    assert(vstate != NULL);

    Pitch_vstate* pvstate = (Pitch_vstate*)vstate;
    pvstate->arpeggio_tones[0] = pvstate->arpeggio_tones[1] = NAN;
    pvstate->arpeggio_tone_index = 0;

    return;
}


