

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/Device.h>

#include <debug/assert.h>
#include <init/devices/Device_impl.h>
#include <mathnum/common.h>
#include <string/common.h>
#include <Value.h>

#include <inttypes.h>
#include <math.h>
#include <stdlib.h>


bool Device_init(Device* device, bool req_impl)
{
    assert(device != NULL);

    static uint32_t id = 1;
    device->id = id;
    ++id;

    device->existent = false;
    device->req_impl = req_impl;

    device->enable_signal_support = false;

    device->dparams = NULL;
    device->dimpl = NULL;

    device->create_state = new_Device_state_plain;

    device->set_control_var_generic = NULL;
    device->init_control_vars = NULL;

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        for (Device_port_type type = DEVICE_PORT_TYPE_RECEIVE;
                type < DEVICE_PORT_TYPES; ++type)
            device->existence[type][port] = false;
    }

    device->dparams = new_Device_params();
    if (device->dparams == NULL)
    {
        Device_deinit(device);
        return false;
    }

    return true;
}


uint32_t Device_get_id(const Device* device)
{
    assert(device != NULL);
    return device->id;
}


bool Device_has_complete_type(const Device* device)
{
    assert(device != NULL);
    return !device->req_impl || (device->dimpl != NULL);
}


void Device_set_existent(Device* device, bool existent)
{
    assert(device != NULL);
    device->existent = existent;
    return;
}


bool Device_is_existent(const Device* device)
{
    assert(device != NULL);
    return device->existent;
}


void Device_set_impl(Device* device, Device_impl* dimpl)
{
    assert(device != NULL);
    assert(dimpl != NULL);

    del_Device_impl(device->dimpl);

    Device_impl_set_device(dimpl, device);
    device->dimpl = dimpl;

    return;
}


const Device_impl* Device_get_impl(const Device* device)
{
    assert(device != NULL);
    return device->dimpl;
}


Device_state* Device_create_state(
        const Device* device, int32_t audio_rate, int32_t buffer_size)
{
    assert(device != NULL);
    assert(device->create_state != NULL);
    assert(audio_rate > 0);
    assert(buffer_size >= 0);

    return device->create_state(device, audio_rate, buffer_size);
}


void Device_set_state_creator(
        Device* device, Device_state* (*creator)(const Device*, int32_t, int32_t))
{
    assert(device != NULL);

    if (creator != NULL)
        device->create_state = creator;
    else
        device->create_state = new_Device_state_plain;

    return;
}


void Device_set_mixed_signals(Device* device, bool enabled)
{
    assert(device != NULL);
    device->enable_signal_support = enabled;
    return;
}


bool Device_get_mixed_signals(const Device* device)
{
    assert(device != NULL);
    return device->enable_signal_support;
}


void Device_register_set_control_var_generic(
        Device* device, Device_set_control_var_generic_func* set_func)
{
    assert(device != NULL);
    assert(set_func != NULL);

    device->set_control_var_generic = set_func;

    return;
}


void Device_register_init_control_vars(
        Device* device, Device_init_control_vars_func* init_func)
{
    assert(device != NULL);
    assert(init_func != NULL);

    device->init_control_vars = init_func;

    return;
}


void Device_set_port_existence(
        Device* device, Device_port_type type, int port, bool exists)
{
    assert(device != NULL);
    assert(type < DEVICE_PORT_TYPES);
    assert(port >= 0);
    assert(port < KQT_DEVICE_PORTS_MAX);

    device->existence[type][port] = exists;

    return;
}


bool Device_get_port_existence(const Device* device, Device_port_type type, int port)
{
    assert(device != NULL);
    assert(type < DEVICE_PORT_TYPES);
    assert(port >= 0);
    assert(port < KQT_DEVICE_PORTS_MAX);

    return device->existence[type][port];
}


