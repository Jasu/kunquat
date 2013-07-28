

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
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include <Voice_state.h>
#include <Generator.h>
#include <frame.h>
#include <kunquat/limits.h>
#include <math_common.h>


/**
 * Gets the output buffers.
 */
#define Generator_common_get_buffers(ds, state, mixed, bufs)  \
    if (true)                                                 \
    {                                                         \
        Audio_buffer* buffer = Device_state_get_audio_buffer( \
                (ds), DEVICE_PORT_TYPE_SEND, 0);              \
        if (buffer == NULL)                                   \
        {                                                     \
            (state)->active = false;                          \
            return (mixed);                                   \
        }                                                     \
        bufs[0] = Audio_buffer_get_buffer(buffer, 0);         \
        bufs[1] = Audio_buffer_get_buffer(buffer, 1);         \
    } else (void)0


/**
 * Checks whether there's anything left of the note to be mixed.
 * This should be called at the beginning of the mixing function of the
 * Generator.
 */
#define Generator_common_check_active(gen, state, mixed)                     \
    if (true)                                                                \
    {                                                                        \
        if (!(state)->active || (!(state)->note_on &&                        \
                                 ((state)->pos == 0) &&                      \
                                 ((state)->pos_rem == 0) &&                  \
                                 !(gen)->ins_params->env_force_rel_enabled)) \
        {                                                                    \
            (state)->active = false;                                         \
            return (mixed);                                                  \
        }                                                                    \
    } else (void)0


/**
 * Adjusts Voice state according to tempo and/or mixing frequency changes.
 *
 * This should be called before the mixing loop of the Generator.
 *
 * \param gen     The Generator -- must not be \c NULL.
 * \param state   The Voice state -- must not be \c NULL.
 * \param freq    The mixing frequency -- must be > \c 0.
 * \param tempo   The tempo -- must be > \c 0 and finite.
 */
void Generator_common_check_relative_lengths(Generator* gen,
                                             Voice_state* state,
                                             uint32_t freq,
                                             double tempo);


/**
 * Pitch handling.
 *
 * \param gen     The Generator -- must not be \c NULL.
 * \param state   The Voice state -- must not be \c NULL.
 */
void Generator_common_handle_pitch(Generator* gen,
                                   Voice_state* state);


/**
 * Force handling.
 *
 * \param gen           The Generator -- must not be \c NULL.
 * \param state         The Voice state -- must not be \c NULL.
 * \param frames        The frames to be modified -- must not be \c NULL.
 * \param frame_count   The number of frames to be modified -- must be > \c 0.
 * \param freq          The mixing frequency -- must be > \c 0.
 */
void Generator_common_handle_force(Generator* gen,
                                   Voice_state* state,
                                   double frames[],
                                   int frame_count,
                                   uint32_t freq);


/**
 * Filter handling.
 *
 * This should be called after force handling.
 *
 * \param gen           The Generator -- must not be \c NULL.
 * \param state         The Voice state -- must not be \c NULL.
 * \param frames        The frames to be modified -- must not be \c NULL.
 * \param frame_count   The number of frames to be modified -- must be > \c 0.
 * \param freq          The mixing frequency -- must be > \c 0.
 */
void Generator_common_handle_filter(Generator* gen,
                                    Voice_state* state,
                                    double frames[],
                                    int frame_count,
                                    uint32_t freq);


/**
 * Automatic volume ramping for note start and end.
 *
 * This should be called after force handling if needed (not all Generators
 * need this).
 *
 * \param gen           The Generator -- must not be \c NULL.
 * \param state         The Voice state -- must not be \c NULL.
 * \param frames        The frames to be modified -- must not be \c NULL.
 * \param frame_count   The number of frames to be modified -- must be > \c 0.
 * \param freq          The mixing frequency -- must be > \c 0.
 */
void Generator_common_ramp_attack(Generator* gen,
                                  Voice_state* state,
                                  double frames[],
                                  int frame_count,
                                  uint32_t freq);


/**
 * Panning handling.
 *
 * \param gen           The Generator -- must not be \c NULL.
 * \param state         The Voice state -- must not be \c NULL.
 * \param frames        The frames to be modified -- must not be \c NULL.
 * \param frame_count   The number of frames to be modified -- must be > \c 0.
 */
void Generator_common_handle_panning(Generator* gen,
                                     Voice_state* state,
                                     double frames[],
                                     int frame_count);


