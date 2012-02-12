

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2012
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_PLAYDATA_H
#define K_PLAYDATA_H


#include <stdint.h>

#include <Active_names.h>
#include <Reltime.h>
#include <Subsong_table.h>
#include <Call_map.h>
//#include <Channel.h>
#include <Column.h>
#include <Environment.h>
#include <Event_names.h>
#include <General_state.h>
#include <Random.h>
#include <Set_map.h>
#include <Slider.h>
#include <Voice_pool.h>
#include <Ins_table.h>
#include <kunquat/limits.h>
#include <frame.h>


/**
 * Playback states.
 */
typedef enum Play_mode
{
    STOP = 0,       ///< Don't play.
//    PLAY_EVENT,     ///< Play a single event.
    PLAY_PATTERN,   ///< Play one pattern.
    PLAY_SUBSONG,   ///< Play one subsong.
    PLAY_SONG,      ///< Play all subsongs.
    PLAY_LAST       ///< Sentinel value -- never used as a mode.
} Play_mode;


typedef struct Playdata
{
    General_state parent;
    uint64_t play_id;                 ///< A unique identifier for successive playbacks.
    bool silent;                      ///< \c true if this Playdata is used for statistics only.
    Play_mode mode;                   ///< Current playback mode.
    uint32_t freq;                    ///< Mixing frequency.
    uint32_t old_freq;                ///< Old mixing frequency (used to detect freq change).
    Subsong_table* subsongs;          ///< The Subsongs.
    Reltime play_time;                ///< The number of beats played since the start of playback.
    uint64_t play_frames;             ///< The number of frames mixed since the start of playback.
    Random* random;                   ///< Random source.

    bool infinite;                    ///< Infinite mode enabled.

    Event_names* event_filter;        ///< Event filter.
    Call_map* call_map;
    Set_map* set_map;

    Scale** scales;                   ///< The Scales.
    Scale** active_scale;             ///< A reference to the currently active Scale. FIXME: obsolete
    int scale;                        ///< Currently active Scale index.
    int scale_fixed_point;            ///< Current fixed point in interval shifts.

    int16_t jump_set_counter;         ///< Jump counter passed to a jump event.
    int16_t jump_set_subsong;         ///< Subsong number setting passed to a jump event.
    int16_t jump_set_section;         ///< Section number setting passed to a jump event.
    Reltime jump_set_row;             ///< Pattern position passed to a jump event.
    bool jump;                        ///< Jump trigger.
    int16_t jump_subsong;             ///< Jump target subsong (-1 = no change).
    int16_t jump_section;             ///< Jump target section (-1 = no change).
    Reltime jump_row;                 ///< Jump target pattern position.

    int16_t goto_set_subsong;         ///< Subsong number setting passed to a goto event.
    int16_t goto_set_section;         ///< Section number setting passed to a goto event.
    Reltime goto_set_row;             ///< Pattern position passed to a goto event.
    bool goto_trigger;                ///< Goto trigger.
    int16_t goto_subsong;             ///< Goto target subsong (-1 = no change).
    int16_t goto_section;             ///< Goto target section (-1 = no change).
    Reltime goto_row;                 ///< Goto target pattern position.

    double volume;                    ///< Current global volume.
    Slider volume_slider;

    double tempo;                     ///< Current tempo.
    int tempo_slide;                  ///< Tempo slide state (0 = no slide, -1 = down, 1 = up).
    Reltime tempo_slide_length;
    double tempo_slide_target;        ///< Final target tempo of the tempo slide.
    Reltime tempo_slide_left;         ///< The total time left to finish the tempo slide.
    double tempo_slide_int_target;    ///< Intermediate target tempo of the tempo slide.
    Reltime tempo_slide_int_left;     ///< Time left until shifting tempo.
    double tempo_slide_update;        ///< The update amount of the tempo slide.
    double old_tempo;                 ///< Old tempo (used to detect tempo change).

    Reltime delay_left;               ///< The amount of pattern delay left.
    int event_index;                  ///< Current event index.
    int delay_event_index;            ///< Position of the delay event.

    uint16_t orig_subsong;            ///< Subsong at the start of playback.
    uint16_t subsong;                 ///< Current subsong -- used when \a play == \c PLAY_SONG.
    uint16_t section;                 ///< Current section -- used when \a play == \c PLAY_SONG.
    int16_t pattern;                  ///< Current pattern.
    Reltime pos;                      ///< Current position inside a pattern.
    Voice_pool* voice_pool;           ///< The Voice pool used.
    Column_iter* citer;               ///< Column iterator.
    uint16_t active_voices;             ///< Number of Voices used simultaneously.
    double min_amps[KQT_BUFFERS_MAX];   ///< Minimum amplitude values encountered.
    double max_amps[KQT_BUFFERS_MAX];   ///< Maximum amplitude values encountered.
    uint64_t clipped[KQT_BUFFERS_MAX];  ///< Number of clipped frames encountered.
} Playdata;


