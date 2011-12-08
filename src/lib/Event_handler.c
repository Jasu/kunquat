

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2011
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <DSP_conf.h>
#include <Effect.h>
#include <Event_buffer.h>
#include <Event_handler.h>
#include <Event_names.h>
#include <Event_type.h>
#include <File_base.h>
#include <Channel_state.h>
#include <General_state.h>
#include <Generator.h>
#include <Ins_table.h>
#include <Playdata.h>
#include <kunquat/limits.h>

#include <Event_control_pause.h>
#include <Event_control_resume.h>
#include <Event_control_play_pattern.h>

#include <Event_control_env_set_bool_name.h>
#include <Event_control_env_set_bool.h>
#include <Event_control_env_set_int_name.h>
#include <Event_control_env_set_int.h>
#include <Event_control_env_set_float_name.h>
#include <Event_control_env_set_float.h>
#include <Event_control_env_set_timestamp_name.h>
#include <Event_control_env_set_timestamp.h>

#include <Event_control_set_goto_row.h>
#include <Event_control_set_goto_section.h>
#include <Event_control_set_goto_subsong.h>
#include <Event_control_goto.h>

#include <Event_control_turing.h>

#include <Event_control_receive_event.h>

#include <Event_general_comment.h>

#include <Event_general_cond.h>
#include <Event_general_if.h>
#include <Event_general_end_if.h>

#include <Event_general_call_bool.h>
#include <Event_general_call_int.h>
#include <Event_general_call_float.h>

#include <Event_global_pattern_delay.h>
#include <Event_global_set_jump_counter.h>
#include <Event_global_set_jump_row.h>
#include <Event_global_set_jump_section.h>
#include <Event_global_set_jump_subsong.h>
//#include <Event_global_jump.h>

#include <Event_global_set_scale.h>
#include <Event_global_set_scale_offset.h>
#include <Event_global_mimic_scale.h>
#include <Event_global_set_scale_fixed_point.h>
#include <Event_global_shift_scale_intervals.h>

#include <Event_global_set_tempo.h>
#include <Event_global_set_volume.h>
#include <Event_global_slide_tempo.h>
#include <Event_global_slide_tempo_length.h>
#include <Event_global_slide_volume.h>
#include <Event_global_slide_volume_length.h>

#include <Event_channel_set_instrument.h>
#include <Event_channel_set_generator.h>
#include <Event_channel_set_effect.h>
#include <Event_channel_set_global_effects.h>
#include <Event_channel_set_instrument_effects.h>
#include <Event_channel_set_dsp.h>

#include <Event_channel_note_on.h>
#include <Event_channel_hit.h>
#include <Event_channel_note_off.h>

#include <Event_channel_set_force.h>
#include <Event_channel_slide_force.h>
#include <Event_channel_slide_force_length.h>
#include <Event_channel_tremolo_speed.h>
#include <Event_channel_tremolo_depth.h>
#include <Event_channel_tremolo_delay.h>

#include <Event_channel_slide_pitch.h>
#include <Event_channel_slide_pitch_length.h>
#include <Event_channel_vibrato_speed.h>
#include <Event_channel_vibrato_depth.h>
#include <Event_channel_vibrato_delay.h>

#include <Event_channel_reset_arpeggio.h>
#include <Event_channel_set_arpeggio_note.h>
#include <Event_channel_set_arpeggio_index.h>
#include <Event_channel_set_arpeggio_speed.h>
#include <Event_channel_arpeggio_on.h>
#include <Event_channel_arpeggio_off.h>

#include <Event_channel_set_lowpass.h>
#include <Event_channel_slide_lowpass.h>
#include <Event_channel_slide_lowpass_length.h>
#include <Event_channel_autowah_speed.h>
#include <Event_channel_autowah_depth.h>
#include <Event_channel_autowah_delay.h>

#include <Event_channel_set_resonance.h>

#include <Event_channel_set_panning.h>
#include <Event_channel_slide_panning.h>
#include <Event_channel_slide_panning_length.h>

#include <Event_channel_set_gen_bool_name.h>
#include <Event_channel_set_gen_bool.h>
#include <Event_channel_set_gen_int_name.h>
#include <Event_channel_set_gen_int.h>
#include <Event_channel_set_gen_float_name.h>
#include <Event_channel_set_gen_float.h>
#include <Event_channel_set_gen_reltime_name.h>
#include <Event_channel_set_gen_reltime.h>

#include <Event_ins_set_sustain.h>

#include <Event_generator_set_bool_name.h>
#include <Event_generator_set_bool.h>
#include <Event_generator_set_int_name.h>
#include <Event_generator_set_int.h>
#include <Event_generator_set_float_name.h>
#include <Event_generator_set_float.h>
#include <Event_generator_set_reltime_name.h>
#include <Event_generator_set_reltime.h>

