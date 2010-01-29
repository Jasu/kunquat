

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


#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

#include <kunquat/limits.h>
#include <File_base.h>
#include <Subsong.h>

#include <xmemory.h>


static bool Subsong_parse(Subsong* ss, char* str, Read_state* state);


Subsong* new_Subsong(void)
{
    Subsong* ss = xalloc(Subsong);
    if (ss == NULL)
    {
        return NULL;
    }
    ss->res = 8;
    ss->pats = xnalloc(int16_t, ss->res);
    if (ss->pats == NULL)
    {
        xfree(ss);
        return NULL;
    }
    for (int i = 0; i < ss->res; ++i)
    {
        ss->pats[i] = KQT_SECTION_NONE;
    }
    ss->tempo = SUBSONG_DEFAULT_TEMPO;
    ss->global_vol = SUBSONG_DEFAULT_GLOBAL_VOL;
    ss->scale = SUBSONG_DEFAULT_SCALE;
    return ss;
}


Subsong* new_Subsong_from_string(char* str, Read_state* state)
{
    assert(state != NULL);
    Subsong* ss = new_Subsong();
    if (ss == NULL)
    {
        return NULL;
    }
    if (!Subsong_parse(ss, str, state))
    {
        del_Subsong(ss);
        return NULL;
    }
    return ss;
}


static bool Subsong_parse(Subsong* ss, char* str, Read_state* state)
{
    assert(ss != NULL);
    assert(state != NULL);
    if (str == NULL)
    {
        return true;
    }
    str = read_const_char(str, '{', state);
    if (state->error)
    {
        return false;
    }
    str = read_const_char(str, '}', state);
    if (!state->error)
    {
        return true;
    }
    Read_state_clear_error(state);
    char key[128] = { '\0' };
    bool expect_pair = true;
    while (expect_pair)
    {
        str = read_string(str, key, 128, state);
        str = read_const_char(str, ':', state);
        if (state->error)
        {
            return false;
        }
        if (strcmp(key, "tempo") == 0)
        {
            str = read_double(str, &ss->tempo, state);
            if (state->error)
            {
                return false;
            }
            if (ss->tempo < 1 || ss->tempo > 999)
            {
                Read_state_set_error(state, "Tempo (%f) is outside valid range", ss->tempo);
                return false;
            }
        }
        else if (strcmp(key, "global_vol") == 0)
        {
            str = read_double(str, &ss->global_vol, state);
        }
        else if (strcmp(key, "scale") == 0)
        {
            int64_t num = 0;
            str = read_int(str, &num, state);
            if (state->error)
            {
                return false;
            }
            if (num < 0 || num >= KQT_SCALES_MAX)
            {
                Read_state_set_error(state, "Scale number (%" PRId64
                                     ") is outside valid range", num);
                return false;
            }
            ss->scale = num;
        }
        else if (strcmp(key, "patterns") == 0)
        {
            str = read_const_char(str, '[', state);
            if (state->error)
            {
                return false;
            }
            str = read_const_char(str, ']', state);
            if (state->error)
            {
                Read_state_clear_error(state);
                bool expect_num = true;
                int index = 0;
                while (expect_num && index < KQT_SECTIONS_MAX)
                {
                    int64_t num = 0;
                    str = read_int(str, &num, state);
                    if (state->error)
                    {
                        return false;
                    }
                    if ((num < 0 || num >= KQT_PATTERNS_MAX) && num != KQT_SECTION_NONE)
                    {
                        Read_state_set_error(state,
                                 "Pattern number (%" PRId64 ") is outside valid range", num);
                        return false;
                    }
                    if (!Subsong_set(ss, index, num))
                    {
                        Read_state_set_error(state,
                                 "Couldn't allocate memory for a Subsong");
                        return false;
                    }
                    ++index;
                    check_next(str, state, expect_num);
                }
                str = read_const_char(str, ']', state);
                if (state->error)
                {
                    return false;
                }
            }
        }
        else
        {
            Read_state_set_error(state, "Unrecognised key in Subsong: %s\n", key);
            return false;
        }
        if (state->error)
        {
            return false;
        }
        check_next(str, state, expect_pair);
    }
    str = read_const_char(str, '}', state);
    if (state->error)
    {
        return false;
    }
    return true;
}


bool Subsong_set(Subsong* ss, int index, int16_t pat)
{
    assert(ss != NULL);
    assert(index >= 0);
    assert(index < KQT_SECTIONS_MAX);
    assert(pat >= 0 || pat == KQT_SECTION_NONE);
    if (index >= ss->res)
    {
        int new_res = ss->res << 1;
        if (index >= new_res)
        {
            new_res = index + 1;
        }
        int16_t* new_pats = xrealloc(int16_t, new_res, ss->pats);
        if (new_pats == NULL)
        {
            return false;
        }
        ss->pats = new_pats;
        for (int i = ss->res; i < new_res; ++i)
        {
            ss->pats[i] = KQT_SECTION_NONE;
        }
        ss->res = new_res;
    }
    ss->pats[index] = pat;
    return true;
}


int16_t Subsong_get(Subsong* ss, int index)
{
    assert(ss != NULL);
    assert(index >= 0);
    assert(index < KQT_SECTIONS_MAX);
    if (index >= ss->res)
    {
        return KQT_SECTION_NONE;
    }
    return ss->pats[index];
}


int16_t Subsong_get_length(Subsong* ss)
{
    assert(ss != NULL);
    int length = 0;
    for (length = 0; length < ss->res; ++length)
    {
        if (ss->pats[length] == KQT_SECTION_NONE)
        {
            break;
        }
    }
    return length;
}


void Subsong_set_tempo(Subsong* ss, double tempo)
{
    assert(ss != NULL);
    assert(isfinite(tempo));
    assert(tempo > 0);
    ss->tempo = tempo;
    return;
}


double Subsong_get_tempo(Subsong* ss)
{
    assert(ss != NULL);
    return ss->tempo;
}


void Subsong_set_global_vol(Subsong* ss, double vol)
{
    assert(ss != NULL);
    assert(isfinite(vol) || vol == -INFINITY);
    ss->global_vol = vol;
    return;
}


double Subsong_get_global_vol(Subsong* ss)
{
    assert(ss != NULL);
    return ss->global_vol;
}


void Subsong_set_scale(Subsong* ss, int index)
{
    assert(ss != NULL);
    assert(index >= 0);
    assert(index < KQT_SCALES_MAX);
    ss->scale = index;
    return;
}


int Subsong_get_scale(Subsong* ss)
{
    assert(ss != NULL);
    return ss->scale;
}


#if 0
void Subsong_clear(Subsong* ss)
{
    assert(ss != NULL);
    for (int i = 0; i < ss->res; ++i)
    {
        ss->pats[i] = KQT_SECTION_NONE;
    }
    return;
}
#endif


void del_Subsong(Subsong* ss)
{
    assert(ss != NULL);
    xfree(ss->pats);
    xfree(ss);
    return;
}

