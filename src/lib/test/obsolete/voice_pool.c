

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
#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <math.h>

#include <check.h>

#include <Real.h>
#include <Scale.h>
#include <Reltime.h>
#include <Event.h>
#include <Event_voice_note_on.h>
#include <Event_voice_note_off.h>
#include <Generator_debug.h>
#include <Instrument.h>
#include <Channel_state.h>
#include <Voice.h>
#include <Voice_pool.h>


Suite* Voice_pool_suite(void);


START_TEST (new)
{
    Voice_pool* pool = new_Voice_pool(1, 1);
    if (pool == NULL)
    {
        fprintf(stderr, "new_Voice_pool() returned NULL -- out of memory?\n");
        abort();
    }
    del_Voice_pool(pool);
}
END_TEST

#ifndef NDEBUG
START_TEST (new_break_size_inv)
{
    new_Voice_pool(0, 1);
}
END_TEST

START_TEST (new_break_events_inv)
{
    new_Voice_pool(1, 0);
}
END_TEST
#endif


START_TEST (get_voice)
{
    kqt_frame buf_l[1] = { 0 };
    kqt_frame buf_r[1] = { 0 };
    kqt_frame* bufs[2] = { buf_l, buf_r };
    kqt_frame* vbufs[2] = { buf_l, buf_r };
    Scale* nts[KQT_SCALES_MAX] = { NULL };
    Scale** default_scale = &nts[0];
    Instrument* ins = new_Instrument(bufs, vbufs, vbufs, 2, 1, nts, &default_scale, 1);
    if (ins == NULL)
    {
        fprintf(stderr, "new_Instrument() returned NULL -- out of memory?\n");
        abort();
    }
    Generator_debug* gen_debug = new_Generator_debug(Instrument_get_params(ins));
    if (gen_debug == NULL)
    {
        fprintf(stderr, "new_Generator_debug() returned NULL -- out of memory?\n");
        abort();
    }
    Instrument_set_gen(ins, 0, (Generator*)gen_debug);
    Voice_pool* pool = new_Voice_pool(2, 1);
    if (pool == NULL)
    {
        fprintf(stderr, "new_Voice_pool() returned NULL -- out of memory?\n");
        abort();
    }
    Voice* v0 = Voice_pool_get_voice(pool, NULL, 0);
    fail_if(v0 == NULL,
            "Voice_pool_get_voice() returned NULL unexpectedly.");
    fail_unless(v0->prio == VOICE_PRIO_INACTIVE,
            "Voice_pool_get_voice() returned a Voice with active status.");
    uint64_t id0 = Voice_id(v0);
    bool mute = false;
    Channel_state ch_state;
    Channel_state_init(&ch_state, 0, &mute);
    Voice_init(v0, Instrument_get_gen(ins, 0), &ch_state, &ch_state, 64, 120);
    Voice* v1 = Voice_pool_get_voice(pool, NULL, 0);
    fail_if(v1 == NULL,
            "Voice_pool_get_voice() returned NULL unexpectedly.");
    fail_if(v0 == v1,
            "Voice_pool_get_voice() returned an active Voice when inactive Voice was available.");
    fail_unless(v1->prio == VOICE_PRIO_INACTIVE,
            "Voice_pool_get_voice() returned a Voice with active status.");
    uint64_t id1 = Voice_id(v1);
    fail_if(id0 == id1,
            "Voice_pool_get_voice() returned a non-unique ID.");
    Voice_init(v1, Instrument_get_gen(ins, 0), &ch_state, &ch_state, 64, 120);
    Voice* v2 = Voice_pool_get_voice(pool, v0, id0);
    fail_unless(v0 == v2,
            "Voice_pool_get_voice() didn't return the expected Voice with correct ID.");
    v2 = Voice_pool_get_voice(pool, v0, id1);
    fail_unless(v2 == NULL,
            "Voice_pool_get_voice() didn't return NULL when requested a Voice with the wrong ID.");
    v2 = Voice_pool_get_voice(pool, NULL, 0);
    fail_if(v2 == NULL,
            "Voice_pool_get_voice() returned NULL unexpectedly.");
    fail_unless(v2 == v0 || v2 == v1,
            "Voice_pool_get_voice() returned a third Voice from a pool of two Voices.");
    fail_unless(v2->prio == VOICE_PRIO_INACTIVE,
            "Voice_pool_get_voice() returned a Voice with active status.");
    uint64_t id2 = Voice_id(v2);
    fail_if(id2 == id0,
            "Voice_pool_get_voice() returned a non-unique ID.");
    fail_if(id2 == id1,
            "Voice_pool_get_voice() returned a non-unique ID.");
    del_Instrument(ins);
    del_Voice_pool(pool);
}
END_TEST

