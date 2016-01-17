

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


#include <init/devices/processors/Proc_stream.h>

#include <debug/assert.h>
#include <init/devices/Device_impl.h>
#include <memory.h>
#include <player/devices/processors/Stream_state.h>

#include <stdlib.h>


static Set_float_func   Proc_stream_set_default_value;


static void del_Proc_stream(Device_impl* dimpl);


Device_impl* new_Proc_stream(void)
{
    Proc_stream* stream = memory_alloc_item(Proc_stream);
    if (stream == NULL)
        return NULL;

    stream->def_value = 0;

    if (!Device_impl_init(&stream->parent, del_Proc_stream))
    {
        del_Device_impl(&stream->parent);
        return NULL;
    }

    stream->parent.create_pstate = new_Stream_pstate;
    stream->parent.get_vstate_size = Stream_vstate_get_size;
    stream->parent.init_vstate = Stream_vstate_init;

    // Register key handlers
    bool reg_success = true;

#define REGISTER_KEY(type, field, key, def_val)                             \
    reg_success &= Device_impl_register_set_ ## type(                       \
            &stream->parent, key, def_val, Proc_stream_set_ ## field, NULL)

    REGISTER_KEY(float, default_value,  "p_f_default_value.json",   0.0);

#undef REGISTER_KEY

    if (!reg_success)
    {
        del_Device_impl(&stream->parent);
        return NULL;
    }

    return &stream->parent;
}


static bool Proc_stream_set_default_value(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    assert(dimpl != NULL);
    assert(indices != NULL);

    Proc_stream* stream = (Proc_stream*)dimpl;
    stream->def_value = isfinite(value) ? value : 0.0;

    return true;
}


static void del_Proc_stream(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_stream* stream = (Proc_stream*)dimpl;
    memory_free(stream);

    return;
}


