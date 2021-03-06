

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


#include <player/Voice.h>

#include <debug/assert.h>
#include <init/devices/Device_impl.h>
#include <mathnum/common.h>
#include <memory.h>
#include <player/devices/Voice_state.h>

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


Voice* new_Voice(void)
{
    Voice* voice = memory_alloc_item(Voice);
    if (voice == NULL)
        return NULL;

    voice->id = 0;
    voice->group_id = 0;
    voice->ch_num = -1;
    voice->updated = false;
    voice->prio = VOICE_PRIO_INACTIVE;
    voice->use_test_output = false;
    voice->test_proc_index = -1;
    voice->proc = NULL;
    voice->state_size = 0;
    voice->state = NULL;
    voice->wb = NULL;

    voice->state_size = sizeof(Voice_state);
    voice->state = memory_alloc_item(Voice_state);
    if (voice->state == NULL)
    {
        del_Voice(voice);
        return NULL;
    }

    Random_init(&voice->rand_p, "vp");
    Random_init(&voice->rand_s, "vs");
    Voice_state_clear(voice->state);

    return voice;
}


bool Voice_reserve_state_space(Voice* voice, int32_t state_size)
{
    rassert(voice != NULL);
    rassert(state_size >= 0);

    if (state_size <= voice->state_size)
        return true;

    Voice_state* new_state = memory_realloc_items(char, state_size, voice->state);
    if (new_state == NULL)
        return false;

    voice->state_size = state_size;
    voice->state = new_state;

    return true;
}


int Voice_cmp(const Voice* v1, const Voice* v2)
{
    rassert(v1 != NULL);
    rassert(v2 != NULL);

    return (int)v1->prio - (int)v2->prio;
}


uint64_t Voice_id(const Voice* voice)
{
    rassert(voice != NULL);
    return voice->id;
}


uint64_t Voice_get_group_id(const Voice* voice)
{
    rassert(voice != NULL);
    return voice->group_id;
}


int Voice_get_ch_num(const Voice* voice)
{
    rassert(voice != NULL);
    return voice->ch_num;
}


const Processor* Voice_get_proc(const Voice* voice)
{
    rassert(voice != NULL);
    return voice->proc;
}


void Voice_set_work_buffer(Voice* voice, Work_buffer* wb)
{
    rassert(voice != NULL);
    voice->wb = wb;
    return;
}


void Voice_init(
        Voice* voice,
        const Processor* proc,
        uint64_t group_id,
        int ch_num,
        const Proc_state* proc_state,
        uint64_t seed)
{
    rassert(voice != NULL);
    rassert(proc != NULL);
    rassert(proc_state != NULL);
    rassert(ch_num >= -1);
    rassert(ch_num < KQT_CHANNELS_MAX);

    voice->prio = VOICE_PRIO_NEW;
    voice->proc = proc;
    voice->group_id = group_id;
    voice->ch_num = ch_num;
    voice->use_test_output = false;
    voice->test_proc_index = -1;
    Random_set_seed(&voice->rand_p, seed);
    Random_set_seed(&voice->rand_s, seed);

    Proc_type proc_type = Device_impl_get_proc_type(proc->parent.dimpl);

    Voice_state_init(voice->state, proc_type, &voice->rand_p, &voice->rand_s);
    Voice_state_set_work_buffer(voice->state, voice->wb);

    const Device_impl* dimpl = Device_get_impl((const Device*)proc);
    if ((dimpl != NULL) && (dimpl->init_vstate != NULL))
        dimpl->init_vstate(voice->state, proc_state);

    return;
}


void Voice_set_test_processor(Voice* voice, int proc_index)
{
    rassert(voice != NULL);
    rassert(proc_index >= 0);
    rassert(proc_index < KQT_PROCESSORS_MAX);

    voice->use_test_output = true;
    voice->test_proc_index = proc_index;

    return;
}


void Voice_set_test_processor_param(Voice* voice, const char* param)
{
    rassert(voice != NULL);
    rassert(param != NULL);
    rassert(strlen(param) <= KQT_VAR_NAME_MAX);

    strncpy(voice->state->test_proc_param, param, KQT_VAR_NAME_MAX);

    return;
}


bool Voice_is_using_test_output(const Voice* voice)
{
    rassert(voice != NULL);
    return voice->use_test_output;
}


int Voice_get_test_proc_index(const Voice* voice)
{
    rassert(voice != NULL);
    rassert(Voice_is_using_test_output(voice));

    return voice->test_proc_index;
}


void Voice_reset(Voice* voice)
{
    rassert(voice != NULL);

    voice->id = 0;
    voice->group_id = 0;
    // The voice may be part of an active group that needs the channel
    // number information, so let's keep it
    //voice->ch_num = -1;
    voice->prio = VOICE_PRIO_INACTIVE;
    Voice_state_clear(voice->state);
    voice->proc = NULL;
    Random_reset(&voice->rand_p);
    Random_reset(&voice->rand_s);

    return;
}


int32_t Voice_render(
        Voice* voice,
        uint32_t proc_id,
        Device_states* dstates,
        int thread_id,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo)
{
    rassert(implies(voice != NULL, voice->proc != NULL));
    rassert(proc_id > 0);
    rassert(dstates != NULL);
    rassert(thread_id >= 0);
    rassert(thread_id < KQT_THREADS_MAX);
    rassert(wbs != NULL);
    rassert(buf_start >= 0);
    rassert(buf_stop >= buf_start);

    if ((voice != NULL) && (voice->prio == VOICE_PRIO_INACTIVE))
        return buf_start;

    Proc_state* pstate = (Proc_state*)Device_states_get_state(dstates, proc_id);
    const Processor* proc = (const Processor*)pstate->parent.device;
    Device_thread_state* proc_ts =
        Device_states_get_thread_state(dstates, thread_id, proc_id);
    const Au_state* au_state = (const Au_state*)Device_states_get_state(
            dstates, Processor_get_au_params(proc)->device_id);

    Voice_state* vstate = (voice != NULL) ? voice->state : NULL;

    if (vstate != NULL)
        vstate->keep_alive_stop = 0;

    const int32_t process_stop = Voice_state_render_voice(
            vstate, pstate, proc_ts, au_state, wbs, buf_start, buf_stop, tempo);
    ignore(process_stop); // TODO: not sure if we have any use for this

    int32_t keep_alive_stop = 0;

    if (voice != NULL)
    {
        voice->updated = true;

        if (!voice->state->active)
        {
            Voice_reset(voice);
            return buf_start;
        }

        if (!voice->state->note_on)
            voice->prio = VOICE_PRIO_BG;

        keep_alive_stop = voice->state->keep_alive_stop;
    }

    return clamp(keep_alive_stop, buf_start, buf_stop);
}


void del_Voice(Voice* voice)
{
    if (voice == NULL)
        return;

    memory_free(voice->state);
    memory_free(voice);

    return;
}


