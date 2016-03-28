

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


#include <init/Module.h>

#include <debug/assert.h>
#include <mathnum/common.h>
#include <mathnum/Real.h>
#include <memory.h>
#include <init/comp_defaults.h>
#include <init/sheet/Channel_defaults_list.h>
#include <string/common.h>

#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


Module* new_Module(void)
{
    Module* module = memory_alloc_item(Module);
    if (module == NULL)
        return NULL;

    if (!Device_init(&module->parent, false))
    {
        memory_free(module);
        return NULL;
    }

    Device_set_existent(&module->parent, true);

    // Clear fields
    module->songs = NULL;
    module->pats = NULL;
    module->au_map = NULL;
    module->au_controls = NULL;
    module->au_table = NULL;
    module->connections = NULL;
    module->env = NULL;
    module->bind = NULL;
    module->album_is_existent = false;
    module->track_list = NULL;
    module->ch_defs = NULL;
    for (int i = 0; i < KQT_SONGS_MAX; ++i)
        module->order_lists[i] = NULL;
    for (int i = 0; i < KQT_SCALES_MAX; ++i)
        module->scales[i] = NULL;

    // Create fields
    module->songs = new_Song_table();
    module->pats = new_Pat_table(KQT_PATTERNS_MAX);
    module->au_controls = new_Bit_array(KQT_CONTROLS_MAX);
    module->au_table = new_Au_table(KQT_AUDIO_UNITS_MAX);
    if (module->songs == NULL           ||
            module->pats == NULL        ||
            module->au_controls == NULL ||
            module->au_table == NULL)
    {
        del_Module(module);
        return NULL;
    }

    module->scales[0] = new_Scale(SCALE_DEFAULT_REF_PITCH,
            SCALE_DEFAULT_OCTAVE_RATIO);
    if (module->scales[0] == NULL)
    {
        del_Module(module);
        return NULL;
    }

    module->env = new_Environment();
    if (module->env == NULL)
    {
        del_Module(module);
        return NULL;
    }

    Streader* conn_sr = Streader_init(STREADER_AUTO, NULL, 0);
    module->connections =
        new_Connections_from_string(conn_sr, false, module->au_table, &module->parent);
    if (module->connections == NULL)
    {
        del_Module(module);
        return NULL;
    }

    if (Scale_ins_note(module->scales[0], 0,
                       Real_init_as_frac(REAL_AUTO, 1, 1)) < 0)
    {
        del_Module(module);
        return NULL;
    }

    for (int i = 1; i < 12; ++i)
    {
        if (Scale_ins_note_cents(module->scales[0], i, i * 100) < 0)
        {
            del_Module(module);
            return NULL;
        }
    }

    module->mix_vol_dB = COMP_DEFAULT_MIX_VOL;
    module->mix_vol = exp2(module->mix_vol_dB / 6);
    //module->init_subsong = SONG_DEFAULT_INIT_SUBSONG;
    module->random_seed = 0;

    return module;
}


bool Module_read_mixing_volume(Module* module, Streader* sr)
{
    assert(module != NULL);
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    double mix_vol = COMP_DEFAULT_MIX_VOL;

    if (Streader_has_data(sr))
    {
        if (!Streader_read_float(sr, &mix_vol))
            return false;

        if (!isfinite(mix_vol) && mix_vol != -INFINITY)
        {
            Streader_set_error(sr, "Invalid mixing volume: %s", mix_vol);
            return false;
        }
    }

    module->mix_vol_dB = mix_vol;
    module->mix_vol = exp2(module->mix_vol_dB / 6);

    return true;
}


bool Module_parse_random_seed(Module* module, Streader* sr)
{
    assert(module != NULL);
    assert(sr != NULL);

    if (Streader_is_error_set(sr))
        return false;

    int64_t seed = 0;

    if (Streader_has_data(sr))
    {
        if (!Streader_read_int(sr, &seed))
            return false;

        if (seed < 0)
        {
            Streader_set_error(sr, "Random seed must be non-negative");
            return false;
        }
    }

    module->random_seed = seed;

    return true;
}


const Track_list* Module_get_track_list(const Module* module)
{
    assert(module != NULL);

    if (!module->album_is_existent)
        return NULL;

    assert(module->track_list != NULL);
    return module->track_list;
}


const Order_list* Module_get_order_list(const Module* module, int16_t song)
{
    assert(module != NULL);
    assert(song >= 0);
    assert(song < KQT_SONGS_MAX);

    if (!Song_table_get_existent(module->songs, song))
        return NULL;

    assert(module->order_lists[song] != NULL);
    return module->order_lists[song];
}


void Module_set_ch_defaults_list(Module* module, Channel_defaults_list* ch_defs)
{
    assert(module != NULL);

    del_Channel_defaults_list(module->ch_defs);
    module->ch_defs = ch_defs;

    return;
}


const Channel_defaults_list* Module_get_ch_defaults_list(const Module* module)
{
    assert(module != NULL);
    return module->ch_defs;
}


const Pattern* Module_get_pattern(
        const Module* module,
        const Pat_inst_ref* piref)
{
    assert(module != NULL);
    assert(piref != NULL);
    assert(piref->pat >= 0);
    assert(piref->pat < KQT_PATTERNS_MAX);

    if (!Pat_table_get_existent(module->pats, piref->pat))
        return NULL;

    const Pattern* pat = Pat_table_get(module->pats, piref->pat);
    if (pat != NULL && !Pattern_get_inst_existent(pat, piref->inst))
        return NULL;

    return pat;
}


