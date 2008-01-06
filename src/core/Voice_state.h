

/*
 * Copyright 2008 Tomi Jylhä-Ollila
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


#ifndef K_VOICE_STATE_H
#define K_VOICE_STATE_H


#include <stdbool.h>
#include <stdint.h>


typedef struct Voice_state
{
	/// Whether there is anything left to process.
	bool active;
	/// The frequency at which the note is played.
	double freq;
	/// The current playback position.
	uint64_t pos;
	/// The current playback position fine grain.
	double pos_part;
	/// The current relative playback position.
	uint64_t rel_pos;
	/// The current relative playback position fine grain.
	double rel_pos_part;
} Voice_state;


/**
 * Initialises a Voice state.
 *
 * \param state   The Voice state -- must not be \c NULL.
 *
 * \return   The parameter \a state.
 */
Voice_state* Voice_state_init(Voice_state* state);


#endif // K_VOICE_STATE_H


