

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


#include <player/devices/processors/Stream_state.h>

#include <debug/assert.h>
#include <init/devices/Device.h>
#include <init/devices/processors/Proc_stream.h>
#include <memory.h>
#include <player/devices/Proc_state.h>
#include <player/devices/Voice_state.h>
#include <player/Linear_controls.h>

#include <stdint.h>
#include <stdlib.h>


static void apply_controls(
        Linear_controls* controls,
        const Work_buffer* out_wb,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    assert(controls != NULL);
    assert(buf_start >= 0);
    assert(buf_stop >= buf_start);

    Linear_controls_set_tempo(controls, tempo);

    if (out_wb != NULL)
        Linear_controls_fill_work_buffer(controls, out_wb, buf_start, buf_stop);
    else
        Linear_controls_skip(controls, buf_stop - buf_start);

    return;
}


typedef struct Stream_pstate
{
    Proc_state parent;

    double init_value;
    Linear_controls controls;
} Stream_pstate;


static bool Stream_pstate_set_audio_rate(Device_state* dstate, int32_t audio_rate)
{
    assert(dstate != NULL);
    assert(audio_rate > 0);

    Stream_pstate* spstate = (Stream_pstate*)dstate;

    Linear_controls_set_audio_rate(&spstate->controls, audio_rate);

    return true;
}


static void Stream_pstate_reset(Device_state* dstate)
{
    assert(dstate != NULL);

    Stream_pstate* spstate = (Stream_pstate*)dstate;

    Linear_controls_init(&spstate->controls);
    Linear_controls_set_value(&spstate->controls, spstate->init_value);

    return;
}


static void Stream_pstate_render_mixed(
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

    Stream_pstate* spstate = (Stream_pstate*)dstate;

    // Get output
    Work_buffer* out_wb =
        Device_state_get_audio_buffer(dstate, DEVICE_PORT_TYPE_SEND, 0);

    apply_controls(&spstate->controls, out_wb, buf_start, buf_stop, tempo);

    return;
}


Device_state* new_Stream_pstate(
        const Device* device, int32_t audio_rate, int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Stream_pstate* spstate = memory_alloc_item(Stream_pstate);
    if ((spstate == NULL) ||
            !Proc_state_init(&spstate->parent, device, audio_rate, audio_buffer_size))
    {
        memory_free(spstate);
        return NULL;
    }

    spstate->parent.parent.is_stream_state = true;
    spstate->parent.set_audio_rate = Stream_pstate_set_audio_rate;
    spstate->parent.reset = Stream_pstate_reset;
    spstate->parent.render_mixed = Stream_pstate_render_mixed;

    const Proc_stream* stream = (const Proc_stream*)device->dimpl;
    spstate->init_value = stream->init_value;

    Linear_controls_init(&spstate->controls);
    Linear_controls_set_audio_rate(&spstate->controls, audio_rate);
    Linear_controls_set_tempo(&spstate->controls, 120);
    Linear_controls_set_value(&spstate->controls, spstate->init_value);

    return (Device_state*)spstate;
}


bool Stream_pstate_set_init_value(
        Device_state* dstate, const Key_indices indices, double value)
{
    assert(dstate != NULL);
    assert(indices != NULL);

    Stream_pstate* spstate = (Stream_pstate*)dstate;

    spstate->init_value = isfinite(value) ? value : 0.0;

    return true;
}


void Stream_pstate_set_value(Device_state* dstate, double value)
{
    assert(dstate != NULL);
    assert(isfinite(value));

    Stream_pstate* spstate = (Stream_pstate*)dstate;

    Linear_controls_set_value(&spstate->controls, value);

    return;
}


void Stream_pstate_slide_target(Device_state* dstate, double value)
{
    assert(dstate != NULL);
    assert(isfinite(value));

    Stream_pstate* spstate = (Stream_pstate*)dstate;

    Linear_controls_slide_value_target(&spstate->controls, value);

    return;
}


void Stream_pstate_slide_length(Device_state* dstate, const Tstamp* length)
{
    assert(dstate != NULL);
    assert(length != NULL);

    Stream_pstate* spstate = (Stream_pstate*)dstate;

    Linear_controls_slide_value_length(&spstate->controls, length);

    return;
}


void Stream_pstate_set_osc_speed(Device_state* dstate, double speed)
{
    assert(dstate != NULL);
    assert(isfinite(speed));

    Stream_pstate* spstate = (Stream_pstate*)dstate;

    Linear_controls_osc_speed_value(&spstate->controls, speed);

    return;
}


void Stream_pstate_set_osc_depth(Device_state* dstate, double depth)
{
    assert(dstate != NULL);
    assert(isfinite(depth));

    Stream_pstate* spstate = (Stream_pstate*)dstate;

    Linear_controls_osc_depth_value(&spstate->controls, depth);

    return;
}


void Stream_pstate_set_osc_speed_slide(Device_state* dstate, const Tstamp* length)
{
    assert(dstate != NULL);
    assert(length != NULL);

    Stream_pstate* spstate = (Stream_pstate*)dstate;

    Linear_controls_osc_speed_slide_value(&spstate->controls, length);

    return;
}


void Stream_pstate_set_osc_depth_slide(Device_state* dstate, const Tstamp* length)
{
    assert(dstate != NULL);
    assert(length != NULL);

    Stream_pstate* spstate = (Stream_pstate*)dstate;

    Linear_controls_osc_depth_slide_value(&spstate->controls, length);

    return;
}


typedef struct Stream_vstate
{
    Voice_state parent;

    double init_value;
    Linear_controls controls;
} Stream_vstate;


size_t Stream_vstate_get_size(void)
{
    return sizeof(Stream_vstate);
}


static int32_t Stream_vstate_render_voice(
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

    Stream_vstate* svstate = (Stream_vstate*)vstate;

    // Get output
    Work_buffer* out_wb =
        Proc_state_get_voice_buffer_mut(proc_state, DEVICE_PORT_TYPE_SEND, 0);
    if (out_wb == NULL)
    {
        vstate->active = false;
        return buf_start;
    }

    apply_controls(&svstate->controls, out_wb, buf_start, buf_stop, tempo);

    return buf_stop;
}


void Stream_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    assert(vstate != NULL);
    assert(proc_state != NULL);

    vstate->render_voice = Stream_vstate_render_voice;

    vstate->is_stream_state = true;

    Stream_vstate* svstate = (Stream_vstate*)vstate;

    const Proc_stream* stream = (const Proc_stream*)proc_state->parent.device->dimpl;

    svstate->init_value = stream->init_value;

    Linear_controls_init(&svstate->controls);
    Linear_controls_set_audio_rate(&svstate->controls, proc_state->parent.audio_rate);
    Linear_controls_set_tempo(&svstate->controls, 120);
    Linear_controls_set_value(&svstate->controls, svstate->init_value);

    return;
}


const Linear_controls* Stream_vstate_get_controls(const Voice_state* vstate)
{
    assert(vstate != NULL);
    assert(vstate->is_stream_state);

    const Stream_vstate* svstate = (const Stream_vstate*)vstate;

    return &svstate->controls;
}


void Stream_vstate_set_controls(Voice_state* vstate, const Linear_controls* controls)
{
    assert(vstate != NULL);
    assert(vstate->is_stream_state);
    assert(controls != NULL);

    Stream_vstate* svstate = (Stream_vstate*)vstate;

    Linear_controls_copy(&svstate->controls, controls);

    return;
}


