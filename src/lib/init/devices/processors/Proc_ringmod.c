

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/processors/Proc_ringmod.h>

#include <debug/assert.h>
#include <init/devices/Device_impl.h>
#include <init/devices/Processor.h>
#include <memory.h>
#include <player/devices/processors/Ringmod_state.h>

#include <stdbool.h>
#include <stdlib.h>


static void del_Proc_ringmod(Device_impl* dimpl);


Device_impl* new_Proc_ringmod(void)
{
    Proc_ringmod* ringmod = memory_alloc_item(Proc_ringmod);
    if (ringmod == NULL)
        return NULL;

    if (!Device_impl_init(&ringmod->parent, del_Proc_ringmod))
    {
        del_Device_impl(&ringmod->parent);
        return NULL;
    }

    ringmod->parent.create_pstate = new_Ringmod_pstate;
    ringmod->parent.init_vstate = Ringmod_vstate_init;

    return &ringmod->parent;
}


static void del_Proc_ringmod(Device_impl* dimpl)
{
    if (dimpl == NULL)
        return;

    Proc_ringmod* ringmod = (Proc_ringmod*)dimpl;
    memory_free(ringmod);

    return;
}


