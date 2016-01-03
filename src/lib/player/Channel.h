

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


#ifndef K_CHANNEL_H
#define K_CHANNEL_H


#include <decl.h>
#include <init/Au_table.h>
#include <init/sheet/Channel_defaults.h>
#include <kunquat/limits.h>
#include <mathnum/Random.h>
#include <mathnum/Tstamp.h>
#include <player/Channel_cv_state.h>
#include <player/Env_state.h>
#include <player/Event_cache.h>
#include <player/Force_controls.h>
#include <player/General_state.h>
#include <player/LFO.h>
#include <player/Pitch_controls.h>
#include <player/Voice_pool.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


/**
 * This structure is used for transferring channel-specific settings to
 * Voices.
 */
struct Channel
{
    General_state parent;
    int num;                       ///< Channel number.
    Random* rand;                  ///< Random source for this channel.
    Event_cache* event_cache;
    Channel_cv_state* cvstate;

    Voice_pool* pool;              ///< All Voices.
    Voice* fg[KQT_PROCESSORS_MAX]; ///< Foreground Voices.
    uint64_t fg_id[KQT_PROCESSORS_MAX]; ///< Voice reservation IDs.
    int fg_count;

    int32_t au_input;              ///< Currently active Audio unit input.
    Au_table* au_table;
    int32_t* freq;
    double* tempo;

    double volume;                 ///< Channel volume (linear factor).

    Tstamp force_slide_length;
    double tremolo_speed;
    Tstamp tremolo_speed_slide;
    double tremolo_depth;
    Tstamp tremolo_depth_slide;
    bool carry_force;
    Force_controls force_controls;

    Tstamp pitch_slide_length;
    double vibrato_speed;
    Tstamp vibrato_speed_slide;
    double vibrato_depth;
    Tstamp vibrato_depth_slide;
    bool carry_pitch;
    double orig_pitch;
    Pitch_controls pitch_controls;

    double arpeggio_ref;
    double arpeggio_speed;
    int arpeggio_edit_pos;
    double arpeggio_tones[KQT_ARPEGGIO_NOTES_MAX];
};


/**
 * Create a new Channel.
 *
 * \param module     The Module -- must not be \c NULL.
 * \param num        The Channel number -- must be >= \c 0 and
 *                   < \c KQT_CHANNELS_MAX.
 * \param au_table   The audio unit table -- must not be \c NULL.
 * \param estate     The Environment state -- must not be \c NULL.
 * \param voices     The Voice pool -- must not be \c NULL.
 * \param tempo      A reference to the current tempo -- must not be \c NULL.
 * \param rate       A reference to the current audio rate -- must not be \c NULL.
 *
 * \return   The new Channel state if successful, or \c NULL if memory
 *           allocation failed.
 */
Channel* new_Channel(
        const Module* module,
        int num,
        Au_table* au_table,
        Env_state* estate,
        Voice_pool* voices,
        double* tempo,
        int32_t* audio_rate);


/**
 * Set the Channel audio rate.
 *
 * \param ch           The Channel -- must not be \c NULL.
 * \param audio_rate   The audio rate -- must be positive.
 */
void Channel_set_audio_rate(Channel* ch, int32_t audio_rate);


/**
 * Set the Channel tempo.
 *
 * \param ch      The Channel -- must not be \c NULL.
 * \param tempo   The tempo -- must be positive.
 */
void Channel_set_tempo(Channel* ch, double tempo);


/**
 * Set the Channel random seed.
 *
 * \param ch     The Channel -- must not be \c NULL.
 * \param seed   The random seed.
 */
void Channel_set_random_seed(Channel* ch, uint64_t seed);


/**
 * Set the Event cache of the Channel.
 *
 * \param ch      The Channel -- must not be \c NULL.
 * \param cache   The Event cache -- must not be \c NULL.
 */
void Channel_set_event_cache(Channel* ch, Event_cache* cache);


/**
 * Reset the Channel.
 *
 * \param ch   The Channel -- must not be \c NULL.
 */
void Channel_reset(Channel* ch);


/**
 * Apply Channel defaults to the Channel.
 *
 * \param ch            The Channel -- must not be \c NULL.
 * \param ch_defaults   The Channel defaults -- must not be \c NULL.
 */
void Channel_apply_defaults(Channel* ch, const Channel_defaults* ch_defaults);


/**
 * Get the Channel Random source.
 *
 * \param ch   The Channel -- must not be \c NULL.
 *
 * \return   The Random source. This is never \c NULL.
 */
Random* Channel_get_random_source(Channel* ch);


/**
 * Get current foreground Voice of the Channel.
 *
 * \param ch           The Channel -- must not be \c NULL.
 * \param proc_index   The Processor index -- must be >= \c 0 and
 *                     < \c KQT_PROCESSORS_MAX.
 *
 * \return   The foreground Voice at \a proc_index if one exists, otherwise \c NULL.
 */
Voice* Channel_get_fg_voice(Channel* ch, int proc_index);


/**
 * Return an actual force of a current foreground Voice.
 *
 * \param ch           The Channel -- must not be \c NULL.
 * \param proc_index   The Processor index -- must be >= \c 0 and
 *                     < \c KQT_PROCESSORS_MAX.
 *
 * \return   The actual force if the active foreground Voice at \a proc_index
 *           exists, otherwise NAN.
 */
double Channel_get_fg_force(Channel* ch, int proc_index);


/**
 * Get control variable state of the Channel.
 *
 * \param ch   The Channel -- must not be \c NULL.
 *
 * \return   The control variable state of the Channel. This is never \c NULL.
 */
const Channel_cv_state* Channel_get_cv_state(const Channel* ch);


/**
 * Get mutable control variable state of the Channel.
 *
 * \param ch   The Channel -- must not be \c NULL.
 *
 * \return   The mutable control variable state of the Channel. This is never \c NULL.
 */
Channel_cv_state* Channel_get_cv_state_mut(Channel* ch);


/**
 * Deinitialise the Channel.
 *
 * \param ch   The Channel, or \c NULL.
 */
void Channel_deinit(Channel* ch);


/**
 * Destroy an existing Channel.
 *
 * \param ch   The Channel, or \c NULL.
 */
void del_Channel(Channel* ch);


#endif // K_CHANNEL_H


