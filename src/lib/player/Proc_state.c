

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdbool.h>

#include <debug/assert.h>
#include <player/Proc_state.h>


static bool Proc_state_add_buffer(
        Device_state* dstate, Device_port_type port_type, int port_num);

static bool Proc_state_resize_buffers(Device_state* dstate, int32_t new_size);


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

    Device_state_init(&proc_state->parent, device, audio_rate, audio_buffer_size);

    for (Device_port_type port_type = DEVICE_PORT_TYPE_RECEIVE;
            port_type < DEVICE_PORT_TYPES; ++port_type)
    {
        for (int port_num = 0; port_num < KQT_DEVICE_PORTS_MAX; ++port_num)
            proc_state->voice_buffers[port_type][port_num] = NULL;
    }

    proc_state->voice_out_buffers_modified = NULL;

    proc_state->parent.add_buffer = Proc_state_add_buffer;
    proc_state->parent.resize_buffers = Proc_state_resize_buffers;
    proc_state->parent.deinit = Proc_state_deinit;

    proc_state->voice_out_buffers_modified = new_Bit_array(KQT_DEVICE_PORTS_MAX);
    if (proc_state->voice_out_buffers_modified == NULL)
    {
        Proc_state_deinit(&proc_state->parent);
        return false;
    }

    return true;
}


void Proc_state_reset(Proc_state* proc_state)
{
    assert(proc_state != NULL);

    Device_state_reset(&proc_state->parent);

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
            Audio_buffer* buffer = proc_state->voice_buffers[port_type][port_num];
            if (buffer != NULL)
                Audio_buffer_clear(buffer, 0, Audio_buffer_get_size(buffer));
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


const Audio_buffer* Proc_state_get_voice_buffer(
        const Proc_state* proc_state, Device_port_type port_type, int port_num)
{
    assert(proc_state != NULL);
    assert(port_type == DEVICE_PORT_TYPE_RECEIVE || port_type == DEVICE_PORT_TYPE_SEND);
    assert(port_num >= 0);
    assert(port_num < KQT_DEVICE_PORTS_MAX);

    return proc_state->voice_buffers[port_type][port_num];
}


Audio_buffer* Proc_state_get_voice_buffer_mut(
        Proc_state* proc_state, Device_port_type port_type, int port_num)
{
    assert(proc_state != NULL);
    assert(port_type == DEVICE_PORT_TYPE_RECEIVE || port_type == DEVICE_PORT_TYPE_SEND);
    assert(port_num >= 0);
    assert(port_num < KQT_DEVICE_PORTS_MAX);

    if (port_type == DEVICE_PORT_TYPE_SEND)
        Bit_array_set(proc_state->voice_out_buffers_modified, port_num, true);

    return proc_state->voice_buffers[port_type][port_num];
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

    proc_state->voice_buffers[port_type][port_num] = new_Audio_buffer(
            dstate->audio_buffer_size);
    if (proc_state->voice_buffers[port_type][port_num] == NULL)
        return false;

    return true;
}


static bool Proc_state_resize_buffers(Device_state* dstate, int32_t new_size)
{
    assert(dstate != NULL);
    assert(new_size >= 0);

    Proc_state* proc_state = (Proc_state*)dstate;

    for (Device_port_type port_type = DEVICE_PORT_TYPE_RECEIVE;
            port_type < DEVICE_PORT_TYPES; ++port_type)
    {
        for (int port_num = 0; port_num < KQT_DEVICE_PORTS_MAX; ++port_num)
        {
            Audio_buffer* buffer = proc_state->voice_buffers[port_type][port_num];
            if ((buffer != NULL) && !Audio_buffer_resize(buffer, new_size))
                return false;
        }
    }

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
            del_Audio_buffer(proc_state->voice_buffers[port_type][port_num]);
    }

    del_Bit_array(proc_state->voice_out_buffers_modified);

    return;
}

