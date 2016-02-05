

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/processors/Ringmod_state.h>

#include <debug/assert.h>
#include <init/devices/processors/Proc_ringmod.h>
#include <memory.h>
#include <player/devices/processors/Proc_state_utils.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>


static void multiply_signals(
        float* in1_buffers[2],
        float* in2_buffers[2],
        float* out_buffers[2],
        int32_t buf_start,
        int32_t buf_stop)
{
    assert(in1_buffers != NULL);
    assert(in2_buffers != NULL);
    assert(out_buffers != NULL);
    assert(buf_start >= 0);
    assert(buf_stop >= 0);

    for (int ch = 0; ch < 2; ++ch)
    {
        const float* in1_values = in1_buffers[ch];
        const float* in2_values = in2_buffers[ch];
        float* out_values = out_buffers[ch];

        if ((in1_values != NULL) && (in2_values != NULL) && (out_values != NULL))
        {
            for (int32_t i = buf_start; i < buf_stop; ++i)
                out_values[i] = in1_values[i] * in2_values[i];
        }
    }

    return;
}


static void Ringmod_pstate_render_mixed(
        Device_state* dstate,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    assert(dstate != NULL);
    assert(wbs != NULL);
    assert(isfinite(tempo));
    assert(tempo > 0);

    Proc_state* proc_state = (Proc_state*)dstate;

    // Get inputs
    float* in1_buffers[2] = { NULL };
    Proc_state_get_mixed_audio_in_buffers(proc_state, 0, 2, in1_buffers);

    float* in2_buffers[2] = { NULL };
    Proc_state_get_mixed_audio_in_buffers(proc_state, 2, 4, in2_buffers);

    // Get outputs
    float* out_buffers[2] = { NULL };
    Proc_state_get_mixed_audio_out_buffers(proc_state, 0, 2, out_buffers);

    // Multiply the signals
    multiply_signals(in1_buffers, in2_buffers, out_buffers, buf_start, buf_stop);

    return;
}


Device_state* new_Ringmod_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Proc_state* proc_state =
        new_Proc_state_default(device, audio_rate, audio_buffer_size);
    if (proc_state == NULL)
        return NULL;

    proc_state->render_mixed = Ringmod_pstate_render_mixed;

    return (Device_state*)proc_state;
}


static int32_t Ringmod_vstate_render_voice(
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

    // Get inputs
    float* in1_buffers[2] = { NULL };
    Proc_state_get_voice_audio_in_buffers(proc_state, 0, 2, in1_buffers);

    float* in2_buffers[2] = { NULL };
    Proc_state_get_voice_audio_in_buffers(proc_state, 2, 4, in2_buffers);

    if (((in1_buffers[0] == NULL) || (in2_buffers[0] == NULL)) &&
            ((in1_buffers[1] == NULL) || (in2_buffers[1] == NULL)))
    {
        vstate->active = false;
        return buf_start;
    }

    // Get output
    float* out_buffers[2] = { NULL };
    Proc_state_get_voice_audio_out_buffers(proc_state, 0, 2, out_buffers);

    // Multiply the signals
    multiply_signals(in1_buffers, in2_buffers, out_buffers, buf_start, buf_stop);

    return buf_stop;
}


void Ringmod_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);

    vstate->render_voice = Ringmod_vstate_render_voice;

    return;
}


