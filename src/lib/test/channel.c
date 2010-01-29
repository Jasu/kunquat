

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
#include <Event_queue.h>
#include <Event_voice_note_on.h>
#include <Event_voice_note_off.h>
#include <Generator_debug.h>
#include <Instrument.h>
#include <Voice.h>
#include <Voice_pool.h>
#include <Column.h>
#include <Channel.h>


Suite* Channel_suite(void);


START_TEST (new)
{
    Ins_table* table = new_Ins_table(1);
    if (table == NULL)
    {
        fprintf(stderr, "new_Ins_table() returned NULL -- out of memory?\n");
        abort();
    }
    Event_queue* ins_events = new_Event_queue(16);
    if (ins_events == NULL)
    {
        fprintf(stderr, "new_Event_queue() returned NULL -- out of memory?\n");
        abort();
    }
    Channel* ch = new_Channel(table, 0, ins_events);
    if (ch == NULL)
    {
        fprintf(stderr, "new_Channel() returned NULL -- out of memory?\n");
        abort();
    }
    del_Channel(ch);
    del_Ins_table(table);
}
END_TEST

#ifndef NDEBUG
START_TEST (new_break_insts_null)
{
    new_Channel(NULL, 0, NULL);
}
END_TEST
#endif


START_TEST (set_voices)
{
    kqt_frame buf_l[128] = { 0 };
    kqt_frame buf_r[128] = { 0 };
    kqt_frame* bufs[2] = { buf_l, buf_r };
    kqt_frame* vbufs[2] = { buf_l, buf_r };
    Scale* scales[KQT_SCALES_MAX] = { NULL };
    Scale** default_scale = &scales[0];
    Instrument* ins = new_Instrument(bufs, vbufs, vbufs, 2, 128, scales, &default_scale, 16);
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
    scales[0] = notes;
    Scale_set_note(notes, 0, Real_init(REAL_AUTO));
    Instrument_set_scale(ins, 0);
    Ins_table* table = new_Ins_table(1);
    if (table == NULL)
    {
        fprintf(stderr, "new_Ins_table() returned NULL -- out of memory?\n");
        abort();
    }
    if (!Ins_table_set(table, 1, ins))
    {
        fprintf(stderr, "Ins_table_set() returned false -- out of memory?\n");
        abort();
    }
    Event_queue* ins_events = new_Event_queue(16);
    if (ins_events == NULL)
    {
        fprintf(stderr, "new_Event_queue() returned NULL -- out of memory?\n");
        abort();
    }
    Channel* ch = new_Channel(table, 0, ins_events);
    if (ch == NULL)
    {
        fprintf(stderr, "new_Channel() returned NULL -- out of memory?\n");
        abort();
    }
    Voice_pool* pool = new_Voice_pool(2, 2);
    if (pool == NULL)
    {
        fprintf(stderr, "new_Voice_pool() returned NULL -- out of memory?\n");
        abort();
    }
    Column* col = new_Column(NULL);
    if (col == NULL)
    {
        fprintf(stderr, "new_Column() returned NULL -- out of memory?\n");
        abort();
    }
    Column_iter* citer = new_Column_iter(col);
    if (citer == NULL)
    {
        fprintf(stderr, "new_Column_iter() returned NULL -- out of memory?\n");
        abort();
    }
    Event* ev1_on = (Event*)new_Event_voice_note_on(Reltime_init(RELTIME_AUTO));
    if (ev1_on == NULL)
    {
        fprintf(stderr, "new_Event() returned NULL -- out of memory?\n");
        abort();
    }
    Event* ev1_off = (Event*)new_Event_voice_note_off(Reltime_init(RELTIME_AUTO));
    if (ev1_off == NULL)
    {
        fprintf(stderr, "new_Event() returned NULL -- out of memory?\n");
        abort();
    }
    Event* ev2_on = (Event*)new_Event_voice_note_on(Reltime_init(RELTIME_AUTO));
    if (ev2_on == NULL)
    {
        fprintf(stderr, "new_Event() returned NULL -- out of memory?\n");
        abort();
    }
    Event* ev2_off = (Event*)new_Event_voice_note_off(Reltime_init(RELTIME_AUTO));
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
    int64_t instrument = 1;
    Event_set_field(ev1_on, 0, &note);
    Event_set_field(ev1_on, 1, &mod);
    Event_set_field(ev1_on, 2, &octave);
    ch->cur_inst = instrument;
    if (!Column_ins(col, ev1_on))
    {
        fprintf(stderr, "Column_ins() returned false -- out of memory?\n");
        abort();
    }
    Column_iter_change_col(citer, col);
    Channel_set_voices(ch,
            pool,
            citer,
            Reltime_init(RELTIME_AUTO),
            Reltime_set(RELTIME_AUTO, 10, 0),
            0,
            60,
            8);
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
    // Test again in small parts
    for (int i = 0; i < 128; ++i)
    {
        buf_l[i] = buf_r[i] = 0;
    }
    Channel_reset(ch);
    Voice_pool_reset(pool);
    for (int i = 0; i < 128; ++i)
    {
        Channel_set_voices(ch,
                pool,
                citer,
                Reltime_set(RELTIME_AUTO, i / 8, (KQT_RELTIME_BEAT / 8) * (i % 8)),
                Reltime_set(RELTIME_AUTO, (i + i) / 8, (KQT_RELTIME_BEAT / 8) * ((i + 1) % 8)),
                i,
                60,
                8);
        Voice_pool_mix(pool, i + 1, i, 8, 120);
    }
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
    // Note #1 starts at the beginning and is released at frame 2
    // Note #2 frequency is 2 Hz (2 cycles/beat).
    // Note #2 starts at position 0:(1/4) (frame 2) and plays until the end
    // Result should be as described in the code below.
    for (int i = 0; i < 128; ++i)
    {
        buf_l[i] = buf_r[i] = 0;
    }
    Channel_reset(ch);
    Voice_pool_reset(pool);
    note = 0;
    mod = -1;
    octave = KQT_SCALE_MIDDLE_OCTAVE - 1;
    instrument = 1;
    Event_set_field(ev1_on, 0, &note);
    Event_set_field(ev1_on, 1, &mod);
    Event_set_field(ev1_on, 2, &octave);
    Event_set_field(ev1_on, 3, &instrument);
    octave = KQT_SCALE_MIDDLE_OCTAVE;
    Event_set_field(ev2_on, 0, &note);
    Event_set_field(ev2_on, 1, &mod);
    Event_set_field(ev2_on, 2, &octave);
    Event_set_field(ev2_on, 3, &instrument);
    Event_set_pos(ev2_on, Reltime_set(RELTIME_AUTO, 0, KQT_RELTIME_BEAT / 4));
    if (!Column_ins(col, ev2_on))
    {
        fprintf(stderr, "Column_ins() returned false -- out of memory?\n");
        abort();
    }
    Channel_set_voices(ch,
            pool,
            citer,
            Reltime_init(RELTIME_AUTO),
            Reltime_set(RELTIME_AUTO, 10, 0),
            0,
            60,
            8);
    Voice_pool_mix(pool, 128, 0, 8, 120);
    fail_unless(bufs[0][0] > 0.99 && bufs[0][0] < 1.01,
            "Buffer contains %f at index %d (expected 1).", bufs[0][0], 0);
    fail_unless(bufs[0][1] > 0.49 && bufs[0][1] < 0.51,
            "Buffer contains %f at index %d (expected 0.5).", bufs[0][1], 1);
    for (int i = 2; i < 18; ++i)
    {
        if (i % 8 == 0)
        {
            fail_unless(bufs[0][i] < -0.49 && bufs[0][i] > -0.51,
                    "Buffer contains %f at index %d (expected -0.5).", bufs[0][i], i);
        }
        else if (i % 4 == 2)
        {
            fail_unless(bufs[0][i] > 0.49 && bufs[0][i] < 0.51,
                    "Buffer contains %f at index %d (expected 0.5).", bufs[0][i], i);
        }
        else
        {
            fail_unless(fabs(bufs[0][i]) < 0.01,
                    "Buffer contains %f at index %d (expected 0).", bufs[0][i], i);
        }
    }
    for (int i = 18; i < 42; ++i)
    {
        if (i % 4 == 2)
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
    for (int i = 42; i < 128; ++i)
    {
        fail_unless(fabs(bufs[0][i]) < 0.01,
                "Buffer contains %f at index %d (expected 0).", bufs[0][i], i);
    }
    // Test again in small parts
    for (int i = 0; i < 128; ++i)
    {
        buf_l[i] = buf_r[i] = 0;
    }
    Channel_reset(ch);
    Voice_pool_reset(pool);
    for (int i = 0; i < 128; ++i)
    {
        Channel_set_voices(ch,
                pool,
                citer,
                Reltime_set(RELTIME_AUTO, i / 8, (KQT_RELTIME_BEAT / 8) * (i % 8)),
                Reltime_set(RELTIME_AUTO, (i + i) / 8, (KQT_RELTIME_BEAT / 8) * ((i + 1) % 8)),
                i,
                60,
                8);
        Voice_pool_mix(pool, i + 1, i, 8, 120);
    }
    fail_unless(bufs[0][0] > 0.99 && bufs[0][0] < 1.01,
            "Buffer contains %f at index %d (expected 1).", bufs[0][0], 0);
    fail_unless(bufs[0][1] > 0.49 && bufs[0][1] < 0.51,
            "Buffer contains %f at index %d (expected 0.5).", bufs[0][1], 1);
    for (int i = 2; i < 18; ++i)
    {
        if (i % 8 == 0)
        {
            fail_unless(bufs[0][i] < -0.49 && bufs[0][i] > -0.51,
                    "Buffer contains %f at index %d (expected -0.5).", bufs[0][i], i);
        }
        else if (i % 4 == 2)
        {
            fail_unless(bufs[0][i] > 0.49 && bufs[0][i] < 0.51,
                    "Buffer contains %f at index %d (expected 0.5).", bufs[0][i], i);
        }
        else
        {
            fail_unless(fabs(bufs[0][i]) < 0.01,
                    "Buffer contains %f at index %d (expected 0).", bufs[0][i], i);
        }
    }
    for (int i = 18; i < 42; ++i)
    {
        if (i % 4 == 2)
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
    for (int i = 42; i < 128; ++i)
    {
        fail_unless(fabs(bufs[0][i]) < 0.01,
                "Buffer contains %f at index %d (expected 0).", bufs[0][i], i);
    }

    del_Column_iter(citer);
    del_Column(col);
    del_Voice_pool(pool);
    del_Channel(ch);
    del_Ins_table(table);
    del_Scale(notes);
}
END_TEST

#ifndef NDEBUG
START_TEST (set_voices_break_ch_null)
{
    Ins_table* table = new_Ins_table(1);
    if (table == NULL)
    {
        fprintf(stderr, "new_Ins_table() returned NULL -- out of memory?\n");
        return;
    }
    Voice_pool* pool = new_Voice_pool(1, 1);
    if (pool == NULL)
    {
        fprintf(stderr, "new_Voice_pool() returned NULL -- out of memory?\n");
        return;
    }
    Column* col = new_Column(NULL);
    if (col == NULL)
    {
        fprintf(stderr, "new_Column() returned NULL -- out of memory?\n");
        return;
    }
    Column_iter* citer = new_Column_iter(col);
    if (citer == NULL)
    {
        fprintf(stderr, "new_Column_iter() returned NULL -- out of memory?\n");
        abort();
    }
    Channel_set_voices(NULL,
            pool,
            citer,
            Reltime_init(RELTIME_AUTO),
            Reltime_init(RELTIME_AUTO),
            0,
            1,
            1);
    del_Column_iter(citer);
    del_Column(col);
    del_Voice_pool(pool);
    del_Ins_table(table);
}
END_TEST

START_TEST (set_voices_break_pool_null)
{
    Ins_table* table = new_Ins_table(1);
    if (table == NULL)
    {
        fprintf(stderr, "new_Ins_table() returned NULL -- out of memory?\n");
        return;
    }
    Event_queue* ins_events = new_Event_queue(16);
    if (ins_events == NULL)
    {
        fprintf(stderr, "new_Event_queue() returned NULL -- out of memory?\n");
        abort();
    }
    Channel* ch = new_Channel(table, 0, ins_events);
    if (ch == NULL)
    {
        fprintf(stderr, "new_Channel() returned NULL -- out of memory?\n");
        return;
    }
    Column* col = new_Column(NULL);
    if (col == NULL)
    {
        fprintf(stderr, "new_Column() returned NULL -- out of memory?\n");
        return;
    }
    Column_iter* citer = new_Column_iter(col);
    if (citer == NULL)
    {
        fprintf(stderr, "new_Column_iter() returned NULL -- out of memory?\n");
        abort();
    }
    Channel_set_voices(ch,
            NULL,
            citer,
            Reltime_init(RELTIME_AUTO),
            Reltime_init(RELTIME_AUTO),
            0,
            1,
            1);
    del_Column_iter(citer);
    del_Column(col);
    del_Channel(ch);
    del_Ins_table(table);
}
END_TEST

// This test doesn't apply anymore
#if 0
START_TEST (set_voices_break_col_null)
{
    Ins_table* table = new_Ins_table(1);
    if (table == NULL)
    {
        fprintf(stderr, "new_Ins_table() returned NULL -- out of memory?\n");
        return;
    }
    Channel* ch = new_Channel(table);
    if (ch == NULL)
    {
        fprintf(stderr, "new_Channel() returned NULL -- out of memory?\n");
        return;
    }
    Voice_pool* pool = new_Voice_pool(1, 1);
    if (pool == NULL)
    {
        fprintf(stderr, "new_Voice_pool() returned NULL -- out of memory?\n");
        return;
    }
    Channel_set_voices(ch,
            pool,
            NULL,
            Reltime_init(RELTIME_AUTO),
            Reltime_init(RELTIME_AUTO),
            0,
            1,
            1);
    del_Voice_pool(pool);
    del_Channel(ch);
    del_Ins_table(table);
}
END_TEST
#endif

START_TEST (set_voices_break_start_null)
{
    Ins_table* table = new_Ins_table(1);
    if (table == NULL)
    {
        fprintf(stderr, "new_Ins_table() returned NULL -- out of memory?\n");
        return;
    }
    Event_queue* ins_events = new_Event_queue(16);
    if (ins_events == NULL)
    {
        fprintf(stderr, "new_Event_queue() returned NULL -- out of memory?\n");
        abort();
    }
    Channel* ch = new_Channel(table, 0, ins_events);
    if (ch == NULL)
    {
        fprintf(stderr, "new_Channel() returned NULL -- out of memory?\n");
        return;
    }
    Voice_pool* pool = new_Voice_pool(1, 1);
    if (pool == NULL)
    {
        fprintf(stderr, "new_Voice_pool() returned NULL -- out of memory?\n");
        return;
    }
    Column* col = new_Column(NULL);
    if (col == NULL)
    {
        fprintf(stderr, "new_Column() returned NULL -- out of memory?\n");
        return;
    }
    Column_iter* citer = new_Column_iter(col);
    if (citer == NULL)
    {
        fprintf(stderr, "new_Column_iter() returned NULL -- out of memory?\n");
        abort();
    }
    Channel_set_voices(ch,
            pool,
            citer,
            NULL,
            Reltime_init(RELTIME_AUTO),
            0,
            1,
            1);
    del_Column_iter(citer);
    del_Column(col);
    del_Voice_pool(pool);
    del_Channel(ch);
    del_Ins_table(table);
}
END_TEST

START_TEST (set_voices_break_end_null)
{
    Ins_table* table = new_Ins_table(1);
    if (table == NULL)
    {
        fprintf(stderr, "new_Ins_table() returned NULL -- out of memory?\n");
        return;
    }
    Event_queue* ins_events = new_Event_queue(16);
    if (ins_events == NULL)
    {
        fprintf(stderr, "new_Event_queue() returned NULL -- out of memory?\n");
        abort();
    }
    Channel* ch = new_Channel(table, 0, ins_events);
    if (ch == NULL)
    {
        fprintf(stderr, "new_Channel() returned NULL -- out of memory?\n");
        return;
    }
    Voice_pool* pool = new_Voice_pool(1, 1);
    if (pool == NULL)
    {
        fprintf(stderr, "new_Voice_pool() returned NULL -- out of memory?\n");
        return;
    }
    Column* col = new_Column(NULL);
    if (col == NULL)
    {
        fprintf(stderr, "new_Column() returned NULL -- out of memory?\n");
        return;
    }
    Column_iter* citer = new_Column_iter(col);
    if (citer == NULL)
    {
        fprintf(stderr, "new_Column_iter() returned NULL -- out of memory?\n");
        abort();
    }
    Channel_set_voices(ch,
            pool,
            citer,
            Reltime_init(RELTIME_AUTO),
            NULL,
            0,
            1,
            1);
    del_Column_iter(citer);
    del_Column(col);
    del_Voice_pool(pool);
    del_Channel(ch);
    del_Ins_table(table);
}
END_TEST

START_TEST (set_voices_break_tempo_inv)
{
    Ins_table* table = new_Ins_table(1);
    if (table == NULL)
    {
        fprintf(stderr, "new_Ins_table() returned NULL -- out of memory?\n");
        return;
    }
    Event_queue* ins_events = new_Event_queue(16);
    if (ins_events == NULL)
    {
        fprintf(stderr, "new_Event_queue() returned NULL -- out of memory?\n");
        abort();
    }
    Channel* ch = new_Channel(table, 0, ins_events);
    if (ch == NULL)
    {
        fprintf(stderr, "new_Channel() returned NULL -- out of memory?\n");
        return;
    }
    Voice_pool* pool = new_Voice_pool(1, 1);
    if (pool == NULL)
    {
        fprintf(stderr, "new_Voice_pool() returned NULL -- out of memory?\n");
        return;
    }
    Column* col = new_Column(NULL);
    if (col == NULL)
    {
        fprintf(stderr, "new_Column() returned NULL -- out of memory?\n");
        return;
    }
    Column_iter* citer = new_Column_iter(col);
    if (citer == NULL)
    {
        fprintf(stderr, "new_Column_iter() returned NULL -- out of memory?\n");
        abort();
    }
    Channel_set_voices(ch,
            pool,
            citer,
            Reltime_init(RELTIME_AUTO),
            Reltime_init(RELTIME_AUTO),
            0,
            0,
            1);
    del_Column_iter(citer);
    del_Column(col);
    del_Voice_pool(pool);
    del_Channel(ch);
    del_Ins_table(table);
}
END_TEST

START_TEST (set_voices_break_freq_inv)
{
    Ins_table* table = new_Ins_table(1);
    if (table == NULL)
    {
        fprintf(stderr, "new_Ins_table() returned NULL -- out of memory?\n");
        return;
    }
    Event_queue* ins_events = new_Event_queue(16);
    if (ins_events == NULL)
    {
        fprintf(stderr, "new_Event_queue() returned NULL -- out of memory?\n");
        abort();
    }
    Channel* ch = new_Channel(table, 0, ins_events);
    if (ch == NULL)
    {
        fprintf(stderr, "new_Channel() returned NULL -- out of memory?\n");
        return;
    }
    Voice_pool* pool = new_Voice_pool(1, 1);
    if (pool == NULL)
    {
        fprintf(stderr, "new_Voice_pool() returned NULL -- out of memory?\n");
        return;
    }
    Column* col = new_Column(NULL);
    if (col == NULL)
    {
        fprintf(stderr, "new_Column() returned NULL -- out of memory?\n");
        return;
    }
    Column_iter* citer = new_Column_iter(col);
    if (citer == NULL)
    {
        fprintf(stderr, "new_Column_iter() returned NULL -- out of memory?\n");
        abort();
    }
    Channel_set_voices(ch,
            pool,
            citer,
            Reltime_init(RELTIME_AUTO),
            Reltime_init(RELTIME_AUTO),
            0,
            1,
            0);
    del_Column_iter(citer);
    del_Column(col);
    del_Voice_pool(pool);
    del_Channel(ch);
    del_Ins_table(table);
}
END_TEST
#endif


Suite* Channel_suite(void)
{
    Suite* s = suite_create("Channel");
    TCase* tc_new = tcase_create("new");
    TCase* tc_set_voices = tcase_create("set_voices");
    suite_add_tcase(s, tc_new);
    suite_add_tcase(s, tc_set_voices);

    int timeout = 10;
    tcase_set_timeout(tc_new, timeout);
    tcase_set_timeout(tc_set_voices, timeout);

    tcase_add_test(tc_new, new);
    tcase_add_test(tc_set_voices, set_voices);

#ifndef NDEBUG
    tcase_add_test_raise_signal(tc_new, new_break_insts_null, SIGABRT);

    tcase_add_test_raise_signal(tc_set_voices, set_voices_break_ch_null, SIGABRT);
    tcase_add_test_raise_signal(tc_set_voices, set_voices_break_pool_null, SIGABRT);
//  tcase_add_test_raise_signal(tc_set_voices, set_voices_break_col_null, SIGABRT);
    tcase_add_test_raise_signal(tc_set_voices, set_voices_break_start_null, SIGABRT);
    tcase_add_test_raise_signal(tc_set_voices, set_voices_break_end_null, SIGABRT);
    tcase_add_test_raise_signal(tc_set_voices, set_voices_break_tempo_inv, SIGABRT);
    tcase_add_test_raise_signal(tc_set_voices, set_voices_break_freq_inv, SIGABRT);
#endif

    return s;
}


int main(void)
{
    int fail_count = 0;
    Suite* s = Channel_suite();
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

