

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdbool.h>
#include <stdlib.h>

#include <Connections.h>
#include <Connections_search.h>
#include <Device.h>
#include <DSP_table.h>
#include <Effect.h>
#include <Effect_interface.h>
#include <memory.h>
#include <player/Device_states.h>
#include <player/Effect_state.h>
#include <xassert.h>


struct Effect
{
    Device parent;

    Effect_interface* out_iface;
    Effect_interface* in_iface;
    Connections* connections;
    DSP_table* dsps;
};


static Device_state* Effect_create_state(
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size);

static void Effect_reset(Device* device, Device_states* dstates);

static bool Effect_set_mix_rate(Device* device, uint32_t mix_rate);

static bool Effect_set_buffer_size(Device* device, uint32_t size);

static bool Effect_sync(Device* device, Device_states* dstates);

static void Effect_process(
        Device* device,
        Device_states* states,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
        double tempo);


Effect* new_Effect(uint32_t buf_len, uint32_t mix_rate)
{
    assert(buf_len > 0);
    assert(mix_rate > 0);

    Effect* eff = memory_alloc_item(Effect);
    if (eff == NULL)
        return NULL;

    eff->out_iface = NULL;
    eff->in_iface = NULL;
    eff->connections = NULL;
    eff->dsps = NULL;

    if (!Device_init(&eff->parent, buf_len, mix_rate))
    {
        del_Effect(eff);
        return NULL;
    }

    Device_set_state_creator(&eff->parent, Effect_create_state);
    Device_set_reset(&eff->parent, Effect_reset);
    Device_set_mix_rate_changer(&eff->parent, Effect_set_mix_rate);
    Device_set_buffer_size_changer(&eff->parent, Effect_set_buffer_size);
    Device_set_sync(&eff->parent, Effect_sync);
    Device_set_process(&eff->parent, Effect_process);

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Device_register_port(&eff->parent, DEVICE_PORT_TYPE_RECEIVE, port);
        Device_register_port(&eff->parent, DEVICE_PORT_TYPE_SEND, port);
    }

    eff->out_iface = new_Effect_interface(buf_len, mix_rate);
    eff->in_iface = new_Effect_interface(buf_len, mix_rate);
    eff->dsps = new_DSP_table(KQT_DSPS_MAX);
    if (eff->out_iface == NULL || eff->in_iface == NULL || eff->dsps == NULL)
    {
        del_Effect(eff);
        return NULL;
    }

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Device_register_port(&eff->out_iface->parent,
                             DEVICE_PORT_TYPE_SEND, port);
        Device_register_port(&eff->in_iface->parent,
                             DEVICE_PORT_TYPE_SEND, port);
    }
    Device_register_port(&eff->out_iface->parent,
                         DEVICE_PORT_TYPE_RECEIVE, 0);

    //fprintf(stderr, "New effect %p\n", (void*)eff);

    return eff;
}


#if 0
bool Effect_parse_header(Effect* eff, char* str, Read_state* state)
{
    assert(eff != NULL);
    assert(state != NULL);

    if (state->error)
        return false;

    (void)eff;
    (void)str;
    assert(false);

    return false;
}
#endif


DSP* Effect_get_dsp(Effect* eff, int index)
{
    assert(eff != NULL);
    assert(index >= 0);
    assert(index < KQT_DSPS_MAX);

    return DSP_table_get_dsp(eff->dsps, index);
}


DSP_table* Effect_get_dsps(Effect* eff)
{
    assert(eff != NULL);
    return eff->dsps;
}


void Effect_set_connections(Effect* eff, Connections* graph)
{
    assert(eff != NULL);

    //fprintf(stderr, "Set new connections for %p: %p\n", (void*)eff, (void*)graph);
    del_Connections(eff->connections);
    eff->connections = graph;

    return;
}


bool Effect_prepare_connections(Effect* eff, Device_states* states)
{
    assert(eff != NULL);
    assert(states != NULL);

    if (eff->connections == NULL)
        return true;

    return Connections_prepare(eff->connections, states);
}


Device* Effect_get_input_interface(Effect* eff)
{
    assert(eff != NULL);
    return &eff->in_iface->parent;
}


Device* Effect_get_output_interface(Effect* eff)
{
    assert(eff != NULL);
    return &eff->out_iface->parent;
}


static Device_state* Effect_create_state(
        const Device* device,
        int32_t audio_rate,
        int32_t audio_buffer_size)
{
    assert(device != NULL);
    assert(audio_rate > 0);
    assert(audio_buffer_size >= 0);

    Effect_state* es = memory_alloc_item(Effect_state);
    if (es == NULL)
        return NULL;

    Device_state_init(&es->parent, device, audio_rate, audio_buffer_size);
    Effect_state_reset(es);

    return &es->parent;
}