#ifndef NDEBUG
START_TEST (get_voice_break_pool_null)
{
    Voice_pool_get_voice(NULL, NULL, 0);
}
END_TEST
#endif


#if 0
START_TEST (mix)
{
    kqt_frame buf_l[128] = { 0 };
    kqt_frame buf_r[128] = { 0 };
    kqt_frame* bufs[2] = { buf_l, buf_r };
    kqt_frame* vbufs[2] = { buf_l, buf_r };
    Scale* nts[KQT_SCALES_MAX] = { NULL };
    Scale** default_scale = &nts[0];
    Instrument* ins = new_Instrument(bufs, vbufs, vbufs, 2, 128, nts, &default_scale, 16);
    if (ins == NULL)
    {
        fprintf(stderr, "new_Instrument() returned NULL -- out of memory?\n");
        abort();
    }
    Generator_debug* gen_debug = new_Generator_debug(Instrument_get_params(ins));
    if (gen_debug == NULL)
    {
        fprintf(stderr, "new_Generator_debug() returned NULL -- out of memory?\n");
        abort();
    }
    Instrument_set_gen(ins, 0, (Generator*)gen_debug);
    Scale* notes = new_Scale(2, Real_init_as_frac(REAL_AUTO, 2, 1));
    if (notes == NULL)
    {
        fprintf(stderr, "new_Scale() returned NULL -- out of memory?\n");
        abort();
    }
    nts[0] = notes;
    Scale_set_note(notes, 0, Real_init(REAL_AUTO));
    Instrument_set_scale(ins, 0);
    Voice_pool* pool = new_Voice_pool(2, 16);
    if (pool == NULL)
    {
        fprintf(stderr, "new_Voice_pool() returned NULL -- out of memory?\n");
        abort();
    }
    Event* ev1_on = new_Event_voice_note_on(Reltime_init(RELTIME_AUTO));
    if (ev1_on == NULL)
    {
        fprintf(stderr, "new_Event() returned NULL -- out of memory?\n");
        abort();
    }
    Event* ev1_off = new_Event_voice_note_off(Reltime_init(RELTIME_AUTO));
    if (ev1_off == NULL)
    {
        fprintf(stderr, "new_Event() returned NULL -- out of memory?\n");
        abort();
    }
    Event* ev2_on = new_Event_voice_note_on(Reltime_init(RELTIME_AUTO));
    if (ev2_on == NULL)
    {
        fprintf(stderr, "new_Event() returned NULL -- out of memory?\n");
        abort();
    }
    Event* ev2_off = new_Event_voice_note_off(Reltime_init(RELTIME_AUTO));
    if (ev2_off == NULL)
    {
        fprintf(stderr, "new_Event() returned NULL -- out of memory?\n");
        abort();
    }
    
    // Testing scenario 1:
    //
    // Mixing frequency is 8 Hz.
    // Tempo is 60 BPM.
    // Playing a note of debug instrument.
    // Note frequency is 2 Hz (2 cycles/beat).
    // Note starts at the beginning and plays until the end
    // Result should be (1, 0.5, 0.5, 0.5) 10 times, the rest are zero.
    int64_t note = 0;
    int64_t mod = -1;
    int64_t octave = KQT_SCALE_MIDDLE_OCTAVE;
    Event_set_field(ev1_on, 0, &note);
    Event_set_field(ev1_on, 1, &mod);
    Event_set_field(ev1_on, 2, &octave);
    Voice* v1 = Voice_pool_get_voice(pool, NULL, 0);
    fail_if(v1 == NULL,
            "Voice_pool_get_voice() returned NULL unexpectedly.");
//  uint64_t id1 = Voice_id(v1);
    bool mute = false;
    Channel_state ch_state;
    Channel_state_init(&ch_state, 0, &mute);
    Voice_init(v1, Instrument_get_gen(ins, 0), &ch_state, &ch_state, 64, 120);
    fail_unless(Voice_add_event(v1, ev1_on, 0),
            "Voice_add_event() failed.");
    Voice_pool_mix(pool, 128, 0, 8, 120);
    for (int i = 0; i < 40; ++i)
    {
        if (i % 4 == 0)
        {
            fail_unless(bufs[0][i] > 0.99 && bufs[0][i] < 1.01,
                    "Buffer contains %f at index %d (expected 1).", bufs[0][i], i);
        }
        else
        {
            fail_unless(bufs[0][i] > 0.49 && bufs[0][i] < 0.51,
                    "Buffer contains %f at index %d (expected 0.5).", bufs[0][i], i);
        }
    }
    for (int i = 40; i < 128; ++i)
    {
        fail_unless(fabs(bufs[0][i]) < 0.01,
                "Buffer contains %f at index %d (expected 0).", bufs[0][i], i);
    }
    
    // Testing scenario 2:
    //
    // Mixing frequency is 8 Hz.
    // Tempo is 60 BPM.
    // Playing a note of debug instrument.
    // Note #1 frequency is 1 Hz (1 cycle/beat).
    // Note #1 starts at the beginning and plays until the end
    // Note #2 frequency is 2 Hz (2 cycles/beat).
    // Note #2 starts at frame 2 and plays until the end
    // Result should be (1, 0.5),
    //    (1.5, 1, 1, 1, 1.5, 1, 1.5, 1) 5 times,
    //    (0.5, 0.5, 0.5, 0.5, 0.5, 0.5) once,
    //    (1, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5) 4 times, the rest are zero.
    for (int i = 0; i < 128; ++i)
    {
        buf_l[i] = buf_r[i] = 0;
    }
    Voice_pool_reset(pool);
    v1 = Voice_pool_get_voice(pool, NULL, 0);
    fail_if(v1 == NULL,
            "Voice_pool_get_voice() returned NULL unexpectedly.");
    Voice_init(v1, Instrument_get_gen(ins, 0), &ch_state, &ch_state, 64, 120);
    note = 0;
    mod = -1;
    octave = KQT_SCALE_MIDDLE_OCTAVE - 1;
    Event_set_field(ev1_on, 0, &note);
    Event_set_field(ev1_on, 1, &mod);
    Event_set_field(ev1_on, 2, &octave);
    fail_unless(Voice_add_event(v1, ev1_on, 0),
            "Voice_add_event() failed.");
    Voice* v2 = Voice_pool_get_voice(pool, NULL, 0);
    fail_if(v2 == NULL,
            "Voice_pool_get_voice() returned NULL unexpectedly.");
    fail_if(v1 == v2,
            "Voice_pool_get_voice() returned an active Voice when inactive Voice was available.");
    Voice_init(v2, Instrument_get_gen(ins, 0), &ch_state, &ch_state, 64, 120);
    note = 0;
    mod = -1;
    octave = KQT_SCALE_MIDDLE_OCTAVE;
    Event_set_field(ev2_on, 0, &note);
    Event_set_field(ev2_on, 1, &mod);
    Event_set_field(ev2_on, 2, &octave);
    fail_unless(Voice_add_event(v2, ev2_on, 2),
            "Voice_add_event() failed.");
    Voice_pool_mix(pool, 128, 0, 8, 120);
    fail_unless(bufs[0][0] > 0.99 && bufs[0][0] < 1.01,
            "Buffer contains %f at index %d (expected 1).", bufs[0][0], 0);
    fail_unless(bufs[0][1] > 0.49 && bufs[0][1] < 0.51,
            "Buffer contains %f at index %d (expected 0.5).", bufs[0][1], 1);
    for (int i = 2; i < 42; ++i)
    {
        if (i % 8 == 0 || i % 4 == 2)
        {
            fail_unless(bufs[0][i] > 1.49 && bufs[0][i] < 1.51,
                    "Buffer contains %f at index %d (expected 1.5).", bufs[0][i], i);
        }
        else
        {
            fail_unless(bufs[0][i] > 0.99 && bufs[0][i] < 1.01,
                    "Buffer contains %f at index %d (expected 1).", bufs[0][i], i);
        }
    }
    for (int i = 42; i < 80; ++i)
    {
        if (i % 8 == 0)
        {
            fail_unless(bufs[0][i] > 0.99 && bufs[0][i] < 1.01,
                    "Buffer contains %f at index %d (expected 1).", bufs[0][i], i);
        }
        else
        {
            fail_unless(bufs[0][i] > 0.49 && bufs[0][i] < 0.51,
                    "Buffer contains %f at index %d (expected 0.5).", bufs[0][i], i);
        }
    }
    for (int i = 80; i < 128; ++i)
    {
        fail_unless(fabs(bufs[0][i]) < 0.01,
                "Buffer contains %f at index %d (expected 0).", bufs[0][i], i);
    }
    
    // Testing scenario 3:
    //
    // Mixing frequency is 8 Hz.
    // Tempo is 60 BPM.
    // Playing a note of debug instrument.
    // Note #1 frequency is 1 Hz (1 cycle/beat).
    // Note #1 starts at the beginning and plays until the end.
    // Note #2 frequency is 2 Hz (2 cycles/beat).
    // Note #2 starts at the beginning and is released at frame 20.
    // Note #3 frequency is 2 Hz (2 cycles/beat).
    // Note #3 starts at frame 22 and plays until the end.
    // Result should be (2, 1, 1, 1, 1.5, 1, 1, 1) twice,
    //    (2, 1, 1, 1) once,
    //    (-0.5, 0) once,
    //    (1.5, 1, 1.5, 1, 1.5, 1, 1, 1) 5 times,
    //    (0.5, 0.5) once,
    //    (1, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5) twice, the rest are zero.
    for (int i = 0; i < 128; ++i)
    {
        buf_l[i] = buf_r[i] = 0;
    }
    Voice_pool_reset(pool);
    v1 = Voice_pool_get_voice(pool, NULL, 0);
    fail_if(v1 == NULL,
            "Voice_pool_get_voice() returned NULL unexpectedly.");
    uint64_t id1 = Voice_id(v1);
    Voice_init(v1, Instrument_get_gen(ins, 0), &ch_state, &ch_state, 64, 120);
    note = 0;
    mod = -1;
    octave = KQT_SCALE_MIDDLE_OCTAVE - 1;
    Event_set_field(ev1_on, 0, &note);
    Event_set_field(ev1_on, 1, &mod);
    Event_set_field(ev1_on, 2, &octave);
    fail_unless(Voice_add_event(v1, ev1_on, 0),
            "Voice_add_event() failed.");
    v2 = Voice_pool_get_voice(pool, NULL, 0);
    fail_if(v2 == NULL,
            "Voice_pool_get_voice() returned NULL unexpectedly.");
    fail_if(v1 == v2,
            "Voice_pool_get_voice() returned an active Voice when inactive Voice was available.");
    Voice_init(v2, Instrument_get_gen(ins, 0), &ch_state, &ch_state, 64, 120);
    note = 0;
    mod = -1;
    octave = KQT_SCALE_MIDDLE_OCTAVE;
    Event_set_field(ev2_on, 0, &note);
    Event_set_field(ev2_on, 1, &mod);
    Event_set_field(ev2_on, 2, &octave);
    fail_unless(Voice_add_event(v2, ev2_on, 0),
            "Voice_add_event() failed.");
    fail_unless(Voice_add_event(v2, ev2_off, 20),
            "Voice_add_event() failed.");
    Voice_pool_mix(pool, 22, 0, 8, 120);
    Voice* v3 = Voice_pool_get_voice(pool, NULL, 0);
    fail_if(v3 == NULL,
            "Voice_pool_get_voice() returned NULL unexpectedly.");
    Voice_init(v3, Instrument_get_gen(ins, 0), &ch_state, &ch_state, 64, 120);
    fail_if(v1 == v3,
            "A higher-priority Voice was killed.");
    fail_unless(Voice_add_event(v3, ev2_on, 22),
            "Voice_add_event() failed.");
    fail_unless(Voice_pool_get_voice(pool, v1, id1) == v1,
            "A higher-priority Voice was killed.");
    Voice_pool_mix(pool, 128, 22, 8, 120);
    for (int i = 0; i < 20; ++i)
    {
        if (i % 8 == 0)
        {
            fail_unless(bufs[0][i] > 1.99 && bufs[0][i] < 2.01,
                    "Buffer contains %f at index %d (expected 2).", bufs[0][i], i);
        }
        else if (i % 4 == 0)
        {
            fail_unless(bufs[0][i] > 1.49 && bufs[0][i] < 1.51,
                    "Buffer contains %f at index %d (expected 1.5).", bufs[0][i], i);
        }
        else
        {
            fail_unless(bufs[0][i] > 0.99 && bufs[0][i] < 1.01,
                    "Buffer contains %f at index %d (expected 1).", bufs[0][i], i);
        }
    }
    fail_unless(bufs[0][20] < -0.49 && bufs[0][20] > -0.51,
            "Buffer contains %f at index %d (expected -0.5).", bufs[0][20], 20);
    fail_unless(fabs(bufs[0][21]) < 0.01,
            "Buffer contains %f at index %d (expected 0).", bufs[0][21], 21);
    for (int i = 22; i < 62; ++i)
    {
        if (i % 8 == 0 || i % 8 == 2 || i % 8 == 6)
        {
            fail_unless(bufs[0][i] > 1.49 && bufs[0][i] < 1.51,
                    "Buffer contains %f at index %d (expected 1.5).", bufs[0][i], i);
        }
        else
        {
            fail_unless(bufs[0][i] > 0.99 && bufs[0][i] < 1.01,
                    "Buffer contains %f at index %d (expected 1).", bufs[0][i], i);
        }
    }
    for (int i = 62; i < 80; ++i)
    {
        if (i % 8 == 0)
        {
            fail_unless(bufs[0][i] > 0.99 && bufs[0][i] < 1.01,
                    "Buffer contains %f at index %d (expected 1).", bufs[0][i], i);
        }
        else
        {
            fail_unless(bufs[0][i] > 0.49 && bufs[0][i] < 0.51,
                    "Buffer contains %f at index %d (expected 0.5).", bufs[0][i], i);
        }
    }
    for (int i = 80; i < 128; ++i)
    {
        fail_unless(fabs(bufs[0][i]) < 0.01,
                "Buffer contains %f at index %d (expected 0).", bufs[0][i], i);
    }
    
    del_Event(ev1_on);
    del_Event(ev1_off);
    del_Event(ev2_on);
    del_Event(ev2_off);
    del_Scale(notes);
    del_Instrument(ins);
    del_Voice_pool(pool);
}
END_TEST

