

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
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
#include <math.h>

#include <Voice_pool.h>
#include <Channel.h>
#include <Environment.h>
#include <memory.h>
#include <Playdata.h>
#include <Random.h>
#include <Tstamp.h>
#include <serialise.h>
#include <Slider.h>
#include <string_common.h>
#include <xassert.h>


Playdata* new_Playdata(Ins_table* insts,
                       Environment* env,
                       Random* random)
{
    assert(insts != NULL);
    assert(env != NULL);
    (void)insts; // FIXME: remove?
    assert(random != NULL);
    Playdata* play = memory_alloc_item(Playdata);
    if (play == NULL)
    {
        return NULL;
    }
    play->random = random;
    play->infinite = false;
    play->event_filter = NULL;
    play->bind = NULL;
    play->play_id = 1;
    play->silent = false;
    play->citer = new_Column_iter(NULL);
    play->voice_pool = new_Voice_pool(256);
    if (!General_state_init(&play->parent, true, env) ||
            play->citer == NULL || play->voice_pool == NULL)
    {
        del_Playdata(play);
        return NULL;
    }
    play->mode = PLAY_SONG;
    play->freq = 48000;
    play->old_freq = play->freq;
    play->subsongs = NULL;
    play->track_list = NULL;
    play->order_lists = NULL;
//    play->events = NULL;

    play->scales = NULL;
    play->active_scale = NULL;
    play->scale = 0;
    play->scale_fixed_point = 0;

    play->jump_set_counter = 0;
    play->jump_set_subsong = -1;
    play->jump_set_section = -1;
    Tstamp_init(&play->jump_set_row);
    play->jump = false;
    play->jump_subsong = -1;
    play->jump_section = -1;
    Tstamp_init(&play->jump_row);

    play->goto_set_subsong = -1;
    play->goto_set_section = -1;
    Tstamp_init(&play->goto_set_row);
    play->goto_trigger = false;
    play->goto_subsong = -1;
    play->goto_section = -1;
    Tstamp_init(&play->goto_row);

    play->volume = 1;
    Slider_init(&play->volume_slider, SLIDE_MODE_EXP);
    Slider_set_mix_rate(&play->volume_slider, 48000);

    play->tempo = 0;
    play->old_tempo = 0;
    play->tempo_slide = 0;
    Tstamp_init(&play->tempo_slide_length);
    play->tempo_slide_target = 0;
    Tstamp_init(&play->tempo_slide_left);
    play->tempo_slide_int_target = 0;
    Tstamp_init(&play->tempo_slide_int_left);
    play->tempo_slide_update = 0;

    Tstamp_init(&play->delay_left);
    play->event_index = 0;
    play->delay_event_index = -1;

    //play->orig_subsong = 0;
    //play->subsong = 0;
    play->system = 0;
    play->piref.pat = -1;
    play->piref.inst = 0;
    Tstamp_init(&play->play_time);
    play->play_frames = 0;
    Tstamp_init(&play->pos);
    Playdata_reset_stats(play);

    return play;
}


Playdata* new_Playdata_silent(Environment* env, uint32_t freq)
{
    assert(env != NULL);
    assert(freq > 0);
    Playdata* play = memory_alloc_item(Playdata);
    if (play == NULL)
    {
        return NULL;
    }
    play->random = NULL;
    play->infinite = false;
    play->event_filter = NULL;
    play->bind = NULL;
    play->play_id = 0x8000000000000001ULL; // prevent conflict with normal state
    play->silent = true;
    play->citer = new_Column_iter(NULL);
    play->voice_pool = NULL;
    if (!General_state_init(&play->parent, true, env) || play->citer == NULL)
    {
        del_Playdata(play);
        return NULL;
    }
//    play->ins_events = NULL;
    play->mode = PLAY_SONG;
    play->freq = freq;
    play->old_freq = play->freq;
    play->subsongs = NULL;
    play->track_list = NULL;
    play->order_lists = NULL;
//    play->events = NULL;

    play->scales = NULL;
    play->active_scale = NULL;
    play->scale = 0;
    play->scale_fixed_point = 0;

    play->jump_set_counter = 0;
    play->jump_set_subsong = -1;
    play->jump_set_section = -1;
    Tstamp_init(&play->jump_set_row);
    play->jump = false;
    play->jump_subsong = -1;
    play->jump_section = -1;
    Tstamp_init(&play->jump_row);

    play->goto_set_subsong = -1;
    play->goto_set_section = -1;
    Tstamp_init(&play->goto_set_row);
    play->goto_trigger = false;
    play->goto_subsong = -1;
    play->goto_section = -1;
    Tstamp_init(&play->goto_row);

    play->volume = 1;
    Slider_init(&play->volume_slider, SLIDE_MODE_EXP);
    Slider_set_mix_rate(&play->volume_slider, freq);

    play->tempo = 0;
    play->old_tempo = 0;
    play->tempo_slide = 0;
    Tstamp_init(&play->tempo_slide_length);
    play->tempo_slide_target = 0;
    Tstamp_init(&play->tempo_slide_left);
    play->tempo_slide_int_target = 0;
    Tstamp_init(&play->tempo_slide_int_left);
    play->tempo_slide_update = 0;

    Tstamp_init(&play->delay_left);
    play->event_index = 0;
    play->delay_event_index = -1;

    //play->orig_subsong = 0;
    //play->subsong = 0;
    play->system = 0;
    play->piref.pat = -1;
    play->piref.inst = 0;
    Tstamp_init(&play->play_time);
    play->play_frames = 0;
    Tstamp_init(&play->pos);
    Playdata_reset_stats(play);

    return play;
}