#include <Event_effect_bypass_on.h>
#include <Event_effect_bypass_off.h>

#include <Event_dsp_set_bool_name.h>
#include <Event_dsp_set_bool.h>
#include <Event_dsp_set_int_name.h>
#include <Event_dsp_set_int.h>
#include <Event_dsp_set_float_name.h>
#include <Event_dsp_set_float.h>
#include <Event_dsp_set_reltime_name.h>
#include <Event_dsp_set_reltime.h>

#include <xassert.h>
#include <xmemory.h>


struct Event_handler
{
    bool mute; // FIXME: this is just to make the stupid Channel_state_init happy
    Channel_state* ch_states[KQT_COLUMNS_MAX];
    Ins_table* insts;
    Effect_table* effects;
    Playdata* global_state;
    Event_names* event_names;
    Event_buffer* event_buffer;
    bool (*control_process[EVENT_CONTROL_UPPER])(General_state*, char*);
    bool (*general_process[EVENT_GENERAL_UPPER])(General_state*, char*);
    bool (*ch_process[EVENT_CHANNEL_UPPER])(Channel_state*, char*);
    bool (*global_process[EVENT_GLOBAL_UPPER])(Playdata*, char*);
    bool (*ins_process[EVENT_INS_UPPER])(Instrument_params*, char*);
    bool (*generator_process[EVENT_GENERATOR_UPPER])(Generator*,
                                                     Channel_state*, char*);
    bool (*effect_process[EVENT_EFFECT_UPPER])(Effect*, char*);
    bool (*dsp_process[EVENT_DSP_UPPER])(DSP_conf*, Channel_state*, char*);
};


static bool Event_handler_handle(Event_handler* eh,
                                 int index,
                                 Event_type type,
                                 char* fields);


