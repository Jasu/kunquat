

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/Proc_state.h>

#include <debug/assert.h>
#include <init/devices/Device.h>
#include <init/devices/Device_impl.h>
#include <mathnum/Tstamp.h>
#include <player/devices/Device_state.h>
#include <player/devices/Voice_state.h>
#include <player/Work_buffer.h>

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


static bool Proc_state_add_buffer(
        Device_state* dstate, Device_port_type port_type, int port_num);

static bool Proc_state_set_audio_buffer_size(Device_state* dstate, int32_t new_size);

static Device_state_set_audio_rate_func Proc_state_set_audio_rate;

static Device_state_set_tempo_func Proc_state_set_tempo;

static Device_state_reset_func Proc_state_reset;

static Device_state_render_mixed_func Proc_state_render_mixed;


bool Proc_state_init(
        Proc_state* proc_state,
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size)
{
    assert(proc_state != NULL);
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    if (!Device_state_init(&proc_state->parent, device, audio_rate, audio_buffer_size))
        return false;

    for (Device_port_type port_type = DEVICE_PORT_TYPE_RECEIVE;
            port_type < DEVICE_PORT_TYPES; ++port_type)
    {
        for (int port_num = 0; port_num < KQT_DEVICE_PORTS_MAX; ++port_num)
            proc_state->voice_buffers[port_type][port_num] = NULL;
    }

    proc_state->voice_out_buffers_modified = NULL;

    proc_state->parent.add_buffer = Proc_state_add_buffer;
    proc_state->parent.set_audio_rate = Proc_state_set_audio_rate;
    proc_state->parent.set_audio_buffer_size = Proc_state_set_audio_buffer_size;
    proc_state->parent.set_tempo = Proc_state_set_tempo;
    proc_state->parent.reset = Proc_state_reset;
    proc_state->parent.render_mixed = Proc_state_render_mixed;
    proc_state->parent.deinit = Proc_state_deinit;

    proc_state->set_audio_rate = NULL;
    proc_state->set_audio_buffer_size = NULL;
    proc_state->set_tempo = NULL;
    proc_state->reset = NULL;
    proc_state->render_mixed = NULL;

    proc_state->clear_history = NULL;

    proc_state->voice_out_buffers_modified = new_Bit_array(KQT_DEVICE_PORTS_MAX);
    if (proc_state->voice_out_buffers_modified == NULL)
    {
        Proc_state_deinit(&proc_state->parent);
        return false;
    }

    return true;
}


bool Proc_state_set_audio_rate(Device_state* dstate, int32_t audio_rate)
{
    assert(dstate != NULL);
    assert(audio_rate > 0);

    Proc_state* proc_state = (Proc_state*)dstate;
    if (proc_state->set_audio_rate != NULL)
        return proc_state->set_audio_rate(dstate, audio_rate);

    return true;
}


void Proc_state_set_tempo(Device_state* dstate, double tempo)
{
    assert(dstate != NULL);
    assert(isfinite(tempo));
    assert(tempo > 0);

    Proc_state* proc_state = (Proc_state*)dstate;
    if (proc_state->set_tempo != NULL)
        proc_state->set_tempo(dstate, tempo);

    return;
}


void Proc_state_reset(Device_state* dstate)
{
    assert(dstate != NULL);

    Proc_state* proc_state = (Proc_state*)dstate;
    if (proc_state->reset != NULL)
        proc_state->reset(dstate);

    return;
}


void Proc_state_render_mixed(
        Device_state* dstate,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    assert(dstate != NULL);
    assert(wbs != NULL);
    assert(buf_start >= 0);
    assert(isfinite(tempo));
    assert(tempo > 0);

    Proc_state* proc_state = (Proc_state*)dstate;
    if (proc_state->render_mixed != NULL)
        proc_state->render_mixed(dstate, wbs, buf_start, buf_stop, tempo);

    return;
}


void Proc_state_clear_history(Proc_state* proc_state)
{
    assert(proc_state != NULL);

    if (proc_state->clear_history != NULL)
        proc_state->clear_history(proc_state);

    return;
}


void Proc_state_clear_voice_buffers(Proc_state* proc_state)
{
    assert(proc_state != NULL);

    for (Device_port_type port_type = DEVICE_PORT_TYPE_RECEIVE;
            port_type < DEVICE_PORT_TYPES; ++port_type)
    {
        for (int port_num = 0; port_num < KQT_DEVICE_PORTS_MAX; ++port_num)
        {
            Work_buffer* buffer = proc_state->voice_buffers[port_type][port_num];
            if (buffer != NULL)
                Work_buffer_clear(buffer, 0, Work_buffer_get_size(buffer));
        }
    }

    Bit_array_clear(proc_state->voice_out_buffers_modified);

    return;
}


