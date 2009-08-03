

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include <Voice_state.h>
#include <Generator.h>
#include <kunquat/frame.h>
#include <math_common.h>


#define RAMP_ATTACK_TIME (500.0)
#define RAMP_RELEASE_TIME (200.0)


#define Generator_common_check_active(gen, state, mixed)                      \
    do                                                                        \
    {                                                                         \
        if (!(state)->active || (!(state)->note_on &&                         \
                                 ((state)->pos == 0) &&                       \
                                 ((state)->pos_rem == 0) &&                   \
                                 !(gen)->ins_params->volume_off_env_enabled)) \
        {                                                                     \
            (state)->active = false;                                          \
            return (mixed);                                                   \
        }                                                                     \
    } while (false)


#define Generator_common_check_relative_lengths(gen, state, freq, tempo)          \
    do                                                                            \
    {                                                                             \
        if ((state)->freq != (freq) || (state)->tempo != (tempo))                 \
        {                                                                         \
            if ((state)->pitch_slide != 0)                                        \
            {                                                                     \
                double slide_step = log2((state)->pitch_slide_update);            \
                slide_step = slide_step * (state)->freq / (freq);                 \
                slide_step = slide_step * (tempo) / (state)->tempo;               \
                (state)->pitch_slide_update = exp2(slide_step);                   \
                (state)->pitch_slide_frames =                                     \
                        (state)->pitch_slide_frames * (freq) / (state)->freq;     \
                (state)->pitch_slide_frames =                                     \
                        (state)->pitch_slide_frames * (state)->tempo / (tempo);   \
            }                                                                     \
            if ((state)->force_slide != 0)                                        \
            {                                                                     \
                double update_dB = log2((state)->force_slide_update) * 6;         \
                update_dB = update_dB * (state)->freq / (freq);                   \
                update_dB = update_dB * (tempo) / (state)->tempo;                 \
                (state)->force_slide_update = exp2(update_dB / 6);                \
                (state)->force_slide_frames =                                     \
                        (state)->force_slide_frames * (freq) / (state)->freq;     \
                (state)->force_slide_frames =                                     \
                        (state)->force_slide_frames * (state)->tempo / (tempo);   \
            }                                                                     \
            if ((state)->tremolo_length > 0 && (state)->tremolo_depth > 0)        \
            {                                                                     \
                (state)->tremolo_length =                                         \
                        (state)->tremolo_length * (freq) / (state)->freq;         \
                (state)->tremolo_length =                                         \
                        (state)->tremolo_length * (state)->tempo / (tempo);       \
                (state)->tremolo_phase =                                          \
                        (state)->tremolo_phase * (freq) / (state)->freq;          \
                (state)->tremolo_phase =                                          \
                        (state)->tremolo_phase * (state)->tempo / (tempo);        \
                (state)->tremolo_update =                                         \
                        (state)->tremolo_update * (state)->freq / (freq);         \
                (state)->tremolo_update =                                         \
                        (state)->tremolo_update * (tempo) / (state)->tempo;       \
            }                                                                     \
            (state)->freq = (freq);                                               \
            (state)->tempo = (tempo);                                             \
        }                                                                         \
    } while (false)


#define Generator_common_ramp_attack(gen, state, frames, frame_count, freq) \
    do                                                                      \
    {                                                                       \
        if ((state)->ramp_attack < 1)                                       \
        {                                                                   \
            for (int i = 0; i < (frame_count); ++i)                         \
            {                                                               \
                (frames)[i] *= (state)->ramp_attack;                        \
            }                                                               \
            (state)->ramp_attack += RAMP_ATTACK_TIME / (freq);              \
        }                                                                   \
    } while (false)


#define Generator_common_handle_note_off(gen, state, frames, frame_count, freq,      \
                                         mixed)                                      \
    do                                                                               \
    {                                                                                \
        if (!(state)->note_on)                                                       \
        {                                                                            \
            if ((gen)->ins_params->volume_off_env_enabled)                           \
            {                                                                        \
                double scale = Envelope_get_value((gen)->ins_params->volume_off_env, \
                                                  (state)->off_ve_pos);              \
                if (!isfinite(scale))                                                \
                {                                                                    \
                    (state)->active = false;                                         \
                    return (mixed);                                                  \
                }                                                                    \
                (state)->off_ve_pos += (1.0 - (state)->pedal) / (freq);              \
                for (int i = 0; i < (frame_count); ++i)                              \
                {                                                                    \
                    (frames)[i] *= scale;                                            \
                }                                                                    \
            }                                                                        \
            else                                                                     \
            {                                                                        \
                if ((state)->ramp_release < 1)                                       \
                {                                                                    \
                    for (int i = 0; i < (frame_count); ++i)                          \
                    {                                                                \
                        (frames)[i] *= 1 - (state)->ramp_release;                    \
                    }                                                                \
                }                                                                    \
                else                                                                 \
                {                                                                    \
                    (state)->active = false;                                         \
                    return (mixed);                                                  \
                }                                                                    \
                (state)->ramp_release += RAMP_RELEASE_TIME / (freq);                 \
            }                                                                        \
        }                                                                            \
    } while (false)


