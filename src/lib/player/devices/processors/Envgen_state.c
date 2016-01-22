

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


#include <player/devices/processors/Envgen_state.h>

#include <debug/assert.h>
#include <init/devices/processors/Proc_envgen.h>
#include <mathnum/common.h>
#include <mathnum/conversions.h>
#include <memory.h>
#include <player/devices/Proc_state.h>
#include <player/devices/processors/Proc_utils.h>
#include <player/devices/Voice_state.h>
#include <player/Work_buffers.h>

#include <stdlib.h>
#include <string.h>


typedef struct Envgen_vstate
{
    Voice_state parent;

    Time_env_state env_state;
} Envgen_vstate;


size_t Envgen_vstate_get_size(void)
{
    return sizeof(Envgen_vstate);
}


static const int ENVGEN_WB_FIXED_PITCH = WORK_BUFFER_IMPL_1;
static const int ENVGEN_WB_FIXED_FORCE = WORK_BUFFER_IMPL_2;


static int32_t Envgen_vstate_render_voice(
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

    const Proc_envgen* egen = (Proc_envgen*)proc_state->parent.device->dimpl;
    Envgen_vstate* egen_state = (Envgen_vstate*)vstate;

    // Get pitch input
    float* pitches = Proc_state_get_voice_buffer_contents_mut(
            proc_state, DEVICE_PORT_TYPE_RECEIVE, 0);
    if (pitches == NULL)
    {
        pitches = Work_buffers_get_buffer_contents_mut(wbs, ENVGEN_WB_FIXED_PITCH);
        for (int32_t i = buf_start; i < buf_stop; ++i)
            pitches[i] = 0;
    }

    // Get force scales
    float* force_scales = Proc_state_get_voice_buffer_contents_mut(
            proc_state, DEVICE_PORT_TYPE_RECEIVE, 1);

    // Get output buffer for writing
    float* out_buffer =
        Proc_state_get_voice_buffer_contents_mut(proc_state, DEVICE_PORT_TYPE_SEND, 0);
    if (out_buffer == NULL)
    {
        vstate->active = false;
        return buf_start;
    }

    const bool is_time_env_enabled =
        egen->is_time_env_enabled && (egen->time_env != NULL);
    const bool is_force_env_enabled =
        egen->is_force_env_enabled && (egen->force_env != NULL);

    const double range_width = egen->y_max - egen->y_min;

    int32_t new_buf_stop = buf_stop;

    if (is_time_env_enabled && (!egen->is_release_env || !vstate->note_on))
    {
        // Apply the time envelope
        const int32_t env_stop = Time_env_state_process(
                &egen_state->env_state,
                egen->time_env,
                egen->is_loop_enabled,
                egen->env_scale_amount,
                egen->env_scale_center,
                0, // sustain
                0, 1, // range, NOTE: this needs to be mapped to our [y_min, y_max]!
                pitches,
                Work_buffers_get_buffer_contents_mut(wbs, WORK_BUFFER_TIME_ENV),
                buf_start,
                new_buf_stop,
                proc_state->parent.audio_rate);

        float* time_env = Work_buffers_get_buffer_contents_mut(
                wbs, WORK_BUFFER_TIME_ENV);

        // Check the end of envelope processing
        if (egen_state->env_state.is_finished)
        {
            const double* last_node = Envelope_get_node(
                    egen->time_env, Envelope_node_count(egen->time_env) - 1);
            const double last_value = last_node[1];
            /*
            if (fabs(egen->y_min + last_value * range_width) < 0.0001)
            {
                Voice_state_set_finished(vstate);
                new_buf_stop = env_stop;
            }
            else
            // */
            {
                // Fill the rest of the envelope buffer with the last value
                for (int32_t i = env_stop; i < new_buf_stop; ++i)
                    time_env[i] = last_value;
            }
        }

        // Write to signal output
        for (int32_t i = buf_start; i < new_buf_stop; ++i)
            out_buffer[i] = time_env[i];
    }
    else
    {
        // Initialise with default values
        for (int32_t i = buf_start; i < new_buf_stop; ++i)
            out_buffer[i] = 1;
    }

    if (egen->is_linear_force)
    {
        if (force_scales != NULL)
        {
            // Convert input force to linear scale
            for (int32_t i = buf_start; i < buf_stop; ++i)
                force_scales[i] = fast_dB_to_scale(force_scales[i]);

            if (is_force_env_enabled)
            {
                // Apply force envelope
                for (int32_t i = buf_start; i < new_buf_stop; ++i)
                {
                    const float force_scale = force_scales[i];

                    const double vol_clamped = min(1, force_scale);
                    const double factor = Envelope_get_value(egen->force_env, vol_clamped);
                    assert(isfinite(factor));
                    out_buffer[i] *= factor;
                }
            }
            else
            {
                // Apply linear scaling by default
                for (int32_t i = buf_start; i < new_buf_stop; ++i)
                    out_buffer[i] *= force_scales[i];
            }
        }
        else
        {
            if (is_force_env_enabled)
            {
                // Just apply the rightmost force envelope value (as we assume force 0 dB)
                const double factor = Envelope_get_node(
                        egen->force_env, Envelope_node_count(egen->force_env) - 1)[1];
                for (int32_t i = buf_start; i < new_buf_stop; ++i)
                    out_buffer[i] *= factor;
            }
        }

        // Convert to dB
        const double global_adjust = egen->global_adjust;
        for (int32_t i = buf_start; i < new_buf_stop; ++i)
            out_buffer[i] = fast_scale_to_dB(out_buffer[i]) + global_adjust;
    }
    else
    {
        if ((egen->y_min != 0) || (egen->y_max != 1))
        {
            // Apply range
            for (int32_t i = buf_start; i < new_buf_stop; ++i)
                out_buffer[i] =
                    egen->y_min + out_buffer[i] * range_width;
        }

        const double global_adjust = egen->global_adjust;

        if (force_scales != NULL)
        {
            for (int32_t i = buf_start; i < new_buf_stop; ++i)
                out_buffer[i] += force_scales[i] + global_adjust;
        }
        else
        {
            for (int32_t i = buf_start; i < new_buf_stop; ++i)
                out_buffer[i] += global_adjust;
        }
    }


    return new_buf_stop;
}


void Envgen_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);

    vstate->render_voice = Envgen_vstate_render_voice;

    Envgen_vstate* egen_state = (Envgen_vstate*)vstate;

    Time_env_state_init(&egen_state->env_state);

    return;
}


