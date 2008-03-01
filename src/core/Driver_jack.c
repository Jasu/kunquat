

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


#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include <jack/jack.h>

#include <Player.h>
#include <Song.h>
#include "Driver_jack.h"

#include <frame_t.h>


/**
 * Mixing callback function for JACK.
 *
 * \param nframes   Number of frames to be mixed.
 * \param arg       A Player object. TODO: This will be changed into a list
 *                  of Player objects later.
 *
 * \return   Zero on success, non-zero on error.
 */
static int Driver_jack_process(jack_nframes_t nframes, void* arg);


/**
 * Freewheel callback function.
 *
 * \param starting   Freewheel state. Non-zero means freewheel, zero means
 *                   realtime.
 * \param arg        A Player object. TODO: This will be changed into a list
 *                   of Player objects later.
 */
//static void Driver_jack_freewheel(int starting, void* arg);


/**
 * Buffer size change callback function.
 *
 * \param nframes   New buffer size.
 * \param arg       A Player object. TODO: This will be changed into a list
 *                  of Player objects later.
 *
 * \return   Zero on success, non-zero on error.
 */
//static int Driver_jack_bufsize(jack_nframes_t nframes, void* arg);


/**
 * Sample rate change callback function.
 *
 * \param nframes   New sample rate.
 * \param arg       A Player object. TODO: This will be changed into a list
 *                  of Player objects later.
 *
 * \return   Zero on success, non-zero on error.
 */
//static int Driver_jack_sample_rate(jack_nframes_t nframes, void* arg);


/**
 * XRun callback function.
 *
 * \param arg   TODO: TBD
 *
 * \return   Zero on success, non-zero on error.
 */
//static int Driver_jack_xrun(void* arg);


/**
 * The JACK client handle.
 */
static jack_client_t* handle = NULL;


/**
 * Ports used for output. TODO: size
 */
static jack_port_t* ports[2] = { NULL };


/**
 * Active status of the driver.
 */
static bool active = false;


/**
 * Mixing rate.
 */
static uint32_t mix_freq = 48000;


bool Driver_jack_init(Playlist* playlist, uint32_t* freq)
{
	assert(playlist != NULL);
	assert(freq != NULL);
	jack_status_t status = 0;
	if (handle == NULL)
	{
		handle = jack_client_open("Kunquat", JackNullOption, &status);
		if (handle == NULL)
		{
			return false;
		}
		if (jack_set_process_callback(handle, Driver_jack_process, playlist) != 0)
		{
			handle = NULL;
			return false;
		}
	}
	if (ports[0] == NULL)
	{
		ports[0] = jack_port_register(handle,
				"out_l",
				JACK_DEFAULT_AUDIO_TYPE,
				JackPortIsOutput | JackPortIsTerminal, 0);
		if (ports[0] == NULL)
		{
			return false;
		}
	}
	if (ports[1] == NULL)
	{
		ports[1] = jack_port_register(handle,
				"out_r",
				JACK_DEFAULT_AUDIO_TYPE,
				JackPortIsOutput | JackPortIsTerminal, 0);
		if (ports[1] == NULL)
		{
			return false;
		}
	}
	if (!active)
	{
		if (jack_activate(handle) != 0)
		{
			return false;
		}
		active = true;
	}
	const char** available_ports = jack_get_ports(handle,
			NULL,
			NULL,
			JackPortIsPhysical | JackPortIsInput);
	if (available_ports == NULL)
	{
		return false;
	}
	if (jack_connect(handle,
			jack_port_name(ports[0]),
			available_ports[0]) != 0)
	{
		free(available_ports);
		return false;
	}
	if (jack_connect(handle,
			jack_port_name(ports[1]),
			available_ports[1]) != 0)
	{
		free(available_ports);
		return false;
	}
	free(available_ports);
	*freq = mix_freq = jack_get_sample_rate(handle);
	return true;
}


void Driver_jack_close(void)
{
	if (handle != NULL)
	{
		jack_client_close(handle);
		handle = NULL;
	}
	ports[0] = ports[1] = NULL;
	active = false;
	return;
}


static int Driver_jack_process(jack_nframes_t nframes, void* arg)
{
	assert(arg != NULL);
	if (!active)
	{
		return 0;
	}
	Playlist* playlist = (Playlist*)arg;
	Player* player = playlist->first;
	jack_default_audio_sample_t* jbuf_l = jack_port_get_buffer(ports[0], nframes);
	jack_default_audio_sample_t* jbuf_r = jack_port_get_buffer(ports[1], nframes);
	for (uint32_t i = 0; i < nframes; ++i)
	{
		jbuf_l[i] = jbuf_r[i] = 0;
	}
	while (player != NULL)
	{
		if (!player->play->mode)
		{
			player = player->next;
			continue;
		}
		assert(player->play->mode > STOP);
		assert(player->play->mode < PLAY_LAST);
		int buf_count = Song_get_buf_count(player->song);
		frame_t** bufs = Song_get_bufs(player->song);
		bufs[0] = jbuf_l;
		bufs[1] = jbuf_r;
		uint32_t mixed = Player_mix(player, nframes);
		if (buf_count == 1)
		{
			for (uint32_t i = 0; i < mixed; ++i)
			{
				jbuf_r[i] = jbuf_l[i];
			}
		}
		player = player->next;
	}
	return 0;
}


