

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
#include <stdbool.h>
#include <stdio.h>

#include "Listener.h"
#include "Listener_ins.h"
#include "Listener_ins_pcm.h"

#include <Song.h>
#include <Ins_table.h>
#include <Instrument.h>
#include <Instrument_pcm.h>
#include <Song_limits.h>


int Listener_ins_pcm_load_sample(const char* path,
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
	assert(user_data != NULL);
	Listener* lr = user_data;
	if (lr->host == NULL)
	{
		return 0;
	}
	assert(lr->method_path != NULL);
	int32_t song_id = argv[0]->i;
	int32_t ins_num = argv[1]->i;
	Instrument* ins = NULL;
	if (!ins_get(lr, song_id, ins_num, &ins))
	{
		return 0;
	}
	check_cond(lr, ins != NULL,
			"The Instrument #%ld", (long)ins_num);
	check_cond(lr, Instrument_get_type(ins) == INS_TYPE_PCM,
			"The Instrument type (%d)", (int)Instrument_get_type(ins));
	int32_t sample_index = argv[2]->i;
	check_cond(lr, sample_index >= 0 && sample_index < PCM_SAMPLES_MAX,
			"The Sample index (%ld)", (long)sample_index);
	char* spath = &argv[3]->s;
	if (!Instrument_pcm_set_sample(ins, sample_index, spath))
	{
		lo_message m = new_msg();
		lo_message_add_string(m, "Couldn't load sample");
		lo_message_add_string(m, spath);
		int ret = 0;
		send_msg(lr, "error", m, ret);
		lo_message_free(m);
	}
	ins_info(lr, song_id, ins_num, ins);
	return 0;
}


int Listener_ins_pcm_sample_set_mid_freq(const char* path,
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
	assert(user_data != NULL);
	Listener* lr = user_data;
	if (lr->host == NULL)
	{
		return 0;
	}
	assert(lr->method_path != NULL);
	int32_t song_id = argv[0]->i;
	int32_t ins_num = argv[1]->i;
	Instrument* ins = NULL;
	if (!ins_get(lr, song_id, ins_num, &ins))
	{
		return 0;
	}
	check_cond(lr, ins != NULL,
			"The Instrument #%ld", (long)ins_num);
	check_cond(lr, Instrument_get_type(ins) == INS_TYPE_PCM,
			"The Instrument type (%d)", (int)Instrument_get_type(ins));
	int32_t sample_index = argv[2]->i;
	check_cond(lr, sample_index >= 0 && sample_index < PCM_SAMPLES_MAX,
			"The Sample index (%ld)", (long)sample_index);
	double freq = argv[3]->d;
	check_cond(lr, freq > 0, "The frequency (%f)", freq);
	Instrument_pcm_set_sample_freq(ins, sample_index, freq);
	ins_info(lr, song_id, ins_num, ins);
	return 0;
}


int Listener_ins_pcm_remove_sample(const char* path,
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
	assert(user_data != NULL);
	Listener* lr = user_data;
	if (lr->host == NULL)
	{
		return 0;
	}
	assert(lr->method_path != NULL);
	int32_t song_id = argv[0]->i;
	int32_t ins_num = argv[1]->i;
	Instrument* ins = NULL;
	if (!ins_get(lr, song_id, ins_num, &ins))
	{
		return 0;
	}
	check_cond(lr, ins != NULL,
			"The Instrument #%ld", (long)ins_num);
	check_cond(lr, Instrument_get_type(ins) == INS_TYPE_PCM,
			"The Instrument type (%d)", (int)Instrument_get_type(ins));
	int32_t sample_index = argv[2]->i;
	check_cond(lr, sample_index >= 0 && sample_index < PCM_SAMPLES_MAX,
			"The Sample index (%ld)", (long)sample_index);
	if (!Instrument_pcm_set_sample(ins, sample_index, NULL))
	{
		lo_message m = new_msg();
		lo_message_add_string(m, "Couldn't remove sample");
		lo_message_add_int32(m, sample_index);
		int ret = 0;
		send_msg(lr, "error", m, ret);
		lo_message_free(m);
	}
	ins_info(lr, song_id, ins_num, ins);
	return 0;
}


bool ins_info_pcm(Listener* lr, lo_message m, Instrument* ins)
{
	assert(lr != NULL);
	assert(m != NULL);
	assert(ins != NULL);
	assert(Instrument_get_type(ins) == INS_TYPE_PCM);
	pcm_type_data* type_data = ins->type_data;
	for (uint16_t i = 0; i < PCM_SAMPLES_MAX; ++i)
	{
		if (Instrument_pcm_get_sample(ins, i) == NULL)
		{
			continue;
		}
		lo_message_add_int32(m, i);
		lo_message_add_string(m, Instrument_pcm_get_path(ins, i));
		lo_message_add_double(m,
				Instrument_pcm_get_sample_freq(ins, i));
	}
	lo_message_add_string(m, "__styles");
	// TODO: send styles
	lo_message_add_string(m, "__maps");
	// TODO: send all maps
	lo_message_add_int32(m, 0); // source
	lo_message_add_int32(m, 0); // style
	lo_message_add_int32(m, 1); // # of strength levels

	lo_message_add_double(m, 0); // strength threshold
	lo_message_add_int32(m, type_data->freq_maps[0].entry_count); // # of frequency levels

	freq_entry* key = &(freq_entry){ .freq = 0 };
	freq_entry* entry = AAtree_get(type_data->freq_maps[0].tree, key, 1);
	while (entry != NULL)
	{
		lo_message_add_double(m, entry->freq);
		lo_message_add_int32(m, entry->choices);
		for (int i = 0; i < entry->choices; ++i)
		{
			lo_message_add_int32(m, entry->sample[i]);
			lo_message_add_double(m, entry->freq_scale[i]);
			lo_message_add_double(m, entry->vol_scale[i]);
		}
		entry = AAtree_get_next(type_data->freq_maps[0].tree, 1);
	}
	
	return true;
}

/* \li        For each strength level:
 * \li \li \c d   The lower threshold value for the level.
 * \li \li \c i   The number of frequency levels.
 * \li \li        For each frequency level:
 * \li \li \li \c d   The lower threshold frequency for the level.
 * \li \li \li \c i   The number of samples for this frequency/strength level.
 * \li \li \li        For each sample:
 * \li \li \li \li \c i   The Sample number.
 * \li \li \li \li \c d   The frequency scale factor for this sample.
 * \li \li \li \li \c d   The volume scale factor for this sample.
 */