static void Effect_reset(Device* device, Device_states* dstates)
{
    assert(device != NULL);
    assert(dstates != NULL);

    // Reset DSPs
    Effect* eff = (Effect*)device;
    for (int i = 0; i < KQT_DSPS_MAX; ++i)
    {
        DSP* dsp = DSP_table_get_dsp(eff->dsps, i);
        if (dsp != NULL)
            Device_reset((Device*)dsp, dstates);
    }

    // Reset Effect state
    Effect_state* eff_state = (Effect_state*)Device_states_get_state(
            dstates,
            Device_get_id(device));
    Effect_state_reset(eff_state);

    return;
}


static bool Effect_set_mix_rate(Device* device, uint32_t mix_rate)
{
    assert(device != NULL);
    assert(mix_rate > 0);

    Effect* eff = (Effect*)device;
    for (int i = 0; i < KQT_DSPS_MAX; ++i)
    {
        DSP* dsp = DSP_table_get_dsp(eff->dsps, i);
        if (dsp != NULL && !Device_set_mix_rate((Device*)dsp, mix_rate))
            return false;
    }

    return true;
}


static bool Effect_set_buffer_size(Device* device, uint32_t size)
{
    assert(device != NULL);
    assert(size > 0);
    assert(size <= KQT_AUDIO_BUFFER_SIZE_MAX);

    Effect* eff = (Effect*)device;
    if (!Device_set_buffer_size((Device*)eff->out_iface, size) ||
            !Device_set_buffer_size((Device*)eff->in_iface, size))
        return false;

    for (int i = 0; i < KQT_DSPS_MAX; ++i)
    {
        DSP* dsp = DSP_table_get_dsp(eff->dsps, i);
        if (dsp != NULL && !Device_set_buffer_size((Device*)dsp, size))
            return false;
    }

    return true;
}


static bool Effect_sync(Device* device, Device_states* dstates)
{
    assert(device != NULL);
    assert(dstates != NULL);

    Effect* eff = (Effect*)device;
    for (int i = 0; i < KQT_DSPS_MAX; ++i)
    {
        DSP* dsp = DSP_table_get_dsp(eff->dsps, i);
        if (dsp != NULL && !Device_sync((Device*)dsp, dstates))
            return false;
    }

    return true;
}


static void mix_interface_connection(
        Device_state* ds,
        const Device_state* in_ds,
        uint32_t start,
        uint32_t stop)
{
    assert(ds != NULL);
    assert(in_ds != NULL);

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        Audio_buffer* out = Device_state_get_audio_buffer(
                ds, DEVICE_PORT_TYPE_SEND, port);
        const Audio_buffer* in = Device_state_get_audio_buffer(
                in_ds, DEVICE_PORT_TYPE_RECEIVE, port);

        if (in != NULL && out != NULL)
            Audio_buffer_mix(out, in, start, stop);
    }

    return;
}


static void Effect_process(
        Device* device,
        Device_states* states,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
        double tempo)
{
    assert(device != NULL);
    assert(states != NULL);
    assert(start < device->buffer_size);
    assert(until <= device->buffer_size);
    assert(freq > 0);
    assert(isfinite(tempo));

    Effect_state* eff_state = (Effect_state*)Device_states_get_state(
            states,
            Device_get_id(device));

    Effect* eff = (Effect*)device;

    if (eff_state->bypass)
    {
        Device_state* ds = Device_states_get_state(
                states, Device_get_id((Device*)device));

        mix_interface_connection(ds, ds, start, until);
    }
    else if (eff->connections != NULL)
    {
#ifndef NDEBUG
        static bool in_effect = false;
        assert(!in_effect);
        in_effect = true;
#endif
        Connections_clear_buffers(eff->connections, states, start, until);

        // Fill input interface buffers
        Device_state* ds = Device_states_get_state(
                states, Device_get_id((Device*)device));
        Device_state* in_iface_ds = Device_states_get_state(
                states, Device_get_id(Effect_get_input_interface(eff)));
        mix_interface_connection(in_iface_ds, ds, start, until);

        // Process effect graph
        Connections_mix(eff->connections, states, start, until, freq, tempo);

        // Fill output interface buffers
        Device_state* out_iface_ds = Device_states_get_state(
                states, Device_get_id(Effect_get_output_interface(eff)));
        mix_interface_connection(ds, out_iface_ds, start, until);
#ifndef NDEBUG
        in_effect = false;
#endif
    }

    return;
}


void del_Effect(Effect* eff)
{
    if (eff == NULL)
        return;

    del_Effect_interface(eff->out_iface);
    del_Effect_interface(eff->in_iface);
    del_Connections(eff->connections);
    del_DSP_table(eff->dsps);
    Device_deinit(&eff->parent);
    memory_free(eff);

    return;
}