#ifndef NDEBUG
START_TEST (mix_break_pool_null)
{
    Voice_pool_mix(NULL, 0, 0, 1, 120);
}
END_TEST

START_TEST (mix_break_freq_inv)
{
    Voice_pool* pool = new_Voice_pool(2, 1);
    if (pool == NULL)
    {
        fprintf(stderr, "new_Voice_pool() returned NULL -- out of memory?\n");
        return;
    }
    Voice_pool_mix(pool, 0, 0, 0, 120);
    del_Voice_pool(pool);
}
END_TEST
#endif
#endif


Suite* Voice_pool_suite(void)
{
    Suite* s = suite_create("Voice_pool");
    TCase* tc_new = tcase_create("new");
    TCase* tc_get_voice = tcase_create("get_voice");
//    TCase* tc_mix = tcase_create("mix");
    suite_add_tcase(s, tc_new);
    suite_add_tcase(s, tc_get_voice);
//    suite_add_tcase(s, tc_mix);

    int timeout = 10;
    tcase_set_timeout(tc_new, timeout);
    tcase_set_timeout(tc_get_voice, timeout);
//    tcase_set_timeout(tc_mix, timeout);

    tcase_add_test(tc_new, new);
    tcase_add_test(tc_get_voice, get_voice);
//    tcase_add_test(tc_mix, mix);

#ifndef NDEBUG
    tcase_add_test_raise_signal(tc_new, new_break_size_inv, SIGABRT);
    tcase_add_test_raise_signal(tc_new, new_break_events_inv, SIGABRT);

    tcase_add_test_raise_signal(tc_get_voice, get_voice_break_pool_null, SIGABRT);

//    tcase_add_test_raise_signal(tc_mix, mix_break_pool_null, SIGABRT);
//    tcase_add_test_raise_signal(tc_mix, mix_break_freq_inv, SIGABRT);
#endif

    return s;
}


int main(void)
{
    int fail_count = 0;
    Suite* s = Voice_pool_suite();
    SRunner* sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    fail_count = srunner_ntests_failed(sr);
    srunner_free(sr);
    if (fail_count > 0)
    {
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

