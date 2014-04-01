

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>

#include <Bind.h>
#include <devices/Effect.h>
#include <devices/Generator.h>
#include <Event_names.h>
#include <Event_type.h>
#include <General_state.h>
#include <Ins_table.h>
#include <kunquat/limits.h>
#include <Module.h>
#include <player/Channel.h>
#include <player/Event_handler.h>
#include <Streader.h>
#include <string_common.h>
#include <Value.h>

#include <player/events/Event_control_decl.h>
#include <player/events/Event_general_decl.h>
#include <player/events/Event_master_decl.h>
#include <player/events/Event_channel_decl.h>
#include <player/events/Event_ins_decl.h>
#include <player/events/Event_generator_decl.h>
#include <player/events/Event_effect_decl.h>
#include <player/events/Event_dsp_decl.h>

#include <memory.h>
#include <xassert.h>


struct Event_handler
{
    Channel* channels[KQT_COLUMNS_MAX];
    Device_states* device_states;
    Master_params* master_params;
    Ins_table* insts;
    Effect_table* effects;
    Event_names* event_names;

    bool (*control_process[Event_control_STOP])(General_state*, Value*);
    bool (*general_process[Event_general_STOP])(General_state*, Value*);
    bool (*ch_process[Event_channel_STOP])(Channel*, Device_states*, Value*);
    bool (*master_process[Event_master_STOP])(Master_params*, Value*);
    bool (*ins_process[Event_ins_STOP])(
            const Instrument_params*,
            Ins_state*,
            Value*);
    bool (*generator_process[Event_generator_STOP])(
            const Device_impl*, Device_state*, Channel*, Value*);
    bool (*effect_process[Event_effect_STOP])(
            const Effect*,
            Effect_state*,
            Device_states*,
            Value*);
    bool (*dsp_process[Event_dsp_STOP])(
            const Device_impl*, Device_state*, Channel*, Value*);
};


Event_handler* new_Event_handler(
        Master_params* master_params,
        Channel** channels,
        Device_states* device_states,
        Ins_table* insts,
        Effect_table* effects)
{
    assert(master_params != NULL);
    assert(channels != NULL);
    assert(device_states != NULL);
    assert(insts != NULL);
    assert(effects != NULL);

    Event_handler* eh = memory_alloc_item(Event_handler);
    if (eh == NULL)
        return NULL;

    eh->event_names = new_Event_names();
    if (eh->event_names == NULL)
    {
        del_Event_handler(eh);
        return NULL;
    }

    eh->master_params = master_params;
    for (int i = 0; i < KQT_CHANNELS_MAX; ++i)
        eh->channels[i] = channels[i];
    eh->device_states = device_states;
    eh->insts = insts;
    eh->effects = effects;

#define EVENT_TYPE_DEF(type) Event_handler_set_control_process( \
        eh, Event_control_##type, Event_control_##type##_process);
#include <player/events/Event_control_types.h>

#define EVENT_TYPE_DEF(type) Event_handler_set_general_process( \
        eh, Event_general_##type, Event_general_##type##_process);
#include <player/events/Event_general_types.h>

#define EVENT_TYPE_DEF(type) Event_handler_set_master_process( \
        eh, Event_master_##type, Event_master_##type##_process);
#include <player/events/Event_master_types.h>

#define EVENT_TYPE_DEF(type) Event_handler_set_ch_process( \
        eh, Event_channel_##type, Event_channel_##type##_process);
#include <player/events/Event_channel_types.h>

#define EVENT_TYPE_DEF(type) Event_handler_set_ins_process( \
        eh, Event_ins_##type, Event_ins_##type##_process);
#include <player/events/Event_ins_types.h>

#define EVENT_TYPE_DEF(type) Event_handler_set_generator_process( \
        eh, Event_generator_##type, Event_generator_##type##_process);
#include <player/events/Event_generator_types.h>

#define EVENT_TYPE_DEF(type) Event_handler_set_effect_process( \
        eh, Event_effect_##type, Event_effect_##type##_process);
#include <player/events/Event_effect_types.h>

#define EVENT_TYPE_DEF(type) Event_handler_set_dsp_process( \
        eh, Event_dsp_##type, Event_dsp_##type##_process);
#include <player/events/Event_dsp_types.h>

    return eh;
}


const Event_names* Event_handler_get_names(const Event_handler* eh)
{
    assert(eh != NULL);
    return eh->event_names;
}


bool Event_handler_set_ch_process(
        Event_handler* eh,
        Event_type type,
        bool (*ch_process)(Channel*, Device_states*, Value*))
{
    assert(eh != NULL);
    assert(Event_is_channel(type));
    assert(ch_process != NULL);

    eh->ch_process[type] = ch_process;

    return true;
}