Event_handler* new_Event_handler(Playdata* global_state,
                                 Channel_state** ch_states,
                                 Ins_table* insts,
                                 Effect_table* effects)
{
    assert(global_state != NULL);
    assert(ch_states != NULL);
    assert(insts != NULL);
    assert(effects != NULL);
    Event_handler* eh = xalloc(Event_handler);
    if (eh == NULL)
    {
        return NULL;
    }
    eh->event_buffer = NULL;
    eh->event_names = new_Event_names();
    if (eh->event_names == NULL)
    {
        del_Event_handler(eh);
        return NULL;
    }
    eh->event_buffer = new_Event_buffer(16384);
    if (eh->event_buffer == NULL)
    {
        del_Event_handler(eh);
        return NULL;
    }
    eh->global_state = global_state;
/*    if (eh->global_state == NULL)
    {
        del_Event_handler(eh);
        return NULL;
    } */
    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        eh->ch_states[i] = ch_states[i];
//        Channel_state_init(&eh->ch_states[i], i, &eh->mute);
    }
    eh->insts = insts;
    eh->effects = effects;

    Event_handler_set_control_process(eh, ">pause", EVENT_CONTROL_PAUSE,
                                      Event_control_pause_process);
    Event_handler_set_control_process(eh, ">resume", EVENT_CONTROL_RESUME,
                                      Event_control_resume_process);
    Event_handler_set_control_process(eh, ">pattern", EVENT_CONTROL_PLAY_PATTERN,
                                      Event_control_play_pattern_process);

    Event_handler_set_control_process(eh, ">.Bn", EVENT_CONTROL_ENV_SET_BOOL_NAME,
                                      Event_control_env_set_bool_name_process);
    Event_handler_set_control_process(eh, ">.B", EVENT_CONTROL_ENV_SET_BOOL,
                                      Event_control_env_set_bool_process);
    Event_handler_set_control_process(eh, ">.In", EVENT_CONTROL_ENV_SET_INT_NAME,
                                      Event_control_env_set_int_name_process);
    Event_handler_set_control_process(eh, ">.I", EVENT_CONTROL_ENV_SET_INT,
                                      Event_control_env_set_int_process);
    Event_handler_set_control_process(eh, ">.Fn", EVENT_CONTROL_ENV_SET_FLOAT_NAME,
                                      Event_control_env_set_float_name_process);
    Event_handler_set_control_process(eh, ">.F", EVENT_CONTROL_ENV_SET_FLOAT,
                                      Event_control_env_set_float_process);
    Event_handler_set_control_process(eh, ">.Tn", EVENT_CONTROL_ENV_SET_TIMESTAMP_NAME,
                                      Event_control_env_set_timestamp_name_process);
    Event_handler_set_control_process(eh, ">.T", EVENT_CONTROL_ENV_SET_TIMESTAMP,
                                      Event_control_env_set_timestamp_process);

    Event_handler_set_control_process(eh, ">.gr", EVENT_CONTROL_SET_GOTO_ROW,
                                      Event_control_set_goto_row_process);
    Event_handler_set_control_process(eh, ">.gs", EVENT_CONTROL_SET_GOTO_SECTION,
                                      Event_control_set_goto_section_process);
    Event_handler_set_control_process(eh, ">.gss", EVENT_CONTROL_SET_GOTO_SUBSONG,
                                      Event_control_set_goto_subsong_process);
    Event_handler_set_control_process(eh, ">g", EVENT_CONTROL_GOTO,
                                      Event_control_goto_process);

    Event_handler_set_control_process(eh, ">Turing", EVENT_CONTROL_TURING,
                                      Event_control_turing_process);

    Event_handler_set_control_process(eh, ">receive", EVENT_CONTROL_RECEIVE_EVENT,
                                      Event_control_receive_event);

    Event_handler_set_general_process(eh, "#", EVENT_GENERAL_COMMENT,
                                      Event_general_comment_process);

    Event_handler_set_general_process(eh, "#?", EVENT_GENERAL_COND,
                                      Event_general_cond_process);
    Event_handler_set_general_process(eh, "#if", EVENT_GENERAL_IF,
                                      Event_general_if_process);
    Event_handler_set_general_process(eh, "#endif", EVENT_GENERAL_END_IF,
                                      Event_general_end_if_process);

    Event_handler_set_general_process(eh, "#signal", EVENT_GENERAL_SIGNAL,
                                      Event_general_comment_process);
    Event_handler_set_general_process(eh, "#callBn", EVENT_GENERAL_CALL_BOOL_NAME,
                                      Event_general_comment_process);
    Event_handler_set_general_process(eh, "#callB", EVENT_GENERAL_CALL_BOOL,
                                      Event_general_call_bool_process);
    Event_handler_set_general_process(eh, "#callIn", EVENT_GENERAL_CALL_INT_NAME,
                                      Event_general_comment_process);
    Event_handler_set_general_process(eh, "#callI", EVENT_GENERAL_CALL_INT,
                                      Event_general_call_int_process);
    Event_handler_set_general_process(eh, "#callFn", EVENT_GENERAL_CALL_FLOAT_NAME,
                                      Event_general_comment_process);
    Event_handler_set_general_process(eh, "#callF", EVENT_GENERAL_CALL_FLOAT,
                                      Event_general_call_float_process);

    Event_handler_set_global_process(eh, "wpd", EVENT_GLOBAL_PATTERN_DELAY,
                                     Event_global_pattern_delay_process);
    Event_handler_set_global_process(eh, "w.jc", EVENT_GLOBAL_SET_JUMP_COUNTER,
                                     Event_global_set_jump_counter_process);
    Event_handler_set_global_process(eh, "w.jr", EVENT_GLOBAL_SET_JUMP_ROW,
                                     Event_global_set_jump_row_process);
    Event_handler_set_global_process(eh, "w.js", EVENT_GLOBAL_SET_JUMP_SECTION,
                                     Event_global_set_jump_section_process);
    Event_handler_set_global_process(eh, "w.jss", EVENT_GLOBAL_SET_JUMP_SUBSONG,
                                     Event_global_set_jump_subsong_process);
    //Event_handler_set_global_process(eh, "wj", EVENT_GLOBAL_JUMP,
    //                                 Event_global_jump_process);

    Event_handler_set_global_process(eh, "w.s", EVENT_GLOBAL_SET_SCALE,
                                     Event_global_set_scale_process);
    Event_handler_set_global_process(eh, "w.so", EVENT_GLOBAL_SET_SCALE_OFFSET,
                                     Event_global_set_scale_offset_process);
    Event_handler_set_global_process(eh, "wms", EVENT_GLOBAL_MIMIC_SCALE,
                                     Event_global_mimic_scale_process);
    Event_handler_set_global_process(eh, "w.sfp", EVENT_GLOBAL_SET_SCALE_FIXED_POINT,
                                     Event_global_set_scale_fixed_point_process);
    Event_handler_set_global_process(eh, "wssi", EVENT_GLOBAL_SHIFT_SCALE_INTERVALS,
                                     Event_global_shift_scale_intervals_process);

    Event_handler_set_global_process(eh, "w.t", EVENT_GLOBAL_SET_TEMPO,
                                     Event_global_set_tempo_process);
    Event_handler_set_global_process(eh, "w.v", EVENT_GLOBAL_SET_VOLUME,
                                     Event_global_set_volume_process);
    Event_handler_set_global_process(eh, "w/t", EVENT_GLOBAL_SLIDE_TEMPO,
                                     Event_global_slide_tempo_process);
    Event_handler_set_global_process(eh, "w/=t", EVENT_GLOBAL_SLIDE_TEMPO_LENGTH,
                                     Event_global_slide_tempo_length_process);
    Event_handler_set_global_process(eh, "w/v", EVENT_GLOBAL_SLIDE_VOLUME,
                                     Event_global_slide_volume_process);
    Event_handler_set_global_process(eh, "w/=v", EVENT_GLOBAL_SLIDE_VOLUME_LENGTH,
                                     Event_global_slide_volume_length_process);

    Event_handler_set_ch_process(eh, "c.i", EVENT_CHANNEL_SET_INSTRUMENT,
                                 Event_channel_set_instrument_process);
    Event_handler_set_ch_process(eh, "c.g", EVENT_CHANNEL_SET_GENERATOR,
                                 Event_channel_set_generator_process);
    Event_handler_set_ch_process(eh, "c.e", EVENT_CHANNEL_SET_EFFECT,
                                 Event_channel_set_effect_process);
    Event_handler_set_ch_process(eh, "c.ge", EVENT_CHANNEL_SET_GLOBAL_EFFECTS,
                                 Event_channel_set_global_effects_process);
    Event_handler_set_ch_process(eh, "c.ie",
                                 EVENT_CHANNEL_SET_INSTRUMENT_EFFECTS,
                                 Event_channel_set_instrument_effects_process);
    Event_handler_set_ch_process(eh, "c.d", EVENT_CHANNEL_SET_DSP,
                                 Event_channel_set_dsp_process);

    Event_handler_set_ch_process(eh, "cn+", EVENT_CHANNEL_NOTE_ON,
                                 Event_channel_note_on_process);
    Event_handler_set_ch_process(eh, "ch", EVENT_CHANNEL_HIT,
                                 Event_channel_hit_process);
    Event_handler_set_ch_process(eh, "cn-", EVENT_CHANNEL_NOTE_OFF,
                                 Event_channel_note_off_process);

    Event_handler_set_ch_process(eh, "c.f", EVENT_CHANNEL_SET_FORCE,
                                 Event_channel_set_force_process);
    Event_handler_set_ch_process(eh, "c/f", EVENT_CHANNEL_SLIDE_FORCE,
                                 Event_channel_slide_force_process);
    Event_handler_set_ch_process(eh, "c/=f", EVENT_CHANNEL_SLIDE_FORCE_LENGTH,
                                 Event_channel_slide_force_length_process);
    Event_handler_set_ch_process(eh, "cTs", EVENT_CHANNEL_TREMOLO_SPEED,
                                 Event_channel_tremolo_speed_process);
    Event_handler_set_ch_process(eh, "cTd", EVENT_CHANNEL_TREMOLO_DEPTH,
                                 Event_channel_tremolo_depth_process);
    Event_handler_set_ch_process(eh, "cTdd", EVENT_CHANNEL_TREMOLO_DELAY,
                                 Event_channel_tremolo_delay_process);

    Event_handler_set_ch_process(eh, "c/p", EVENT_CHANNEL_SLIDE_PITCH,
                                 Event_channel_slide_pitch_process);
    Event_handler_set_ch_process(eh, "c/=p", EVENT_CHANNEL_SLIDE_PITCH_LENGTH,
                                 Event_channel_slide_pitch_length_process);
    Event_handler_set_ch_process(eh, "cVs", EVENT_CHANNEL_VIBRATO_SPEED,
                                 Event_channel_vibrato_speed_process);
    Event_handler_set_ch_process(eh, "cVd", EVENT_CHANNEL_VIBRATO_DEPTH,
                                 Event_channel_vibrato_depth_process);
    Event_handler_set_ch_process(eh, "cVdd", EVENT_CHANNEL_VIBRATO_DELAY,
                                 Event_channel_vibrato_delay_process);

    Event_handler_set_ch_process(eh, "c<Arp", EVENT_CHANNEL_RESET_ARPEGGIO,
                                 Event_channel_reset_arpeggio_process);
    Event_handler_set_ch_process(eh, "c.Arpn", EVENT_CHANNEL_SET_ARPEGGIO_NOTE,
                                 Event_channel_set_arpeggio_note_process);
    Event_handler_set_ch_process(eh, "c.Arpi", EVENT_CHANNEL_SET_ARPEGGIO_INDEX,
                                 Event_channel_set_arpeggio_index_process);
    Event_handler_set_ch_process(eh, "c.Arps", EVENT_CHANNEL_SET_ARPEGGIO_SPEED,
                                 Event_channel_set_arpeggio_speed_process);
    Event_handler_set_ch_process(eh, "cArp+", EVENT_CHANNEL_ARPEGGIO_ON,
                                 Event_channel_arpeggio_on_process);
    Event_handler_set_ch_process(eh, "cArp-", EVENT_CHANNEL_ARPEGGIO_OFF,
                                 Event_channel_arpeggio_off_process);

    Event_handler_set_ch_process(eh, "c.l", EVENT_CHANNEL_SET_LOWPASS,
                                 Event_channel_set_lowpass_process);
    Event_handler_set_ch_process(eh, "c/l", EVENT_CHANNEL_SLIDE_LOWPASS,
                                 Event_channel_slide_lowpass_process);
    Event_handler_set_ch_process(eh, "c/=l", EVENT_CHANNEL_SLIDE_LOWPASS_LENGTH,
                                 Event_channel_slide_lowpass_length_process);
    Event_handler_set_ch_process(eh, "cAs", EVENT_CHANNEL_AUTOWAH_SPEED,
                                 Event_channel_autowah_speed_process);
    Event_handler_set_ch_process(eh, "cAd", EVENT_CHANNEL_AUTOWAH_DEPTH,
                                 Event_channel_autowah_depth_process);
    Event_handler_set_ch_process(eh, "cAdd", EVENT_CHANNEL_AUTOWAH_DELAY,
                                 Event_channel_autowah_delay_process);

    Event_handler_set_ch_process(eh, "c.r", EVENT_CHANNEL_SET_RESONANCE,
                                 Event_channel_set_resonance_process);

    Event_handler_set_ch_process(eh, "c.P", EVENT_CHANNEL_SET_PANNING,
                                 Event_channel_set_panning_process);
    Event_handler_set_ch_process(eh, "c/P", EVENT_CHANNEL_SLIDE_PANNING,
                                 Event_channel_slide_panning_process);
    Event_handler_set_ch_process(eh, "c/=P", EVENT_CHANNEL_SLIDE_PANNING_LENGTH,
                                 Event_channel_slide_panning_length_process);

    Event_handler_set_ch_process(eh, "c.gBn", EVENT_CHANNEL_SET_GEN_BOOL_NAME,
                                 Event_channel_set_gen_bool_name_process);
    Event_handler_set_ch_process(eh, "c.gB", EVENT_CHANNEL_SET_GEN_BOOL,
                                 Event_channel_set_gen_bool_process);
    Event_handler_set_ch_process(eh, "c.gIn", EVENT_CHANNEL_SET_GEN_INT_NAME,
                                 Event_channel_set_gen_int_name_process);
    Event_handler_set_ch_process(eh, "c.gI", EVENT_CHANNEL_SET_GEN_INT,
                                 Event_channel_set_gen_int_process);
    Event_handler_set_ch_process(eh, "c.gFn", EVENT_CHANNEL_SET_GEN_FLOAT_NAME,
                                 Event_channel_set_gen_float_name_process);
    Event_handler_set_ch_process(eh, "c.gF", EVENT_CHANNEL_SET_GEN_FLOAT,
                                 Event_channel_set_gen_float_process);
    Event_handler_set_ch_process(eh, "c.gTn", EVENT_CHANNEL_SET_GEN_RELTIME_NAME,
                                 Event_channel_set_gen_reltime_name_process);
    Event_handler_set_ch_process(eh, "c.gT", EVENT_CHANNEL_SET_GEN_RELTIME,
                                 Event_channel_set_gen_reltime_process);

    Event_handler_set_ins_process(eh, "i.sus", EVENT_INS_SET_SUSTAIN,
                                  Event_ins_set_sustain_process);

    Event_handler_set_generator_process(eh, "g.Bn", EVENT_GENERATOR_SET_BOOL_NAME,
                                        Event_generator_set_bool_name_process);
    Event_handler_set_generator_process(eh, "g.B", EVENT_GENERATOR_SET_BOOL,
                                        Event_generator_set_bool_process);
    Event_handler_set_generator_process(eh, "g.In", EVENT_GENERATOR_SET_INT_NAME,
                                        Event_generator_set_int_name_process);
    Event_handler_set_generator_process(eh, "g.I", EVENT_GENERATOR_SET_INT,
                                        Event_generator_set_int_process);
    Event_handler_set_generator_process(eh, "g.Fn", EVENT_GENERATOR_SET_FLOAT_NAME,
                                        Event_generator_set_float_name_process);
    Event_handler_set_generator_process(eh, "g.F", EVENT_GENERATOR_SET_FLOAT,
                                        Event_generator_set_float_process);
    Event_handler_set_generator_process(eh, "g.Tn", EVENT_GENERATOR_SET_RELTIME_NAME,
                                        Event_generator_set_reltime_name_process);
    Event_handler_set_generator_process(eh, "g.T", EVENT_GENERATOR_SET_RELTIME,
                                        Event_generator_set_reltime_process);

    Event_handler_set_effect_process(eh, "ebp+", EVENT_EFFECT_BYPASS_ON,
                                     Event_effect_bypass_on_process);
    Event_handler_set_effect_process(eh, "ebp-", EVENT_EFFECT_BYPASS_OFF,
                                     Event_effect_bypass_off_process);

    Event_handler_set_dsp_process(eh, "d.Bn", EVENT_DSP_SET_BOOL_NAME,
                                  Event_dsp_set_bool_name_process);
    Event_handler_set_dsp_process(eh, "d.B", EVENT_DSP_SET_BOOL,
                                  Event_dsp_set_bool_process);
    Event_handler_set_dsp_process(eh, "d.In", EVENT_DSP_SET_INT_NAME,
                                  Event_dsp_set_int_name_process);
    Event_handler_set_dsp_process(eh, "d.I", EVENT_DSP_SET_INT,
                                  Event_dsp_set_int_process);
    Event_handler_set_dsp_process(eh, "d.Fn", EVENT_DSP_SET_FLOAT_NAME,
                                  Event_dsp_set_float_name_process);
    Event_handler_set_dsp_process(eh, "d.F", EVENT_DSP_SET_FLOAT,
                                  Event_dsp_set_float_process);
    Event_handler_set_dsp_process(eh, "d.Tn", EVENT_DSP_SET_RELTIME_NAME,
                                  Event_dsp_set_reltime_name_process);
    Event_handler_set_dsp_process(eh, "d.T", EVENT_DSP_SET_RELTIME,
                                  Event_dsp_set_reltime_process);

    if (Event_names_error(eh->event_names))
    {
        del_Event_handler(eh);
        return NULL;
    }
    Playdata_set_event_filter(global_state, eh->event_names);
    return eh;
}


