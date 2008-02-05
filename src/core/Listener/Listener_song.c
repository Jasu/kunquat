

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
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "Listener.h"
#include "Listener_song.h"


int Listener_new_song(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data)
{
	(void)path;
	(void)types;
	(void)argv;
	(void)argc;
	(void)msg;
	assert(user_data != NULL);
	Listener* l = user_data;
	Player* player = NULL;
	Song* song = new_Song(2, 128, 16); // TODO: get params from relevant parts of the Listener
	if (song != NULL)
	{
		player = new_Player(1, l->voices, song); // TODO: freq
	}
	if (player == NULL)
	{
		if (song != NULL)
		{
			del_Song(song);
		}
		char* full_path = NULL;
		METHOD_PATH_ALLOC(full_path, l->host_path, "new_song");
		lo_send(l->host, full_path, "s", "Couldn't allocate memory");
		xfree(full_path);
		return 0;
	}
	assert(song != NULL);
	Playlist_ins(l->playlist, player);
	l->player_cur = player;
	char* full_path = NULL;
	METHOD_PATH_ALLOC(full_path, l->host_path, "new_song");
	int ret = lo_send(l->host, full_path, "i", player->id);
	xfree(full_path);
	if (ret == -1)
	{
		fprintf(stderr, "Failed to send the response message\n");
		return 0;
	}
	return 0;
}


int Listener_del_song(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data)
{
	(void)path;
	(void)types;
	(void)argc;
	(void)msg;
	assert(argv != NULL);
	assert(user_data != NULL);
	Listener* l = user_data;
	int32_t player_id = argv[0]->i;
	Player* target = l->player_cur;
	if (target == NULL || target->id != player_id)
	{
		target = Playlist_get(l->playlist, player_id);
	}
	if (target == NULL)
	{
		char* full_path = NULL;
		METHOD_PATH_ALLOC(full_path, l->host_path, "del_song");
		lo_send(l->host, full_path, "");
		xfree(full_path);
		return 0;
	}
	assert(target->id == player_id);
	Playlist_remove(l->playlist, target);
	l->player_cur = l->playlist->first;
	char* full_path = NULL;
	METHOD_PATH_ALLOC(full_path, l->host_path, "del_song");
	int ret = lo_send(l->host, full_path, "i", player_id);
	xfree(full_path);
	if (ret == -1)
	{
		fprintf(stderr, "Failed to send the response message\n");
		return 0;
	}
	return 0;
}