bool Proc_state_is_voice_out_buffer_modified(Proc_state* proc_state, int port_num)
{
    assert(proc_state != NULL);
    assert(port_num >= 0);
    assert(port_num < KQT_DEVICE_PORTS_MAX);

    return Bit_array_get(proc_state->voice_out_buffers_modified, port_num);
}


const Work_buffer* Proc_state_get_voice_buffer(
        const Proc_state* proc_state, Device_port_type port_type, int port_num)
{
    assert(proc_state != NULL);
    assert(port_type < DEVICE_PORT_TYPES);
    assert(port_num >= 0);
    assert(port_num < KQT_DEVICE_PORTS_MAX);

    if ((port_type == DEVICE_PORT_TYPE_RECEIVE) &&
            !Device_state_is_input_port_connected(&proc_state->parent, port_num))
        return NULL;

    return proc_state->voice_buffers[port_type][port_num];
}


Work_buffer* Proc_state_get_voice_buffer_mut(
        Proc_state* proc_state, Device_port_type port_type, int port_num)
{
    assert(proc_state != NULL);
    assert(port_type < DEVICE_PORT_TYPES);
    assert(port_num >= 0);
    assert(port_num < KQT_DEVICE_PORTS_MAX);

    if ((port_type == DEVICE_PORT_TYPE_RECEIVE) &&
            !Device_state_is_input_port_connected(&proc_state->parent, port_num))
        return NULL;

    if (port_type == DEVICE_PORT_TYPE_SEND)
        Bit_array_set(proc_state->voice_out_buffers_modified, port_num, true);

    return proc_state->voice_buffers[port_type][port_num];
}


const float* Proc_state_get_voice_buffer_contents(
        const Proc_state* proc_state, Device_port_type port_type, int port_num)
{
    assert(proc_state != NULL);
    assert(port_type < DEVICE_PORT_TYPES);
    assert(port_num >= 0);
    assert(port_num < KQT_DEVICE_PORTS_MAX);

    const Work_buffer* wb = Proc_state_get_voice_buffer(proc_state, port_type, port_num);
    if (wb == NULL)
        return NULL;

    return Work_buffer_get_contents(wb);
}


float* Proc_state_get_voice_buffer_contents_mut(
        Proc_state* proc_state, Device_port_type port_type, int port_num)
{
    assert(proc_state != NULL);
    assert(port_type < DEVICE_PORT_TYPES);
    assert(port_num >= 0);
    assert(port_num < KQT_DEVICE_PORTS_MAX);

    const Work_buffer* wb =
        Proc_state_get_voice_buffer_mut(proc_state, port_type, port_num);
    if (wb == NULL)
        return NULL;

    return Work_buffer_get_contents_mut(wb);
}


void Proc_state_cv_generic_set(
        Device_state* dstate, const char* key, const Value* value)
{
    assert(dstate != NULL);
    assert(key != NULL);
    assert(value != NULL);

    const Device_impl* dimpl = dstate->device->dimpl;

    Device_impl_proc_cv_callback* cb = DEVICE_IMPL_PROC_CV_CALLBACK_AUTO;
    Device_impl_get_proc_cv_callback(dimpl, key, value->type, cb);

    if (cb->type == VALUE_TYPE_NONE)
        return;

    switch (cb->type)
    {
        case VALUE_TYPE_BOOL:
        {
            cb->cb.set_bool(dstate, cb->indices, value->value.bool_type);
        }
        break;

        case VALUE_TYPE_INT:
        {
            cb->cb.set_int(dstate, cb->indices, value->value.int_type);
        }
        break;

        case VALUE_TYPE_FLOAT:
        {
            Linear_controls* controls =
                cb->cb.get_float_controls(dstate, cb->indices);
            if (controls != NULL)
                Linear_controls_set_value(controls, value->value.float_type);
        }
        break;

        case VALUE_TYPE_TSTAMP:
        {
            cb->cb.set_tstamp(dstate, cb->indices, &value->value.Tstamp_type);
        }
        break;

        default:
            assert(false);
    }

    return;
}


static Linear_controls* get_cv_float_controls(Device_state* dstate, const char* key)
{
    assert(dstate != NULL);
    assert(key != NULL);

    const Device_impl* dimpl = dstate->device->dimpl;

    Device_impl_proc_cv_callback* cb = DEVICE_IMPL_PROC_CV_CALLBACK_AUTO;
    Device_impl_get_proc_cv_callback(dimpl, key, VALUE_TYPE_FLOAT, cb);

    if (cb->type != VALUE_TYPE_FLOAT)
        return NULL;

    return cb->cb.get_float_controls(dstate, cb->indices);
}


