

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


#ifndef EVENT_GENERAL_DEF
#error "EVENT_GENERAL_DEF(..) not defined"
#endif


//                Name      Type suffix     Arg type        Validator
EVENT_GENERAL_DEF("#",      comment,        STRING,         v_any_str)

EVENT_GENERAL_DEF("?",      cond,           BOOL,           v_any_bool)
EVENT_GENERAL_DEF("?if",    if,             NONE,           NULL)
EVENT_GENERAL_DEF("?else",  else,           NONE,           NULL)
EVENT_GENERAL_DEF("?end",   end_if,         NONE,           NULL)

//EVENT_GENERAL_DEF("signal", signal,         STRING,         v_any_str)
EVENT_GENERAL_DEF("calln",  call_name,      STRING,         v_any_str)
EVENT_GENERAL_DEF("call",   call,           REALTIME,       NULL)


#undef EVENT_GENERAL_DEF