Event_names* Event_handler_get_names(Event_handler* eh)
{
    assert(eh != NULL);
    return eh->event_names;
}


bool Event_handler_set_ch_process(Event_handler* eh,
                                  const char* name,
                                  Event_type type,
                                  bool (*ch_process)(Channel_state*, char*))
{
    assert(eh != NULL);
    assert(name != NULL);
    assert(strlen(name) > 0);
    assert(strlen(name) < EVENT_NAME_MAX);
    assert(EVENT_IS_CHANNEL(type));
    assert(ch_process != NULL);
    if (!Event_names_add(eh->event_names, name, type))
    {
        return false;
    }
    eh->ch_process[type] = ch_process;
    return true;
}


bool Event_handler_set_general_process(Event_handler* eh,
                                       const char* name,
                                       Event_type type,
                                       bool (*general_process)(General_state*,
                                                               char*))
{
    assert(eh != NULL);
    assert(name != NULL);
    assert(strlen(name) > 0);
    assert(strlen(name) < EVENT_NAME_MAX);
    assert(EVENT_IS_GENERAL(type));
    assert(general_process != NULL);
    if (!Event_names_add(eh->event_names, name, type))
    {
        return false;
    }
    eh->general_process[type] = general_process;
    return true;
}


bool Event_handler_set_control_process(Event_handler* eh,
                                       const char* name,
                                       Event_type type,
                                       bool (*control_process)(General_state*,
                                                               char*))
{
    assert(eh != NULL);
    assert(name != NULL);
    assert(strlen(name) > 0);
    assert(strlen(name) < EVENT_NAME_MAX);
    assert(EVENT_IS_CONTROL(type));
    assert(control_process != NULL);
    if (!Event_names_add(eh->event_names, name, type))
    {
        return false;
    }
    eh->control_process[type] = control_process;
    return true;
}