#define Generator_common_handle_pitch(gen, state)                      \
    do                                                                 \
    {                                                                  \
        if ((state)->pitch_slide != 0)                                 \
        {                                                              \
            (state)->pitch *= (state)->pitch_slide_update;             \
            (state)->pitch_slide_frames -= 1;                          \
            if ((state)->pitch_slide_frames <= 0)                      \
            {                                                          \
                (state)->pitch = (state)->pitch_slide_target;          \
                (state)->pitch_slide = 0;                              \
            }                                                          \
            else if ((state)->pitch_slide == 1)                        \
            {                                                          \
                if ((state)->pitch > (state)->pitch_slide_target)      \
                {                                                      \
                    (state)->pitch = (state)->pitch_slide_target;      \
                    (state)->pitch_slide = 0;                          \
                }                                                      \
            }                                                          \
            else                                                       \
            {                                                          \
                assert((state)->pitch_slide == -1);                    \
                if ((state)->pitch < (state)->pitch_slide_target)      \
                {                                                      \
                    (state)->pitch = (state)->pitch_slide_target;      \
                    (state)->pitch_slide = 0;                          \
                }                                                      \
            }                                                          \
        }                                                              \
        (state)->actual_pitch = (state)->pitch;                        \
    } while (false)


#define Generator_common_handle_force(gen, state, frames, frame_count)        \
    do                                                                        \
    {                                                                         \
        if ((state)->force_slide != 0)                                        \
        {                                                                     \
            (state)->force *= (state)->force_slide_update;                    \
            (state)->force_slide_frames -= 1;                                 \
            if ((state)->force_slide_frames <= 0)                             \
            {                                                                 \
                (state)->force = (state)->force_slide_target;                 \
                (state)->force_slide = 0;                                     \
            }                                                                 \
            else if ((state)->force_slide == 1)                               \
            {                                                                 \
                if ((state)->force > (state)->force_slide_target)             \
                {                                                             \
                    (state)->force = (state)->force_slide_target;             \
                    (state)->force_slide = 0;                                 \
                }                                                             \
            }                                                                 \
            else                                                              \
            {                                                                 \
                assert((state)->force_slide == -1);                           \
                if ((state)->force < (state)->force_slide_target)             \
                {                                                             \
                    (state)->force = (state)->force_slide_target;             \
                    (state)->force_slide = 0;                                 \
                }                                                             \
            }                                                                 \
        }                                                                     \
        (state)->actual_force = (state)->force;                               \
        if ((state)->tremolo_length > 0 && (state)->tremolo_depth > 0)        \
        {                                                                     \
            double fac_dB = sin((state)->tremolo_phase) *                     \
                    (state)->tremolo_depth;                                   \
            (state)->actual_force *= exp2(fac_dB / 6);                        \
            if (!(state)->tremolo &&                                          \
                    (state)->tremolo_length > (state)->freq)                  \
            {                                                                 \
                (state)->tremolo_length = (state)->freq;                      \
                (state)->tremolo_update = (2 * PI) * (state)->tremolo_length; \
            }                                                                 \
            double new_phase = (state)->tremolo_phase +                       \
                    (state)->tremolo_update;                                  \
            if (new_phase >= (2 * PI))                                        \
            {                                                                 \
                new_phase = fmod(new_phase, (2 * PI));                        \
            }                                                                 \
            if (!(state)->tremolo && (new_phase < (state)->tremolo_phase      \
                        || (new_phase >= PI && (state)->tremolo_phase < PI))) \
            {                                                                 \
                (state)->tremolo_length = 0;                                  \
                (state)->tremolo_depth = 0;                                   \
                (state)->tremolo_phase = 0;                                   \
                (state)->tremolo_update = 0;                                  \
            }                                                                 \
            else                                                              \
            {                                                                 \
                (state)->tremolo_phase = new_phase;                           \
            }                                                                 \
        }                                                                     \
        for (int i = 0; i < (frame_count); ++i)                               \
        {                                                                     \
            (frames)[i] *= (state)->actual_force;                             \
        }                                                                     \
    } while (false)


