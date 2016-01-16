

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef EVENT_CHANNEL_DEF
#error "EVENT_CHANNEL_DEF(..) not defined"
#endif


//                Name      Type suffix                 Arg type    Validator
EVENT_CHANNEL_DEF(".a",     set_au_input,               INT,        v_au)

EVENT_CHANNEL_DEF("n+",     note_on,                    FLOAT,      v_pitch)
EVENT_CHANNEL_DEF("h",      hit,                        INT,        v_hit)
EVENT_CHANNEL_DEF("n-",     note_off,                   NONE,       NULL)

EVENT_CHANNEL_DEF(".f",     set_force,                  FLOAT,      v_force)
EVENT_CHANNEL_DEF("/f",     slide_force,                FLOAT,      v_force)
EVENT_CHANNEL_DEF("/=f",    slide_force_length,         TSTAMP,     v_nonneg_ts)
EVENT_CHANNEL_DEF("ts",     tremolo_speed,              FLOAT,      v_nonneg_float)
EVENT_CHANNEL_DEF("td",     tremolo_depth,              FLOAT,      v_tremolo_depth)
EVENT_CHANNEL_DEF("t/=s",   tremolo_speed_slide,        TSTAMP,     v_nonneg_ts)
EVENT_CHANNEL_DEF("t/=d",   tremolo_depth_slide,        TSTAMP,     v_nonneg_ts)
EVENT_CHANNEL_DEF("->f+",   carry_force_on,             NONE,       NULL)
EVENT_CHANNEL_DEF("->f-",   carry_force_off,            NONE,       NULL)

EVENT_CHANNEL_DEF("/p",     slide_pitch,                FLOAT,      v_pitch)
EVENT_CHANNEL_DEF("/=p",    slide_pitch_length,         TSTAMP,     v_nonneg_ts)
EVENT_CHANNEL_DEF("vs",     vibrato_speed,              FLOAT,      v_nonneg_float)
EVENT_CHANNEL_DEF("vd",     vibrato_depth,              FLOAT,      v_nonneg_float)
EVENT_CHANNEL_DEF("v/=s",   vibrato_speed_slide,        TSTAMP,     v_nonneg_ts)
EVENT_CHANNEL_DEF("v/=d",   vibrato_depth_slide,        TSTAMP,     v_nonneg_ts)
EVENT_CHANNEL_DEF("->p+",   carry_pitch_on,             NONE,       NULL)
EVENT_CHANNEL_DEF("->p-",   carry_pitch_off,            NONE,       NULL)

EVENT_CHANNEL_DEF("<arp",   reset_arpeggio,             NONE,       NULL)
EVENT_CHANNEL_DEF(".arpn",  set_arpeggio_note,          FLOAT,      v_pitch)
EVENT_CHANNEL_DEF(".arpi",  set_arpeggio_index,         INT,        v_arp_index)
EVENT_CHANNEL_DEF(".arps",  set_arpeggio_speed,         FLOAT,      v_arp_speed)
EVENT_CHANNEL_DEF("arp+",   arpeggio_on,                NONE,       NULL)
EVENT_CHANNEL_DEF("arp-",   arpeggio_off,               NONE,       NULL)

EVENT_CHANNEL_DEF(".sn",    set_stream_name,            STRING,     v_var_name)

EVENT_CHANNEL_DEF(".vn",    set_cv_name,                STRING,     v_var_name)
EVENT_CHANNEL_DEF(".v",     set_cv_value,               REALTIME,   v_finite_rt)
EVENT_CHANNEL_DEF("/v",     slide_cv_target,            FLOAT,      v_finite_float)
EVENT_CHANNEL_DEF("/=v",    slide_cv_length,            TSTAMP,     v_nonneg_ts)
EVENT_CHANNEL_DEF("osv",    osc_speed_cv,               FLOAT,      v_nonneg_float)
EVENT_CHANNEL_DEF("odv",    osc_depth_cv,               FLOAT,      v_nonneg_float)
EVENT_CHANNEL_DEF("o/=sv",  osc_speed_slide_cv,         TSTAMP,     v_nonneg_ts)
EVENT_CHANNEL_DEF("o/=dv",  osc_depth_slide_cv,         TSTAMP,     v_nonneg_ts)
EVENT_CHANNEL_DEF("->v+",   carry_cv_on,                NONE,       NULL)
EVENT_CHANNEL_DEF("->v-",   carry_cv_off,               NONE,       NULL)


#undef EVENT_CHANNEL_DEF