void Proc_state_cv_float_slide_target(
        Device_state* dstate, const char* key, double value)
{
    assert(dstate != NULL);
    assert(key != NULL);
    assert(isfinite(value));

    Linear_controls* controls = get_cv_float_controls(dstate, key);
    if (controls != NULL)
        Linear_controls_slide_value_target(controls, value);

    return;
}


void Proc_state_cv_float_slide_length(
        Device_state* dstate, const char* key, const Tstamp* length)
{
    assert(dstate != NULL);
    assert(key != NULL);
    assert(length != NULL);

    Linear_controls* controls = get_cv_float_controls(dstate, key);
    if (controls != NULL)
        Linear_controls_slide_value_length(controls, length);

    return;
}


void Proc_state_cv_float_osc_speed(Device_state* dstate, const char* key, double speed)
{
    assert(dstate != NULL);
    assert(key != NULL);
    assert(isfinite(speed));
    assert(speed >= 0);

    Linear_controls* controls = get_cv_float_controls(dstate, key);
    if (controls != NULL)
        Linear_controls_osc_speed_value(controls, speed);

    return;
}


void Proc_state_cv_float_osc_depth(Device_state* dstate, const char* key, double depth)
{
    assert(dstate != NULL);
    assert(key != NULL);
    assert(isfinite(depth));

    Linear_controls* controls = get_cv_float_controls(dstate, key);
    if (controls != NULL)
        Linear_controls_osc_depth_value(controls, depth);

    return;
}


void Proc_state_cv_float_osc_speed_slide(
        Device_state* dstate, const char* key, const Tstamp* length)
{
    assert(dstate != NULL);
    assert(key != NULL);
    assert(length != NULL);

    Linear_controls* controls = get_cv_float_controls(dstate, key);
    if (controls != NULL)
        Linear_controls_osc_speed_slide_value(controls, length);

    return;
}


void Proc_state_cv_float_osc_depth_slide(
        Device_state* dstate, const char* key, const Tstamp* length)
{
    assert(dstate != NULL);
    assert(key != NULL);
    assert(length != NULL);

    Linear_controls* controls = get_cv_float_controls(dstate, key);
    if (controls != NULL)
        Linear_controls_osc_depth_slide_value(controls, length);

    return;
}


static bool Proc_state_add_buffer(
        Device_state* dstate, Device_port_type port_type, int port_num)
{
    assert(dstate != NULL);
    assert(port_type == DEVICE_PORT_TYPE_RECEIVE || port_type == DEVICE_PORT_TYPE_SEND);
    assert(port_num >= 0);
    assert(port_num < KQT_DEVICE_PORTS_MAX);

    Proc_state* proc_state = (Proc_state*)dstate;

    if (proc_state->voice_buffers[port_type][port_num] != NULL)
        return true;

    Work_buffer* new_buffer = new_Work_buffer(dstate->audio_buffer_size);
    if (new_buffer == NULL)
        return false;

    proc_state->voice_buffers[port_type][port_num] = new_buffer;

    return true;
}


static bool Proc_state_set_audio_buffer_size(Device_state* dstate, int32_t new_size)
{
    assert(dstate != NULL);
    assert(new_size >= 0);

    Proc_state* proc_state = (Proc_state*)dstate;

    for (Device_port_type port_type = DEVICE_PORT_TYPE_RECEIVE;
            port_type < DEVICE_PORT_TYPES; ++port_type)
    {
        for (int port_num = 0; port_num < KQT_DEVICE_PORTS_MAX; ++port_num)
        {
            Work_buffer* buffer = proc_state->voice_buffers[port_type][port_num];
            if ((buffer != NULL) && !Work_buffer_resize(buffer, new_size))
                return false;
        }
    }

    if (proc_state->set_audio_buffer_size != NULL)
        return proc_state->set_audio_buffer_size(dstate, new_size);

    return true;
}


void Proc_state_deinit(Device_state* dstate)
{
    assert(dstate != NULL);

    Proc_state* proc_state = (Proc_state*)dstate;

    for (Device_port_type port_type = DEVICE_PORT_TYPE_RECEIVE;
            port_type < DEVICE_PORT_TYPES; ++port_type)
    {
        for (int port_num = 0; port_num < KQT_DEVICE_PORTS_MAX; ++port_num)
            del_Work_buffer(proc_state->voice_buffers[port_type][port_num]);
    }

    del_Bit_array(proc_state->voice_out_buffers_modified);

    return;
}


