

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_TRIGGER_H
#define K_TRIGGER_H


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <kunquat/limits.h>
#include <player/Event_names.h>
#include <player/Event_type.h>
#include <string/Streader.h>
#include <Tstamp.h>


/**
 * Trigger causes firing of an event at a specified location.
 */
typedef struct Trigger
{
    Tstamp pos;         ///< The Trigger position.
    int ch_index;       ///< Channel number.
    Event_type type;    ///< The event type.
    char* desc;         ///< Trigger description in JSON format.
} Trigger;


/**
 * Create a Trigger of specified type.
 *
 * \param type   The event type -- must be valid.
 * \param pos    The Trigger position -- must not be \c NULL.
 *
 * \return   The new Trigger if successful, or \c NULL if memory allocation
 *           failed or the event type isn't supported.
 */
Trigger* new_Trigger(Event_type type, Tstamp* pos);


/**
 * Create a Trigger from a JSON string.
 *
 * \param sr      The Streader of the data -- must not be \c NULL.
 * \param names   The Event names -- must not be \c NULL.
 *
 * \return   The new Trigger if successful, otherwise \c NULL.
 */
Trigger* new_Trigger_from_string(Streader* sr, const Event_names* names);


/**
 * Get the Trigger position (relative to the containing Pattern).
 *
 * \param trigger   The Trigger -- must not be \c NULL.
 *
 * \return   The position.
 */
Tstamp* Trigger_get_pos(Trigger* trigger);


/**
 * Get the event type of the Trigger.
 *
 * \param trigger   The Trigger -- must not be \c NULL.
 *
 * \return   The event type.
 */
Event_type Trigger_get_type(Trigger* trigger);


/**
 * Get a JSON description of the Trigger (does not include timestamp).
 *
 * \param trigger   The Trigger -- must not be \c NULL.
 *
 * \return   The JSON string.
 */
char* Trigger_get_desc(Trigger* trigger);


/**
 * Destroy an existing Trigger.
 *
 * \param trigger   The Trigger, or \c NULL.
 */
void del_Trigger(Trigger* trigger);


#endif // K_TRIGGER_H