/**
 * Creates a new Playdata object.
 *
 * The caller shall eventually destroy the created object using
 * del_Playdata().
 *
 * \param insts       The Instrument table -- must not be \c NULL.
 * \param env         The Environment -- must not be \c NULL.
 * \param random      The Random source -- must not be \c NULL.
 *
 * \return   The new Playdata object if successful, or \c NULL if memory
 *           allocation failed.
 */
Playdata* new_Playdata(Ins_table* insts,
                       Environment* env,
                       Random* random);


/**
 * Creates a new silent Playdata object (used for retrieving statistics).
 *
 * The caller shall eventually destroy the created object using
 * del_Playdata().
 *
 * \param env     The Environment -- must not be \c NULL.
 * \param freq    The mixing frequency -- must be > \c 0.
 *
 * \return   The new Playdata object if successful, or \c NULL if memory
 *           allocation failed.
 */
Playdata* new_Playdata_silent(Environment* env, uint32_t freq);


/**
 * Sets a new mixing frequency.
 *
 * \param play   The Playdata object -- must not be \c NULL.
 * \param freq   The mixing frequency -- must be > \c 0.
 */
void Playdata_set_mix_freq(Playdata* play, uint32_t freq);


/**
 * Sets the subsong in the Playdata.
 *
 * This should only be called when starting playback of a subsong, i.e. it
 * should not be used by events that jump between subsongs.
 *
 * \param play      The Playdata -- must not be \c NULL.
 * \param subsong   The subsong number -- must be >= \c 0 and < \c KQT_SUBSONGS_MAX.
 */
void Playdata_set_subsong(Playdata* play, int subsong);


/**
 * Resets playback state.
 *
 * \param play   The Playdata object -- must not be \c NULL.
 */
void Playdata_reset(Playdata* play);


/**
 * Resets playback statistics.
 *
 * \param play   The Playdata object -- must not be \c NULL.
 */
void Playdata_reset_stats(Playdata* play);


/**
 * Gets state information from the Playdata.
 *
 * \param play   The Playdata object -- must not be \c NULL.
 * \param key    The key of the state variable -- must not be \c NULL.
 * \param dest   The destination buffer -- must not be \c NULL.
 * \param size   The size of the destination buffer -- must be
 *               positive.
 */
bool Playdata_get_state_value(Playdata* play,
                              char* key,
                              char* dest,
                              int size);


/**
 * Sets an event filter for the Playdata.
 *
 * \param play     The Playdata object -- must not be \c NULL.
 * \param filter   The event filter -- must not be \c NULL.
 */
void Playdata_set_event_filter(Playdata* play,
                               Event_names* filter);


/**
 * Deletes a Playdata object.
 *
 * \param play   The Playdata object, or \c NULL.
 */
void del_Playdata(Playdata* play);


#endif // K_PLAYDATA_H