bool Event_handler_set_global_process(Event_handler* eh,
                                      const char* name,
                                      Event_type type,
                                      bool (*global_process)(Playdata*,
                                                             char*))
{
    assert(eh != NULL);
    assert(name != NULL);
    assert(strlen(name) > 0);
    assert(strlen(name) < EVENT_NAME_MAX);
    assert(EVENT_IS_GLOBAL(type));
    assert(global_process != NULL);
    if (!Event_names_add(eh->event_names, name, type))
    {
        return false;
    }
    eh->global_process[type] = global_process;
    return true;
}


bool Event_handler_set_ins_process(Event_handler* eh,
                                   const char* name,
                                   Event_type type,
                                   bool (*ins_process)(Instrument_params*, char*))
{
    assert(eh != NULL);
    assert(name != NULL);
    assert(strlen(name) > 0);
    assert(strlen(name) < EVENT_NAME_MAX);
    assert(EVENT_IS_INS(type));
    assert(ins_process != NULL);
    if (!Event_names_add(eh->event_names, name, type))
    {
        return false;
    }
    eh->ins_process[type] = ins_process;
    return true;
}


bool Event_handler_set_generator_process(Event_handler* eh,
                                         const char* name,
                                         Event_type type,
                                         bool (*gen_process)(Generator*,
                                                             Channel_state*,
                                                             char*))
{
    assert(eh != NULL);
    assert(name != NULL);
    assert(strlen(name) > 0);
    assert(strlen(name) < EVENT_NAME_MAX);
    assert(EVENT_IS_GENERATOR(type));
    assert(gen_process != NULL);
    if (!Event_names_add(eh->event_names, name, type))
    {
        return false;
    }
    eh->generator_process[type] = gen_process;
    return true;
}


