

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2018
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
    rassert(device != NULL);

    static uint32_t id = 1;
    device->id = id;
    ++id;

    device->existent = false;
    device->req_impl = req_impl;

    device->enable_signal_support = false;

    device->dparams = NULL;
    device->dimpl = NULL;

    device->create_state = new_Device_state_plain;

    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        for (Device_port_type type = DEVICE_PORT_TYPE_RECV;
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
    rassert(device != NULL);
    return device->id;
}


bool Device_has_complete_type(const Device* device)
{
    rassert(device != NULL);
    return !device->req_impl || (device->dimpl != NULL);
}


void Device_set_existent(Device* device, bool existent)
{
    rassert(device != NULL);
    device->existent = existent;
    return;
}


bool Device_is_existent(const Device* device)
{
    rassert(device != NULL);
    return device->existent;
}


void Device_set_impl(Device* device, Device_impl* dimpl)
{
    rassert(device != NULL);
    rassert(dimpl != NULL);

    del_Device_impl(device->dimpl);

    Device_impl_set_device(dimpl, device);
    device->dimpl = dimpl;

    return;
}


const Device_impl* Device_get_impl(const Device* device)
{
    rassert(device != NULL);
    return device->dimpl;
}


Device_state* Device_create_state(
        const Device* device, int32_t audio_rate, int32_t buffer_size)
{
    rassert(device != NULL);
    rassert(device->create_state != NULL);
    rassert(audio_rate > 0);
    rassert(buffer_size >= 0);

    return device->create_state(device, audio_rate, buffer_size);
}


void Device_set_state_creator(
        Device* device, Device_state* (*creator)(const Device*, int32_t, int32_t))
{
    rassert(device != NULL);

    if (creator != NULL)
        device->create_state = creator;
    else
        device->create_state = new_Device_state_plain;

    return;
}


void Device_set_mixed_signals(Device* device, bool enabled)
{
    rassert(device != NULL);
    device->enable_signal_support = enabled;
    return;
}


bool Device_get_mixed_signals(const Device* device)
{
    rassert(device != NULL);
    return device->enable_signal_support;
}


void Device_set_port_existence(
        Device* device, Device_port_type type, int port, bool exists)
{
    rassert(device != NULL);
    rassert(type < DEVICE_PORT_TYPES);
    rassert(port >= 0);
    rassert(port < KQT_DEVICE_PORTS_MAX);

    device->existence[type][port] = exists;

    return;
}


bool Device_get_port_existence(const Device* device, Device_port_type type, int port)
{
    rassert(device != NULL);
    rassert(type < DEVICE_PORT_TYPES);
    rassert(port >= 0);
    rassert(port < KQT_DEVICE_PORTS_MAX);

    return device->existence[type][port];
}


bool Device_sync(Device* device)
{
    rassert(device != NULL);

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
    rassert(device != NULL);
    rassert(dstates != NULL);

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


bool Device_set_key(Device* device, const char* key, int version, Streader* sr)
{
    rassert(device != NULL);
    rassert(key != NULL);
    rassert(string_has_prefix(key, "i/") || string_has_prefix(key, "c/"));
    rassert(version >= 0);
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    if (!Device_params_parse_value(device->dparams, key, version, sr))
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
    rassert(device != NULL);
    rassert(dstates != NULL);
    rassert(key != NULL);
    rassert(string_has_prefix(key, "i/") || string_has_prefix(key, "c/"));

    if (device->dimpl != NULL)
    {
        Device_state* dstate = Device_states_get_state(
                dstates, Device_get_id(device));
        return Device_impl_set_state_key(device->dimpl, dstate, key + 2);
    }

    return true;
}


void Device_print(const Device* device, FILE* out)
{
    rassert(device != NULL);
    rassert(out != NULL);

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
        if (!device->existence[DEVICE_PORT_TYPE_RECV][port])
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
        device->existence[DEVICE_PORT_TYPE_RECV][port] = false;
        device->existence[DEVICE_PORT_TYPE_SEND][port] = false;
    }

    return;
}