bool Event_handler_set_general_process(
        Event_handler* eh,
        Event_type type,
        bool (*general_process)(General_state*, Value*))
{
    assert(eh != NULL);
    assert(Event_is_general(type));
    assert(general_process != NULL);

    eh->general_process[type] = general_process;

    return true;
}


bool Event_handler_set_control_process(
        Event_handler* eh,
        Event_type type,
        bool (*control_process)(General_state*, Value*))
{
    assert(eh != NULL);
    assert(Event_is_control(type));
    assert(control_process != NULL);

    eh->control_process[type] = control_process;

    return true;
}


bool Event_handler_set_master_process(
        Event_handler* eh,
        Event_type type,
        bool (*global_process)(Master_params*, Value*))
{
    assert(eh != NULL);
    assert(Event_is_master(type));
    assert(global_process != NULL);

    eh->master_process[type] = global_process;

    return true;
}


bool Event_handler_set_ins_process(
        Event_handler* eh,
        Event_type type,
        bool (*ins_process)(const Instrument_params*, Ins_state*, Value*))
{
    assert(eh != NULL);
    assert(Event_is_ins(type));
    assert(ins_process != NULL);

    eh->ins_process[type] = ins_process;

    return true;
}


bool Event_handler_set_generator_process(
        Event_handler* eh,
        Event_type type,
        bool (*gen_process)(
            const Device_impl*, Device_state*, Channel*, Value*))
{
    assert(eh != NULL);
    assert(Event_is_generator(type));
    assert(gen_process != NULL);

    eh->generator_process[type] = gen_process;

    return true;
}


bool Event_handler_set_effect_process(
        Event_handler* eh,
        Event_type type,
        bool (*effect_process)(
            const Effect*, Effect_state*, Device_states*, Value*))
{
    assert(eh != NULL);
    assert(Event_is_effect(type));
    assert(effect_process != NULL);

    eh->effect_process[type] = effect_process;

    return true;
}


bool Event_handler_set_dsp_process(
        Event_handler* eh,
        Event_type type,
        bool (*dsp_process)(
            const Device_impl*, Device_state*, Channel*, Value*))
{
    assert(eh != NULL);
    assert(Event_is_dsp(type));
    assert(dsp_process != NULL);

    eh->dsp_process[type] = dsp_process;

    return true;
}


static bool Event_handler_handle(
        Event_handler* eh,
        int index,
        Event_type type,
        Value* value)
{
    assert(eh != NULL);
    assert(index >= 0);
    assert(index < KQT_COLUMNS_MAX);
    assert(Event_is_valid(type));
    assert(eh->channels[index]->freq != NULL);
    assert(*eh->channels[index]->freq > 0);
    assert(eh->channels[index]->tempo != NULL);
    assert(*eh->channels[index]->tempo > 0);

    if (Event_is_channel(type))
    {
        if (eh->ch_process[type] == NULL)
            return false;

        return eh->ch_process[type](
                eh->channels[index],
                eh->device_states,
                value);
    }
    else if (Event_is_ins(type))
    {
        // Find our instrument
        Instrument* ins = Module_get_ins_from_input(
                eh->master_params->parent.module,
                eh->channels[index]->ins_input);
        if (ins == NULL)
            return false;

        const Instrument_params* ins_params = Instrument_get_params(ins);
        assert(ins_params != NULL);
        Ins_state* ins_state = (Ins_state*)Device_states_get_state(
                eh->device_states,
                Device_get_id((Device*)ins));

        return eh->ins_process[type](ins_params, ins_state, value);
    }
    else if (Event_is_master(type))
    {
        if (eh->master_process[type] == NULL)
            return false;

        return eh->master_process[type](eh->master_params, value);
    }
    else if (Event_is_generator(type))
    {
        // Find our instrument
        Instrument* ins = Module_get_ins_from_input(
                eh->master_params->parent.module,
                eh->channels[index]->ins_input);
        if (ins == NULL)
            return false;

        Device* device = (Device*)Instrument_get_gen(
                ins, eh->channels[index]->generator);
        if (device == NULL)
            return false;

        const Device_impl* dimpl = device->dimpl;
        if (dimpl == NULL)
            return false;

        Device_state* dstate = Device_states_get_state(
                eh->device_states,
                Device_get_id(device));

        return eh->generator_process[type](
                dimpl, dstate, eh->channels[index], value);
    }
    else if (Event_is_effect(type))
    {
        Effect_table* effects = eh->effects;
        if (eh->channels[index]->inst_effects)
        {
            if (eh->channels[index]->effect >= KQT_INST_EFFECTS_MAX)
                return false;

            // Find our instrument
            Instrument* ins = Module_get_ins_from_input(
                    eh->master_params->parent.module,
                    eh->channels[index]->ins_input);
            if (ins == NULL)
                return false;

            effects = Instrument_get_effects(ins);
        }
        if (effects == NULL)
            return false;

        Effect* eff = Effect_table_get(effects, eh->channels[index]->effect);
        if (eff == NULL)
            return false;
        Effect_state* eff_state = (Effect_state*)Device_states_get_state(
                eh->device_states,
                Device_get_id((Device*)eff));

        return eh->effect_process[type](
                eff,
                eff_state,
                eh->device_states,
                value);
    }
    else if (Event_is_dsp(type))
    {
        Effect_table* effects = eh->effects;
        if (eh->channels[index]->inst_effects)
        {
            if (eh->channels[index]->effect >= KQT_INST_EFFECTS_MAX)
                return false;

            // Find our instrument
            Instrument* ins = Module_get_ins_from_input(
                    eh->master_params->parent.module,
                    eh->channels[index]->ins_input);
            if (ins == NULL)
                return false;

            effects = Instrument_get_effects(ins);
        }
        if (effects == NULL)
            return false;

        Effect* eff = Effect_table_get(effects, eh->channels[index]->effect);
        if (eff == NULL)
            return false;

        DSP_table* dsps = Effect_get_dsps(eff);
        Device* device = (Device*)DSP_table_get_dsp(
                dsps, eh->channels[index]->dsp);
        if (device == NULL)
            return false;

        const Device_impl* dimpl = device->dimpl;
        if (dimpl == NULL)
            return false;

        Device_state* dstate = Device_states_get_state(
                eh->device_states,
                Device_get_id(device));

        return eh->dsp_process[type](dimpl, dstate, eh->channels[index], value);
    }
    else if (Event_is_control(type))
    {
        return eh->control_process[type](
                (General_state*)eh->master_params,
                value);
    }
    else if (Event_is_general(type))
    {
        General_state* gstate = (General_state*)eh->channels[index];
        return eh->general_process[type](gstate, value);
    }

    return false;
}