bool Module_find_pattern_location(
        const Module* module,
        const Pat_inst_ref* piref,
        int16_t* track,
        int16_t* system)
{
    assert(module != NULL);
    assert(piref != NULL);
    assert(piref->pat >= 0);
    assert(piref->pat < KQT_PATTERNS_MAX);
    assert(piref->inst >= 0);
    assert(piref->inst < KQT_PAT_INSTANCES_MAX);
    assert(track != NULL);
    assert(system != NULL);

    // Linear search all track lists
    for (int ti = 0; ti < KQT_SONGS_MAX; ++ti)
    {
        if (!Song_table_get_existent(module->songs, ti))
            continue;

        const Order_list* ol = module->order_lists[ti];
        assert(ol != NULL);

        for (size_t i = 0; i < Order_list_get_len(ol); ++i)
        {
            const Pat_inst_ref* cur_piref = Order_list_get_pat_inst_ref(ol, i);
            assert(cur_piref != NULL);

            if (cur_piref->pat == piref->pat && cur_piref->inst == piref->inst)
            {
                *track = ti;
                *system = i;
                return true;
            }
        }
    }

    return false;
}


const Connections* Module_get_connections(const Module* module)
{
    assert(module != NULL);
    return module->connections;
}


void Module_set_mix_vol(Module* module, double mix_vol)
{
    assert(module != NULL);
    assert(isfinite(mix_vol) || mix_vol == -INFINITY);

    module->mix_vol_dB = mix_vol;
    module->mix_vol = exp2(mix_vol / 6);

    return;
}


double Module_get_mix_vol(Module* module)
{
    assert(module != NULL);
    return module->mix_vol_dB;
}


Song_table* Module_get_songs(const Module* module)
{
    assert(module != NULL);
    return module->songs;
}


Pat_table* Module_get_pats(Module* module)
{
    assert(module != NULL);
    return module->pats;
}


int32_t Module_get_au_index_from_input(const Module* module, int32_t input)
{
    assert(module != NULL);
    assert(input >= 0);

    if (module->au_map == NULL)
        return -1;

    return Input_map_get_device_index(module->au_map, input);
}


Audio_unit* Module_get_au_from_input(const Module* module, int32_t input)
{
    assert(module != NULL);
    assert(input >= 0);

    const int32_t au_index = Module_get_au_index_from_input(module, input);
    if (au_index < 0)
        return NULL;

    assert(Bit_array_get(module->au_controls, input));
    assert(au_index < KQT_AUDIO_UNITS_MAX);
    return Au_table_get(module->au_table, au_index);
}


void Module_set_control(Module* module, int control, bool existent)
{
    assert(module != NULL);
    assert(control >= 0);
    assert(control < KQT_CONTROLS_MAX);

    Bit_array_set(module->au_controls, control, existent);

    return;
}


bool Module_get_control(const Module* module, int control)
{
    assert(module != NULL);
    assert(control >= 0);
    assert(control < KQT_CONTROLS_MAX);

    return Bit_array_get(module->au_controls, control);
}


bool Module_set_au_map(Module* module, Streader* sr)
{
    assert(module != NULL);
    assert(sr != NULL);

    Input_map* im = new_Input_map(sr, INT32_MAX, KQT_AUDIO_UNITS_MAX);
    if (im == NULL)
        return false;

    del_Input_map(module->au_map);
    module->au_map = im;

    return true;
}


Input_map* Module_get_au_map(const Module* module)
{
    assert(module != NULL);
    return module->au_map;
}


Au_table* Module_get_au_table(const Module* module)
{
    assert(module != NULL);
    return module->au_table;
}


void Module_set_bind(Module* module, Bind* bind)
{
    assert(module != NULL);
    assert(bind != NULL);

    del_Bind(module->bind);
    module->bind = bind;

    return;
}


Scale* Module_get_scale(Module* module, int index)
{
    assert(module != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALES_MAX);

    return module->scales[index];
}


void Module_set_scale(Module* module, int index, Scale* scale)
{
    assert(module != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALES_MAX);
    assert(scale != NULL);

    if (module->scales[index] != NULL &&
            module->scales[index] != scale)
        del_Scale(module->scales[index]);

    module->scales[index] = scale;

    return;
}


bool Module_create_scale(Module* module, int index)
{
    assert(module != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALES_MAX);

    if (module->scales[index] != NULL)
    {
        Scale_clear(module->scales[index]);
        return true;
    }

    module->scales[index] = new_Scale(SCALE_DEFAULT_REF_PITCH,
            SCALE_DEFAULT_OCTAVE_RATIO);
    if (module->scales[index] == NULL)
        return false;

    return true;
}


void Module_remove_scale(Module* module, int index)
{
    assert(module != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALES_MAX);

    if (module->scales[index] != NULL)
    {
        del_Scale(module->scales[index]);
        module->scales[index] = NULL;
    }

    return;
}


void del_Module(Module* module)
{
    if (module == NULL)
        return;

    del_Environment(module->env);
    del_Song_table(module->songs);
    del_Pat_table(module->pats);
    del_Connections(module->connections);
    del_Au_table(module->au_table);
    del_Bit_array(module->au_controls);
    del_Input_map(module->au_map);
    del_Track_list(module->track_list);
    del_Channel_defaults_list(module->ch_defs);

    for (int i = 0; i < KQT_SONGS_MAX; ++i)
        del_Order_list(module->order_lists[i]);

    for (int i = 0; i < KQT_SCALES_MAX; ++i)
        del_Scale(module->scales[i]);

    del_Bind(module->bind);

    Device_deinit(&module->parent);
    memory_free(module);

    return;
}