bool Event_handler_set_effect_process(Event_handler* eh,
                                      const char* name,
                                      Event_type type,
                                      bool (*effect_process)(Effect*, char*))
{
    assert(eh != NULL);
    assert(name != NULL);
    assert(strlen(name) > 0);
    assert(strlen(name) < EVENT_NAME_MAX);
    assert(EVENT_IS_EFFECT(type));
    assert(effect_process != NULL);
    if (!Event_names_add(eh->event_names, name, type))
    {
        return false;
    }
    eh->effect_process[type] = effect_process;
    return true;
}


bool Event_handler_set_dsp_process(Event_handler* eh,
                                   const char* name,
                                   Event_type type,
                                   bool (*dsp_process)(DSP_conf*,
                                                       Channel_state*,
                                                       char*))
{
    assert(eh != NULL);
    assert(name != NULL);
    assert(strlen(name) > 0);
    assert(strlen(name) < EVENT_NAME_MAX);
    assert(EVENT_IS_DSP(type));
    assert(dsp_process != NULL);
    if (!Event_names_add(eh->event_names, name, type))
    {
        return false;
    }
    eh->dsp_process[type] = dsp_process;
    return true;
}


static bool Event_handler_handle(Event_handler* eh,
                                 int index,
                                 Event_type type,
                                 char* fields)
{
    assert(eh != NULL);
    assert(index >= -1);
    assert(index < KQT_COLUMNS_MAX);
    assert(EVENT_IS_VALID(type));
    General_state* gstate = (General_state*)eh->global_state;
    if (index >= 0)
    {
        gstate = (General_state*)eh->ch_states[index];
    }
    if (!General_state_events_enabled(gstate) &&
            type != EVENT_GENERAL_IF && type != EVENT_GENERAL_END_IF)
    {
        return true;
    }
    if (EVENT_IS_CHANNEL(type))
    {
        assert(index >= 0);
        if (eh->ch_process[type] == NULL)
        {
            return false;
        }
        return eh->ch_process[type](eh->ch_states[index], fields);
    }
    else if (EVENT_IS_INS(type))
    {
        assert(index >= 0);
//        Instrument* ins = Ins_table_get(eh->insts, index);
        Instrument* ins = Ins_table_get(eh->insts,
                                        eh->ch_states[index]->instrument);
        if (ins == NULL)
        {
            return false;
        }
        Instrument_params* ins_params = Instrument_get_params(ins);
        assert(ins_params != NULL);
        return eh->ins_process[type](ins_params, fields);
    }
    else if (EVENT_IS_GLOBAL(type))
    {
        assert(index == -1);
        if (eh->global_process[type] == NULL)
        {
            return false;
        }
        return eh->global_process[type](eh->global_state, fields);
    }
    else if (EVENT_IS_GENERATOR(type))
    {
        assert(index >= 0);
        Instrument* ins = Ins_table_get(eh->insts,
                                        eh->ch_states[index]->instrument);
        if (ins == NULL)
        {
            return false;
        }
        Generator* gen = Instrument_get_gen(ins,
                                            eh->ch_states[index]->generator);
        if (gen == NULL)
        {
            return false;
        }
        return eh->generator_process[type](gen, eh->ch_states[index], fields);
    }
    else if (EVENT_IS_EFFECT(type))
    {
        assert(index >= 0);
        Effect_table* effects = eh->effects;
        if (eh->ch_states[index]->inst_effects)
        {
            if (eh->ch_states[index]->effect >= KQT_INST_EFFECTS_MAX)
            {
                return false;
            }
            Instrument* ins = Ins_table_get(eh->insts,
                                            eh->ch_states[index]->instrument);
            if (ins == NULL)
            {
                return false;
            }
            effects = Instrument_get_effects(ins);
        }
        if (effects == NULL)
        {
            return false;
        }
        Effect* eff = Effect_table_get(effects, eh->ch_states[index]->effect);
        if (eff == NULL)
        {
            return false;
        }
        return eh->effect_process[type](eff, fields);
    }
    else if (EVENT_IS_DSP(type))
    {
        assert(index >= 0);
        Effect_table* effects = eh->effects;
        if (eh->ch_states[index]->inst_effects)
        {
            if (eh->ch_states[index]->effect >= KQT_INST_EFFECTS_MAX)
            {
                return false;
            }
            Instrument* ins = Ins_table_get(eh->insts,
                                            eh->ch_states[index]->instrument);
            if (ins == NULL)
            {
                return false;
            }
            effects = Instrument_get_effects(ins);
        }
        if (effects == NULL)
        {
            return false;
        }
        Effect* eff = Effect_table_get(effects, eh->ch_states[index]->effect);
        if (eff == NULL)
        {
            return false;
        }
        DSP_table* dsps = Effect_get_dsps(eff);
#if 0
        if (eh->ch_states[index]->dsp_context >= 0)
        {
            Instrument* ins = Ins_table_get(eh->insts,
                                            eh->ch_states[index]->dsp_context);
            if (ins == NULL)
            {
                return false;
            }
            dsps = Instrument_get_dsps(ins);
        }
        if (dsps == NULL)
        {
            return false;
        }
#endif
        DSP_conf* conf = DSP_table_get_conf(dsps, eh->ch_states[index]->dsp);
        if (conf == NULL)
        {
            return false;
        }
        return eh->dsp_process[type](conf, eh->ch_states[index], fields);
    }
    else if (EVENT_IS_CONTROL(type))
    {
        assert(index == -1);
        return eh->control_process[type]((General_state*)eh->global_state,
                                         fields);
    }
    else if (EVENT_IS_GENERAL(type))
    {
        General_state* gstate = (General_state*)eh->global_state;
        if (index >= 0)
        {
            gstate = (General_state*)eh->ch_states[index];
        }
        return eh->general_process[type](gstate, fields);
    }
    return false;
}