bool Event_handler_trigger(
        Event_handler* eh,
        int ch_num,
        const char* name,
        Value* arg)
{
    assert(eh != NULL);
    assert(ch_num >= 0);
    assert(ch_num < KQT_CHANNELS_MAX);
    assert(name != NULL);
    assert(arg != NULL);

    Event_type type = Event_names_get(eh->event_names, name);
    assert(type != Event_NONE);
    assert(!Event_is_query(type));
    assert(!Event_is_auto(type));

    assert(eh->channels[ch_num]->freq != NULL);
    assert(*eh->channels[ch_num]->freq > 0);
    assert(eh->channels[ch_num]->tempo != NULL);
    assert(*eh->channels[ch_num]->tempo > 0);

    return Event_handler_handle(eh, ch_num, type, arg);
}


#if 0
bool Event_handler_process_type(
        Event_handler* eh,
        int index,
        char** desc,
        char* event_name,
        Event_type* event_type,
        Read_state* state)
{
    assert(eh != NULL);
    assert(index >= 0);
    assert(index < KQT_COLUMNS_MAX);
    assert(desc != NULL);
    assert(*desc != NULL);
    assert(event_name != NULL);
    assert(event_type != NULL);
    assert(state != NULL);
    assert(!state->error);

    *desc = read_const_char(*desc, '[', state);
    *desc = read_string(*desc, event_name, EVENT_NAME_MAX + 2, state);
    *desc = read_const_char(*desc, ',', state);
    if (state->error)
        return false;

    *event_type = Event_names_get(eh->event_names, event_name);
    if (*event_type == Event_NONE)
    {
        Read_state_set_error(
                state,
                "Unsupported event type: %s",
                event_name);
        return false;
    }

    assert(Event_is_valid(*event_type));
    if (!General_state_events_enabled((General_state*)eh->channels[index]) &&
            *event_type != Event_general_if &&
            *event_type != Event_general_else &&
            *event_type != Event_general_end_if)
        return false;

    return true;
}
#endif


#if 0
bool Event_handler_add_channel_gen_state_key(
        Event_handler* eh,
        const char* key)
{
    assert(eh != NULL);
    assert(key != NULL);

    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        if (!Channel_gen_state_set_key(eh->channels[i]->cgstate, key))
            return false;
    }

    return true;
}
#endif


void del_Event_handler(Event_handler* eh)
{
    if (eh == NULL)
        return;

    del_Event_names(eh->event_names);
    memory_free(eh);

    return;
}


