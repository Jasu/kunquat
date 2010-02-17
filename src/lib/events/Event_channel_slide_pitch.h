

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_EVENT_CHANNEL_SLIDE_PITCH_H
#define K_EVENT_CHANNEL_SLIDE_PITCH_H


#include <Event_channel.h>
#include <Reltime.h>


typedef struct Event_channel_slide_pitch
{
    Event_channel parent;
    int64_t note;
    int64_t mod;
    int64_t octave;
} Event_channel_slide_pitch;


Event* new_Event_channel_slide_pitch(Reltime* pos);


bool Event_channel_slide_pitch_handle(Channel_state* ch_state, char* fields);


#endif // K_EVENT_CHANNEL_SLIDE_PITCH_H


