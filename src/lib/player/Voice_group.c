

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


#include <player/Voice_group.h>

#include <debug/assert.h>
#include <devices/Device.h>
#include <player/Voice.h>

#include <stdint.h>
#include <stdlib.h>


Voice_group* Voice_group_init(
        Voice_group* vg, Voice** voices, uint16_t offset, uint16_t vp_size)
{
    assert(vg != NULL);
    assert(voices != NULL);
    assert(offset < vp_size);

    vg->voices = voices + offset;

    const uint64_t group_id = Voice_get_group_id(voices[offset]);
    if (group_id == 0)
    {
        vg->size = 0;
        return vg;
    }

    vg->size = 1;

    for (uint16_t i = offset + 1; i < vp_size; ++i)
    {
        assert(voices[i] != NULL);
        if (Voice_get_group_id(voices[i]) != group_id)
            break;
        ++vg->size;
    }

    for (uint16_t i = 0; i < vg->size; ++i)
        vg->voices[i]->updated = false;

    return vg;
}


uint16_t Voice_group_get_size(const Voice_group* vg)
{
    assert(vg != NULL);
    return vg->size;
}


Voice* Voice_group_get_voice(Voice_group* vg, uint16_t index)
{
    assert(vg != NULL);
    assert(index < Voice_group_get_size(vg));

    return vg->voices[index];
}


Voice* Voice_group_get_voice_by_proc(Voice_group* vg, uint32_t proc_id)
{
    assert(vg != NULL);

    for (uint16_t i = 0; i < vg->size; ++i)
    {
        const Processor* proc = Voice_get_proc(vg->voices[i]);
        if ((proc != NULL) && (Device_get_id((const Device*)proc) == proc_id))
            return vg->voices[i];
    }

    return NULL;
}


void Voice_group_deactivate_unreachable(Voice_group* vg)
{
    assert(vg != NULL);

    for (uint16_t i = 0; i < vg->size; ++i)
    {
        Voice* voice = vg->voices[i];
        if (!voice->updated)
            Voice_reset(voice);
    }

    return;
}


