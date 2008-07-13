

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


#define _POSIX_C_SOURCE 1

#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <Sample.h>
#include "File_wavpack.h"

#include <wavpack/wavpack.h>

#include <xmemory.h>


typedef struct File_context
{
	FILE* file;
	uint64_t offset;
} File_context;


static int32_t read_bytes(void* id, void* data, int32_t bcount);
static uint32_t get_pos(void* id);
static int set_pos_abs(void* id, uint32_t pos);
static int set_pos_rel(void* id, int32_t delta, int mode);
static int push_back_byte(void* id, int c);
static uint32_t get_length(void* id);
static int can_seek(void* id);
static int32_t write_bytes(void* id, void* data, int32_t bcount);


static int32_t read_bytes(void* id, void* data, int32_t bcount)
{
	assert(id != NULL);
	File_context* fc = id;
	assert(fc->file != NULL);
	return (int32_t)fread(data, 1, bcount, fc->file);
}


static uint32_t get_pos(void* id)
{
	assert(id != NULL);
	File_context* fc = id;
	assert(fc->file != NULL);
	assert(ftell(fc->file) >= (int64_t)fc->offset);
	return ftell(fc->file) - fc->offset;
}


static int set_pos_abs(void* id, uint32_t pos)
{
	assert(id != NULL);
	File_context* fc = id;
	assert(fc->file != NULL);
	return fseek(fc->file, pos + fc->offset, SEEK_SET);
}


static int set_pos_rel(void* id, int32_t delta, int mode)
{
	assert(id != NULL);
	File_context* fc = id;
	assert(fc->file != NULL);
	int ret = fseek(fc->file, delta, mode);
	if (ret != 0)
	{
		return ret;
	}
	if (ftell(fc->file) < (int64_t)fc->offset)
	{
		return fseek(fc->file, fc->offset, SEEK_SET);
	}
	return ret;
}


static int push_back_byte(void* id, int c)
{
	assert(id != NULL);
	File_context* fc = id;
	assert(fc->file != NULL);
	return ungetc(c, fc->file);
}


static uint32_t get_length(void* id)
{
	assert(id != NULL);
	File_context* fc = id;
	assert(fc->file != NULL);
	struct stat buf;
	if (fstat(fileno(fc->file), &buf) != 0)
	{
		return 0;
	}
	if (S_ISREG(buf.st_mode) == 0)
	{
		return 0;
	}
	return buf.st_size;
}


static int can_seek(void* id)
{
	assert(id != NULL);
	File_context* fc = id;
	assert(fc->file != NULL);
	struct stat buf;
	if (fstat(fileno(fc->file), &buf) != 0)
	{
		return 0;
	}
	if (S_ISREG(buf.st_mode) == 0)
	{
		return 0;
	}
	return 1;
}


static int32_t write_bytes(void* id, void* data, int32_t bcount)
{
	(void)id;
	(void)data;
	(void)bcount;
	assert(false);
	return 0;
}


static WavpackStreamReader reader =
{
	read_bytes,
	get_pos,
	set_pos_abs,
	set_pos_rel,
	push_back_byte,
	get_length,
	can_seek,
	write_bytes
};


#define read_wp_samples(count, offset, buf_l, buf_r, src, channels, lshift) do\
	{\
		assert(buf_l != NULL);\
		for (uint32_t i = 0; i < count / channels; ++i)\
		{\
			buf_l[offset + i] = src[i * channels] << lshift;\
		}\
		if (channels == 2)\
		{\
			assert(buf_r != NULL);\
			for (uint32_t i = 0; i < count / channels; ++i)\
			{\
				buf_r[offset + i] = src[i * channels + 1];\
			}\
		}\
	} while (false)


bool File_wavpack_load_sample(Sample* sample, FILE* in)
{
	assert(sample != NULL);
	assert(in != NULL);
	File_context fc = { .file = in, .offset = ftell(in) };
	char err_str[80] = { '\0' };
	WavpackContext* context = WavpackOpenFileInputEx(&reader,
			&fc,
			NULL,
			err_str,
			OPEN_2CH_MAX | OPEN_NORMALIZE,
			0);
	if (context == NULL)
	{
		return false;
	}
	int mode = WavpackGetMode(context);
	int channels = WavpackGetReducedChannels(context);
	uint32_t freq = WavpackGetSampleRate(context);
	int bits = WavpackGetBitsPerSample(context);
	int bytes = WavpackGetBytesPerSample(context);
	uint32_t len = WavpackGetNumSamples(context);
//	uint32_t file_size = WavpackGetFileSize(context);
	if (len == (uint32_t)-1)
	{
		WavpackCloseFile(context);
		return false;
	}
	sample->format = SAMPLE_FORMAT_WAVPACK;
	sample->len = len;
	sample->channels = channels;
	sample->mid_freq = freq;
	if (bits <= 8)
	{
		sample->bits = 8;
	}
	else if (bits <= 16)
	{
		sample->bits = 16;
	}
	else if (bits <= 24)
	{
		sample->bits = 24;
	}
	else
	{
		sample->bits = 32;
	}
	if ((mode & MODE_FLOAT))
	{
		sample->is_float = true;
		sample->bits = 32;
	}
	int req_bytes = sample->bits / 8;
	void* nbuf_l = xnalloc(char, sample->len * req_bytes);
	if (nbuf_l == NULL)
	{
		WavpackCloseFile(context);
		return false;
	}
	if (channels == 2)
	{
		void* nbuf_r = xnalloc(char, sample->len * req_bytes);
		if (nbuf_r == NULL)
		{
			xfree(nbuf_l);
			WavpackCloseFile(context);
			return false;
		}
		sample->data[1] = nbuf_r;
	}
	sample->data[0] = nbuf_l;
	int32_t buf[256] = { 0 };
	uint32_t read = WavpackUnpackSamples(context, buf,
			256 / sample->channels);
	uint32_t written = 0;
	while (read > 0 && written < sample->len)
	{
		if (req_bytes == 1)
		{
			int8_t* buf_l = sample->data[0];
			int8_t* buf_r = NULL;
			if (sample->channels == 2)
			{
				buf_r = sample->data[1];
			}
			read_wp_samples(read, written, buf_l, buf_r, buf,
					sample->channels, 0);
		}
		else if (req_bytes == 2)
		{
			int16_t* buf_l = sample->data[0];
			int16_t* buf_r = NULL;
			if (sample->channels == 2)
			{
				buf_r = sample->data[1];
			}
			read_wp_samples(read, written, buf_l, buf_r, buf,
					sample->channels, 0);
		}
		else
		{
			if (sample->is_float)
			{
				float* buf_l = sample->data[0];
				float* buf_r = NULL;
				if (sample->channels == 2)
				{
					buf_r = sample->data[1];
				}
				read_wp_samples(read, written, buf_l, buf_r, buf,
						sample->channels, 0);
			}
			else
			{
				int32_t* buf_l = sample->data[0];
				int32_t* buf_r = NULL;
				int shift = 0;
				if (bytes == 3)
				{
					shift = 8;
				}
				if (sample->channels == 2)
				{
					buf_r = sample->data[1];
				}
				read_wp_samples(read, written, buf_l, buf_r, buf,
						sample->channels, shift);
			}
		}
		written += read;
		read = WavpackUnpackSamples(context, buf, 256 / sample->channels);
	}
	WavpackCloseFile(context);
	if (written < sample->len)
	{
		xfree(sample->data[0]);
		xfree(sample->data[1]);
		sample->data[0] = sample->data[1] = NULL;
		return false;
	}
	return true;
}