void Playdata_set_mix_freq(Playdata* play, uint32_t freq)
{
    assert(play != NULL);
    assert(freq > 0);
    play->freq = freq;
    Slider_set_mix_rate(&play->volume_slider, freq);
    return;
}


void Playdata_set_track(Playdata* play, int track, bool reset)
{
    assert(play != NULL);
    assert(track >= 0);
    assert(track < KQT_TRACKS_MAX);
    play->orig_track = play->track = track;
    play->system = 0;
    if (!play->silent)
    {
        assert(play->voice_pool != NULL);
        if (reset)
        {
            Voice_pool_reset(play->voice_pool);
        }
    }
    play->tempo = NAN;
    return;
}


void Playdata_reset(Playdata* play)
{
    assert(play != NULL);
    General_state_reset(&play->parent);
    ++play->play_id;
    if (!play->silent)
    {
        Voice_pool_reset(play->voice_pool);
        for (int i = 0; i < KQT_SCALES_MAX; ++i)
        {
            if (play->scales[i] != NULL)
            {
                Scale_reset(play->scales[i]);
                //Scale_retune(play->scales[i], -1, 0);
            }
        }
    }
    if (play->random != NULL)
    {
        Random_reset(play->random);
    }
    play->infinite = false;
    play->scale = 0;
    play->scale_fixed_point = 0;

    play->jump_set_counter = 0;
    play->jump_set_subsong = -1;
    play->jump_set_section = -1;
    Tstamp_init(&play->jump_set_row);
    play->jump = false;
    play->jump_subsong = -1;
    play->jump_section = -1;
    Tstamp_init(&play->jump_row);

    play->goto_set_subsong = -1;
    play->goto_set_section = -1;
    Tstamp_init(&play->goto_set_row);
    play->goto_trigger = false;
    play->goto_subsong = -1;
    play->goto_section = -1;
    Tstamp_init(&play->goto_row);

    play->volume = 1;
    Slider_init(&play->volume_slider, SLIDE_MODE_EXP);
//    Slider_set_mix_rate(&play->volume_slider, play->freq);
//    Slider_set_tempo(&play->volume_slider, play->tempo);
    play->tempo_slide = 0;
    Tstamp_init(&play->tempo_slide_length);
    Tstamp_init(&play->delay_left);
    play->event_index = 0;
    play->delay_event_index = -1;
    play->play_frames = 0;
    play->system = 0;
    play->piref.pat = -1;
    play->piref.inst = 0;
    Tstamp_init(&play->play_time);
    Tstamp_init(&play->pos);
    return;
}


void Playdata_reset_stats(Playdata* play)
{
    assert(play != NULL);
    play->active_voices = 0;
    for (int i = 0; i < KQT_BUFFERS_MAX; ++i)
    {
        play->min_amps[i] = INFINITY;
        play->max_amps[i] = -INFINITY;
        play->clipped[i] = 0;
    }
    return;
}


bool Playdata_get_state_value(Playdata* play,
                              char* key,
                              char* dest,
                              int size)
{
    assert(play != NULL);
    assert(key != NULL);
    assert(dest != NULL);
    assert(size > 0);
    if (string_eq(key, "active_voices"))
    {
        serialise_int(dest, size, play->active_voices);
        play->active_voices = 0;
        return true;
    }
    return false;
}


void Playdata_set_event_filter(Playdata* play,
                               Event_names* filter)
{
    assert(play != NULL);
    assert(filter != NULL);
    play->event_filter = filter;
    return;
}


void del_Playdata(Playdata* play)
{
    if (play == NULL)
    {
        return;
    }
    del_Voice_pool(play->voice_pool);
    del_Column_iter(play->citer);
    General_state_uninit(&play->parent);
    memory_free(play);
    return;
}


