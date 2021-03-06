

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/processors/Proc_ks.h>

#include <debug/assert.h>
#include <init/devices/Proc_cons.h>
#include <init/devices/processors/Proc_init_utils.h>
#include <memory.h>
#include <player/devices/processors/Ks_state.h>

#include <stdlib.h>
#include <string.h>


static Set_float_func       Proc_ks_set_damp;

static Device_impl_get_voice_wb_size_func Proc_ks_get_voice_wb_size;

static Device_impl_destroy_func del_Proc_ks;


Device_impl* new_Proc_ks(void)
{
    Proc_ks* ks = memory_alloc_item(Proc_ks);
    if (ks == NULL)
        return NULL;

    ks->damp = KS_DEFAULT_DAMP;

    if (!Device_impl_init(&ks->parent, del_Proc_ks))
    {
        del_Device_impl(&ks->parent);
        return NULL;
    }

#define REG_KEY(type, name, keyp, def_value) \
    REGISTER_SET_FIXED_STATE(ks, type, name, keyp, def_value)

    if (!(REG_KEY(float, damp, "p_f_damp.json", KS_DEFAULT_DAMP)))
    {
        del_Device_impl(&ks->parent);
        return NULL;
    }

#undef REG_KEY

    ks->parent.get_voice_wb_size = Proc_ks_get_voice_wb_size;
    ks->parent.get_vstate_size = Ks_vstate_get_size;
    ks->parent.init_vstate = Ks_vstate_init;
    ks->parent.render_voice = Ks_vstate_render_voice;

    return &ks->parent;
}


static bool Proc_ks_set_damp(
        Device_impl* dimpl, const Key_indices indices, double value)
{
    rassert(dimpl != NULL);
    ignore(indices);

    Proc_ks* ks = (Proc_ks*)dimpl;

    if (value >= KS_MIN_DAMP && value <= KS_MAX_DAMP)
        ks->damp = value;
    else
        ks->damp = KS_DEFAULT_DAMP;

    return true;
}


static int32_t Proc_ks_get_voice_wb_size(const Device_impl* dimpl, int32_t audio_rate)
{
    rassert(dimpl != NULL);
    rassert(audio_rate > 0);

    static const double min_freq = 10;

    return (int32_t)(audio_rate / min_freq) + 1;
}


static void del_Proc_ks(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_ks* ks = (Proc_ks*)dimpl;
    memory_free(ks);

    return;
}


