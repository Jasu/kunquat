

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KT_HANDLE_UTILS_H
#define KT_HANDLE_UTILS_H


#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <test_common.h>

#include <kunquat/Handle.h>
#include <kunquat/Player.h>


static kqt_Handle* handle = NULL;


typedef enum
{
    SONG_SELECTION_FIRST,
    SONG_SELECTION_LAST,
    SONG_SELECTION_ALL,
    SONG_SELECTION_COUNT
} Subsong_selection;


int songs[] =
{
    [SONG_SELECTION_FIRST] = 0,
    [SONG_SELECTION_LAST]  = KQT_SONGS_MAX - 1,
    [SONG_SELECTION_ALL]   = -1,
};


typedef enum
{
    MIXING_RATE_LOW,
    MIXING_RATE_CD,
    MIXING_RATE_DEFAULT,
    MIXING_RATE_HIGH,
    MIXING_RATE_COUNT
} Mixing_rate;


long mixing_rates[] =
{
    [MIXING_RATE_LOW]     = 8,
    [MIXING_RATE_CD]      = 44100,
    [MIXING_RATE_DEFAULT] = 48000,
    [MIXING_RATE_HIGH]    = 384000,
};


#define Note_On_55_Hz "[\"n+\", -3600]"


void check_unexpected_error()
{
    char* error_string = kqt_Handle_get_error(handle);
    fail_unless(
            strcmp(error_string, "") == 0,
            "Unexpected error"
            KT_VALUES("%s", "", error_string));
    return;
}


void setup_empty(void)
{
    assert(handle == NULL);
    handle = kqt_new_Handle();
    fail_if(handle == NULL,
            "Couldn't create handle:\n%s\n", kqt_Handle_get_error(NULL));
    return;
}


void handle_teardown(void)
{
    assert(handle != NULL);
    kqt_del_Handle(handle);
    handle = NULL;
    return;
}


void set_data(char* key, char* data)
{
    assert(handle != NULL);
    assert(key != NULL);
    assert(data != NULL);

    kqt_Handle_set_data(handle, key, data, strlen(data) + 1);
    check_unexpected_error();
}


#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))


void fail_buffers(
        float* expected,
        float* actual,
        int offset,
        int len)
{
    const int margin = 4;
    const int start_idx = max(0, offset - margin);
    const int end_idx = min(len, offset + margin + 1);

    char indices[128] =       "\n             ";
    char expected_vals[128] = "\n    Expected:";
    char actual_vals[128] =   "\n      Actual:";
    int chars_used = strlen(indices);

    char* indices_ptr = indices + chars_used;
    char* expected_vals_ptr = expected_vals + chars_used;
    char* actual_vals_ptr = actual_vals + chars_used;

    ++chars_used;

    for (int i = start_idx; i < end_idx; ++i)
    {
        const int chars_left = 128 - chars_used;
        assert(chars_left > 0);

        int ilen = snprintf(indices_ptr, chars_left, " %5d", i);
        int elen = snprintf(expected_vals_ptr, chars_left,
                " %5.1f", expected[i]);
        int alen = snprintf(actual_vals_ptr, chars_left,
                " %5.1f", actual[i]);
        assert(ilen == elen);
        assert(ilen == alen);
        chars_used += ilen;

        indices_ptr += ilen;
        expected_vals_ptr += elen;
        actual_vals_ptr += alen;
    }

    fail("Buffers differ at offset %d:%s%s%s",
            offset, indices, expected_vals, actual_vals);
}


void check_buffers_equal(
        float* expected,
        float* actual,
        int len,
        float eps)
{
    assert(expected != NULL);
    assert(actual != NULL);
    assert(len >= 0);
    assert(eps >= 0);

    for (int i = 0; i < len; ++i)
    {
        if (fabs(expected[i] - actual[i]) > eps)
        {
            fail_buffers(expected, actual, i, len);
            break;
        }
    }

    return;
}


void set_mixing_rate(long rate)
{
    assert(handle != NULL);
    assert(rate > 0);
    kqt_Handle_set_mixing_rate(handle, rate);
    check_unexpected_error();
    long actual_rate = kqt_Handle_get_mixing_rate(handle);
    check_unexpected_error();
    fail_unless(
            actual_rate == rate,
            "Wrong mixing rate"
            KT_VALUES("%ld", rate, actual_rate));
}


void set_mix_volume(double vol)
{
    assert(handle != NULL);

    char comp_def[] = "{ \"mix_vol\": -384.00000000 }";
    snprintf(comp_def, strlen(comp_def) + 1, "{ \"mix_vol\": %.4f }", vol);
    set_data("p_composition.json", comp_def);

    return;
}


void pause(void)
{
    assert(handle != NULL);

    kqt_Handle_fire(handle, 0, "[\"Ipause\", null]");
    check_unexpected_error();

    return;
}


#define repeat_seq_local(dest, times, seq) \
    repeat_seq((dest), (times), sizeof((seq)) / sizeof(float), (seq))

int repeat_seq(float* dest, int times, int seq_len, float* seq)
{
    assert(dest != NULL);
    assert(times >= 0);
    assert(seq_len >= 0);
    assert(seq != NULL);

    for (int i = 0; i < times; ++i)
    {
        memcpy(dest, seq, seq_len * sizeof(float));
        dest += seq_len;
    }

    return times * seq_len;
}


long mix_and_fill(float* buf, long nframes)
{
    assert(handle != NULL);
    assert(buf != NULL);
    assert(nframes >= 0);

    long mixed = kqt_Handle_mix(handle, nframes);
    check_unexpected_error();
    float* ret_buf = kqt_Handle_get_buffer(handle, 0);
    check_unexpected_error();
    memcpy(buf, ret_buf, nframes * sizeof(float));

    return mixed;
}


void setup_debug_instrument(void)
{
    assert(handle != NULL);

    set_data("p_connections.json",
            "[ [\"ins_00/out_00\", \"out_00\"] ]");

    set_data("ins_00/p_connections.json",
            "[ [\"gen_00/C/out_00\", \"out_00\"] ]");

    set_data("ins_00/gen_00/p_gen_type.json", "\"debug\"");

    return;
}


void setup_debug_single_pulse(void)
{
    assert(handle != NULL);

    set_data("ins_00/gen_00/c/p_single_pulse.jsonb", "true");
    check_unexpected_error();

    return;
}


#endif // KT_HANDLE_UTILS_H


