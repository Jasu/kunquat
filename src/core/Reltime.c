

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

#include "Reltime.h"


#ifndef NDEBUG
	#define Reltime_validate(r) assert((r) != NULL), assert((r)->part >= 0), assert ((r)->part < RELTIME_FULL_PART)
#else
	#define Reltime_validate(r) ((void)0)
#endif


Reltime* Reltime_init(Reltime* r)
{
	assert(r != NULL);
	r->beats = 0;
	r->part = 0;
	return r;
}


int Reltime_cmp(const Reltime* r1, const Reltime* r2)
{
	Reltime_validate(r1);
	Reltime_validate(r2);
	if (r1->beats < r2->beats)
		return -1;
	else if (r1->beats > r2->beats)
		return 1;
	else if (r1->part < r2->part)
		return -1;
	else if (r1->part > r2->part)
		return 1;
	return 0;
}


Reltime* Reltime_set(Reltime* r, int64_t beats, int32_t part)
{
	assert(r != NULL);
	assert(part >= 0);
	assert(part < RELTIME_FULL_PART);
	r->beats = beats;
	r->part = part;
	return r;
}


int64_t Reltime_get_beats(Reltime* r)
{
	assert(r != NULL);
	return r->beats;
}


int32_t Reltime_get_rem(Reltime* r)
{
	assert(r != NULL);
	return r->part;
}


Reltime* Reltime_add(Reltime* result, const Reltime* r1, const Reltime* r2)
{
	Reltime_validate(r1);
	Reltime_validate(r2);
	assert(result != NULL);
	result->beats = r1->beats + r2->beats;
	result->part = r1->part + r2->part;
	if (result->part >= RELTIME_FULL_PART)
	{
		++result->beats;
		result->part -= RELTIME_FULL_PART;
	}
	else if (result->part < 0)
	{
		--result->beats;
		result->part += RELTIME_FULL_PART;
	}
	assert(result->part >= 0);
	assert(result->part < RELTIME_FULL_PART);
	return result;
}


Reltime* Reltime_sub(Reltime* result, const Reltime* r1, const Reltime* r2)
{
	Reltime_validate(r1);
	Reltime_validate(r2);
	assert(result != NULL);
	result->beats = r1->beats - r2->beats;
	result->part = r1->part - r2->part;
	if (result->part < 0)
	{
		--result->beats;
		result->part += RELTIME_FULL_PART;
	}
	else if (result->part >= RELTIME_FULL_PART)
	{
		++result->beats;
		result->part -= RELTIME_FULL_PART;
	}
	assert(result->part >= 0);
	assert(result->part < RELTIME_FULL_PART);
	return result;
}


Reltime* Reltime_copy(Reltime* dest, const Reltime* src)
{
	assert(dest != NULL);
	Reltime_validate(src);
	dest->beats = src->beats;
	dest->part = src->part;
	return dest;
}


uint32_t Reltime_toframes(const Reltime* r,
		double tempo,
		uint32_t freq)
{
	Reltime_validate(r);
	assert(r->beats >= 0);
	assert(tempo > 0);
	assert(freq > 0);
	return (uint32_t)((r->beats
			+ ((double)r->part / RELTIME_FULL_PART)) * 60 * freq / tempo);
}


Reltime* Reltime_fromframes(Reltime* r,
		uint32_t frames,
		double tempo,
		uint32_t freq)
{
	assert(r != NULL);
	assert(tempo > 0);
	assert(freq > 0);
	double val = (double)frames * tempo / freq / 60;
	r->beats = (int64_t)val;
	r->part = (int32_t)((val - r->beats) * RELTIME_FULL_PART);
	return r;
}


