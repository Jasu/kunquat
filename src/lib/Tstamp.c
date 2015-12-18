

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <Tstamp.h>

#include <debug/assert.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


#ifndef NDEBUG
    #define Tstamp_validate(ts)              \
    if (true)                                \
    {                                        \
        assert((ts) != NULL);                \
        assert((ts)->rem >= 0);              \
        assert((ts)->rem < KQT_TSTAMP_BEAT); \
    } else (void)0
#else
    #define Tstamp_validate(ts) (ignore(ts))
#endif


Tstamp* Tstamp_init(Tstamp* ts)
{
    assert(ts != NULL);

    ts->beats = 0;
    ts->rem = 0;

    return ts;
}


int Tstamp_cmp(const Tstamp* ts1, const Tstamp* ts2)
{
    Tstamp_validate(ts1);
    Tstamp_validate(ts2);

    if (ts1->beats < ts2->beats)
        return -1;
    else if (ts1->beats > ts2->beats)
        return 1;
    else if (ts1->rem < ts2->rem)
        return -1;
    else if (ts1->rem > ts2->rem)
        return 1;
    return 0;
}


Tstamp* Tstamp_set(Tstamp* ts, int64_t beats, int32_t rem)
{
    assert(ts != NULL);
    assert(rem >= 0);
    assert(rem < KQT_TSTAMP_BEAT);

    ts->beats = beats;
    ts->rem = rem;

    return ts;
}


int64_t Tstamp_get_beats(const Tstamp* ts)
{
    assert(ts != NULL);
    return ts->beats;
}


int32_t Tstamp_get_rem(const Tstamp* ts)
{
    assert(ts != NULL);
    return ts->rem;
}


Tstamp* Tstamp_add(Tstamp* result, const Tstamp* ts1, const Tstamp* ts2)
{
    assert(result != NULL);
    Tstamp_validate(ts1);
    Tstamp_validate(ts2);

    result->beats = ts1->beats + ts2->beats;
    result->rem = ts1->rem + ts2->rem;
    if (result->rem >= KQT_TSTAMP_BEAT)
    {
        ++result->beats;
        result->rem -= KQT_TSTAMP_BEAT;
    }
    else if (result->rem < 0)
    {
        --result->beats;
        result->rem += KQT_TSTAMP_BEAT;
    }
    assert(result->rem >= 0);
    assert(result->rem < KQT_TSTAMP_BEAT);

    return result;
}


Tstamp* Tstamp_sub(Tstamp* result, const Tstamp* ts1, const Tstamp* ts2)
{
    assert(result != NULL);
    Tstamp_validate(ts1);
    Tstamp_validate(ts2);

    result->beats = ts1->beats - ts2->beats;
    result->rem = ts1->rem - ts2->rem;
    if (result->rem < 0)
    {
        --result->beats;
        result->rem += KQT_TSTAMP_BEAT;
    }
    else if (result->rem >= KQT_TSTAMP_BEAT)
    {
        ++result->beats;
        result->rem -= KQT_TSTAMP_BEAT;
    }
    assert(result->rem >= 0);
    assert(result->rem < KQT_TSTAMP_BEAT);

    return result;
}


Tstamp* Tstamp_copy(Tstamp* dest, const Tstamp* src)
{
    assert(dest != NULL);
    Tstamp_validate(src);

    dest->beats = src->beats;
    dest->rem = src->rem;

    return dest;
}


Tstamp* Tstamp_min(Tstamp* result, const Tstamp* ts1, const Tstamp* ts2)
{
    assert(result != NULL);
    Tstamp_validate(ts1);
    Tstamp_validate(ts2);

    if (Tstamp_cmp(ts1, ts2) < 0)
        Tstamp_copy(result, ts1);
    else
        Tstamp_copy(result, ts2);

    return result;
}


double Tstamp_toframes(
        const Tstamp* ts,
        double tempo,
        uint32_t rate)
{
    Tstamp_validate(ts);
    assert(ts->beats >= 0);
    assert(tempo > 0);
    assert(rate > 0);

    return (ts->beats + ((double)ts->rem / KQT_TSTAMP_BEAT)) *
            60 * rate / tempo;
}


Tstamp* Tstamp_fromframes(
        Tstamp* ts,
        uint32_t frames,
        double tempo,
        uint32_t rate)
{
    assert(ts != NULL);
    assert(tempo > 0);
    assert(rate > 0);

    double val = (double)frames * tempo / rate / 60;
    ts->beats = (int64_t)val;
    ts->rem = (int32_t)((val - ts->beats) * KQT_TSTAMP_BEAT);

    return ts;
}