bool Event_handler_trigger(Event_handler* eh,
                           int index,
                           char* desc,
                           bool silent)
{
    assert(eh != NULL);
    assert(index >= -1);
    assert(index < KQT_COLUMNS_MAX);
    assert(desc != NULL);
    Read_state* state = READ_STATE_AUTO;
    char* str = read_const_char(desc, '[', state);
    char event_name[EVENT_NAME_MAX + 2] = { '\0' };
    str = read_string(str, event_name, EVENT_NAME_MAX + 2, state);
    str = read_const_char(str, ',', state);
    if (state->error)
    {
        return false;
    }
    Event_type type = Event_names_get(eh->event_names, event_name);
    if (type == EVENT_NONE)
    {
        return false;
    }
    assert(Event_type_is_supported(type));
    if ((EVENT_IS_GLOBAL(type) != (index == -1)) &&
            !EVENT_IS_GENERAL(type) && !EVENT_IS_CONTROL(type))
    {
        return false;
    }
    if (!Event_handler_handle(eh, index, type, str))
    {
        return false;
    }
    if (!silent && Event_names_get_pass(eh->event_names, event_name))
    {
        Event_buffer_add(eh->event_buffer, index, desc);
    }
    return true;
}


bool Event_handler_receive(Event_handler* eh, char* dest, int size)
{
    assert(eh != NULL);
    assert(dest != NULL);
    assert(size > 0);
    return Event_buffer_get(eh->event_buffer, dest, size);
}


Playdata* Event_handler_get_global_state(Event_handler* eh)
{
    assert(eh != NULL);
    return eh->global_state;
}


void Event_handler_clear_buffer(Event_handler* eh)
{
    assert(eh != NULL);
    Event_buffer_clear(eh->event_buffer);
    return;
}


bool Event_handler_add_channel_gen_state_key(Event_handler* eh,
                                             const char* key)
{
    assert(eh != NULL);
    assert(key != NULL);
    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        if (!Channel_gen_state_set_key(eh->ch_states[i]->cgstate, key))
        {
            return false;
        }
    }
    return true;
}


void del_Event_handler(Event_handler* eh)
{
    if (eh == NULL)
    {
        return;
    }
    del_Event_names(eh->event_names);
    del_Event_buffer(eh->event_buffer);
//    del_Playdata(eh->global_state); // TODO: enable if Playdata becomes private
    xfree(eh);
    return;
}


