

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


#ifndef K_VOICE_STATE_H
#define K_VOICE_STATE_H


#include <decl.h>
#include <kunquat/limits.h>
#include <mathnum/Random.h>
#include <mathnum/Tstamp.h>
#include <player/Force_controls.h>
#include <player/LFO.h>
#include <player/Pitch_controls.h>
#include <player/Slider.h>
#include <player/Time_env_state.h>
#include <string/key_pattern.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


typedef size_t Voice_state_get_size_func(void);


typedef void Voice_state_init_func(Voice_state*, const Proc_state*);


typedef int32_t Voice_state_render_voice_func(
        Voice_state*,
        Proc_state*,
        const Au_state*,
        const Work_buffers*,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo);


typedef void Voice_state_set_cv_bool_func(
        Voice_state*, const Device_state*, const Key_indices, bool);
typedef void Voice_state_set_cv_int_func(
        Voice_state*, const Device_state*, const Key_indices, int64_t);
typedef void Voice_state_set_cv_float_func(
        Voice_state*, const Device_state*, const Key_indices, double);
typedef void Voice_state_set_cv_tstamp_func(
        Voice_state*, const Device_state*, const Key_indices, const Tstamp*);


struct Voice_state
{
    bool active;                   ///< Whether there is anything left to process.
    bool has_finished;
    Random* rand_p;                ///< Parameter random source.
    Random* rand_s;                ///< Signal random source.

    Voice_state_render_voice_func* render_voice;

    bool has_release_data;
    int32_t release_stop;

    double ramp_attack;            ///< The current state of volume ramp during attack.

    int hit_index;                 ///< The hit index (negative for normal notes).

    uint64_t pos;                  ///< The current playback position.
    double pos_rem;                ///< The current playback position remainder.
    uint64_t rel_pos;              ///< The current relative playback position.
    double rel_pos_rem;            ///< The current relative playback position remainder.
    double dir;                    ///< The current playback direction.
    bool note_on;                  ///< Whether the note is still on.
    uint64_t noff_pos;             ///< Note Off position.
    double noff_pos_rem;           ///< Note Off position remainder.

    // TODO: temp hack -- replace with proper type identifier
    bool is_pitch_state;
    bool is_force_state;
    bool is_stream_state;
};


/**
 * Initialise a Voice state.
 *
 * \param state        The Voice state -- must not be \c NULL.
 * \param rand_p       The parameter Random source -- must not be \c NULL.
 * \param rand_s       The signal Random source -- must not be \c NULL.
 *
 * \return   The parameter \a state.
 */
Voice_state* Voice_state_init(Voice_state* state, Random* rand_p, Random* rand_s);


/**
 * Clear a Voice state.
 *
 * \param state   The Voice state -- must not be \c NULL.
 *
 * \return   The parameter \a state.
 */
Voice_state* Voice_state_clear(Voice_state* state);


/**
 * Render voice signal with the Voice state.
 *
 * \param vstate       The Voice state -- must not be \c NULL.
 * \param proc_state   The Processor state -- must not be \c NULL.
 * \param au_state     The Audio unit state -- must not be \c NULL.
 * \param wbs          The Work buffers -- must not be \c NULL.
 * \param buf_start    The start index of rendering -- must be >= \c 0.
 * \param buf_stop     The stop index of rendering -- must be less than or equal
 *                     to the audio buffer size.
 * \param tempo        The current tempo -- must be finite and > \c 0.
 *
 * \return   The actual stop index of rendering. This is always within
 *           the interval [\a buf_start, \a buf_stop].
 */
int32_t Voice_state_render_voice(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        double tempo);


/**
 * Mix rendered Voice signals to combined signal buffers.
 *
 * \param vstate       The Voice state -- must not be \c NULL.
 * \param proc_state   The Processor state -- must not be \c NULL.
 * \param buf_start    The start index of mixing -- must be >= \c 0.
 * \param buf_stop     The stop index of mixing -- must be less than or equal
 *                     to the audio buffer size.
 */
void Voice_state_mix_signals(
        Voice_state* vstate,
        Proc_state* proc_state,
        int32_t buf_start,
        int32_t buf_stop);


/**
 * Mark Voice state as having release data.
 *
 * \param vstate         The Voice state -- must not be \c NULL.
 * \param release_stop   The buffer end index of rendered release data
 *                       -- must be >= \c 0.
 */
void Voice_state_mark_release_data(Voice_state* vstate, int32_t release_stop);


/**
 * Set Voice state as finished.
 *
 * The Voice state will be deactivated after retrieving the buffer contents
 * written during the current cycle.
 *
 * \param vstate       The Voice state -- must not be \c NULL.
 */
void Voice_state_set_finished(Voice_state* vstate);


/**
 * Set value of a control variable in the Voice state.
 *
 * \param vstate   The Voice state -- must not be \c NULL.
 * \param dstate   The Device state -- must not be \c NULL.
 * \param key      The key of the control variable -- must not be \c NULL.
 * \param value    The value to be set -- must not be \c NULL.
 */
void Voice_state_cv_generic_set(
        Voice_state* vstate,
        const Device_state* dstate,
        const char* key,
        const Value* value);


#endif // K_VOICE_STATE_H