bool Device_sync(Device* device)
{
    assert(device != NULL);

    // Set existing keys on dimpl
    if (device->dimpl != NULL)
    {
        Device_params_iter* iter = Device_params_iter_init(
                DEVICE_PARAMS_ITER_AUTO, device->dparams);

        const char* key = Device_params_iter_get_next_key(iter);
        while (key != NULL)
        {
            if (!Device_impl_set_key(device->dimpl, key))
                return false;

            key = Device_params_iter_get_next_key(iter);
        }
    }

    return true;
}


bool Device_sync_states(const Device* device, Device_states* dstates)
{
    assert(device != NULL);
    assert(dstates != NULL);

    if (device->dimpl != NULL)
    {
        Device_state* dstate = Device_states_get_state(
                dstates, Device_get_id(device));

        Device_params_iter* iter = Device_params_iter_init(
                DEVICE_PARAMS_ITER_AUTO, device->dparams);

        const char* key = Device_params_iter_get_next_key(iter);
        while (key != NULL)
        {
            if (!Device_impl_set_state_key(device->dimpl, dstate, key))
                return false;

            key = Device_params_iter_get_next_key(iter);
        }
    }

    return true;
}


bool Device_set_key(Device* device, const char* key, Streader* sr)
{
    assert(device != NULL);
    assert(key != NULL);
    assert(string_has_prefix(key, "i/") || string_has_prefix(key, "c/"));
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    if (!Device_params_parse_value(device->dparams, key, sr))
        return false;

    if (device->dimpl != NULL && !Device_impl_set_key(device->dimpl, key + 2))
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for device key %s", key);
        return false;
    }

    return true;
}


bool Device_set_state_key(
        const Device* device,
        Device_states* dstates,
        const char* key)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(key != NULL);
    assert(string_has_prefix(key, "i/") || string_has_prefix(key, "c/"));

    if (device->dimpl != NULL)
    {
        Device_state* dstate = Device_states_get_state(
                dstates, Device_get_id(device));
        return Device_impl_set_state_key(device->dimpl, dstate, key + 2);
    }

    return true;
}


void Device_set_control_var_generic(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Random* random,
        Channel* channel,
        const char* var_name,
        const Value* value)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(random != NULL);
    assert(implies(mode == DEVICE_CONTROL_VAR_MODE_VOICE, channel != NULL));
    assert(var_name != NULL);
    assert(value != NULL);
    assert(Value_type_is_realtime(value->type));

    if (device->set_control_var_generic != NULL)
        device->set_control_var_generic(
                device, dstates, mode, random, channel, var_name, value);

    return;
}


void Device_init_control_vars(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Random* random,
        Channel* channel)
{
    assert(device != NULL);
    assert(dstates != NULL);
    assert(implies(mode == DEVICE_CONTROL_VAR_MODE_MIXED, random != NULL));
    assert(implies(mode == DEVICE_CONTROL_VAR_MODE_VOICE, channel != NULL));

    if (device->init_control_vars != NULL)
        device->init_control_vars(device, dstates, mode, random, channel);

    return;
}


void Device_print(const Device* device, FILE* out)
{
    assert(device != NULL);
    assert(out != NULL);

    bool printed = false;
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        if (!device->existence[DEVICE_PORT_TYPE_SEND][port])
            continue;

        if (!printed)
        {
            fprintf(out, "Registered send ports:\n");
            printed = true;
        }

        fprintf(out, "  Port %02x\n", port);
    }

    printed = false;
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        if (!device->existence[DEVICE_PORT_TYPE_RECEIVE][port])
            continue;

        if (!printed)
        {
            fprintf(out, "Registered receive ports:\n");
            printed = true;
        }

        fprintf(out, "  Port %02x\n", port);
    }

    return;
}


void Device_deinit(Device* device)
{
    if (device == NULL)
        return;

    del_Device_impl(device->dimpl);
    device->dimpl = NULL;

    del_Device_params(device->dparams);
    device->dparams = NULL;

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        device->existence[DEVICE_PORT_TYPE_RECEIVE][port] = false;
        device->existence[DEVICE_PORT_TYPE_SEND][port] = false;
    }

    return;
}


