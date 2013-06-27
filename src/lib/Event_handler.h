

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_EVENT_HANDLER_H
#define K_EVENT_HANDLER_H


#include <stdbool.h>

#include <Channel_state.h>
#include <DSP_conf.h>
#include <General_state.h>
#include <Generator.h>
#include <Ins_table.h>
#include <Playdata.h>
#include <Event_names.h>
#include <Event_type.h>
#include <Value.h>


typedef struct Event_handler Event_handler;


/**
 * Creates a new Event handler.
 *
 * \return   The new Event handler if successful, or \c NULL if memory
 *           allocation failed.
 */
Event_handler* new_Event_handler(
        Playdata* global_state,
        Channel_state** ch_states,
        Ins_table* insts,
        Effect_table* effects);


/**
 * Retrieves the Event names from the Event handler.
 *
 * \param eh   The Event handler -- must not be \c NULL.
 *
 * \return   The Event names.
 */
Event_names* Event_handler_get_names(Event_handler* eh);


/**
 * Registers a Control Event processor.
 *
 * \param eh   The Event handler -- must not be \c NULL.
 */
bool Event_handler_set_control_process(
        Event_handler* eh,
        Event_type type,
        bool (*control_process)(General_state*, Value*));


/**
 * Registers a general Event processor.
 *
 * \param eh        The Event handler -- must not be \c NULL.
 * \param name      The name of the Event -- must not be \c NULL, empty string
 *                  or longer than EVENT_NAME_MAX characters.
 * \param type      The type of the Event -- must be a general Event.
 * \param process   The process function -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Event_handler_set_general_process(
        Event_handler* eh,
        Event_type type,
        bool (*general_process)(General_state*, Value*));


/**
 * Registers a Channel Event processor.
 *
 * \param eh        The Event handler -- must not be \c NULL.
 * \param name      The name of the Event -- must not be \c NULL, empty string
 *                  or longer than EVENT_NAME_MAX characters.
 * \param type      The type of the Event -- must be a Channel Event.
 * \param process   The process function -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Event_handler_set_ch_process(
        Event_handler* eh,
        Event_type type,
        bool (*ch_process)(Channel_state*, Value*));


/**
 * Registers a master event processor.
 *
 * \param eh        The Event handler -- must not be \c NULL.
 * \param name      The name of the event -- must not be \c NULL, empty string
 *                  or longer than EVENT_NAME_MAX characters.
 * \param type      The type of the event -- must be a master event.
 * \param process   The process function -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Event_handler_set_master_process(
        Event_handler* eh,
        Event_type type,
        bool (*master_process)(Playdata*, Value*));


/**
 * Registers an Instrument Event processor.
 *
 * \param eh        The Event handler -- must not be \c NULL.
 * \param name      The name of the Event -- must not be \c NULL, empty string
 *                  or longer than EVENT_NAME_MAX characters.
 * \param type      The type of the Event -- must be an Instrument Event.
 * \param process   The process function -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Event_handler_set_ins_process(
        Event_handler* eh,
        Event_type type,
        bool (*ins_process)(Instrument_params*, Value*));


/**
 * Registers a Generator Event processor.
 *
 * \param eh        The Event handler -- must not be \c NULL.
 * \param name      The name of the Event -- must not be \c NULL, empty string
 *                  or longer than EVENT_NAME_MAX characters.
 * \param type      The type of the Event -- must be a Generator Event.
 * \param process   The process function -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Event_handler_set_generator_process(
        Event_handler* eh,
        Event_type type,
        bool (*gen_process)(Generator*, Channel_state*, Value*));


/**
 * Registers an Effect Event processor.
 *
 * \param eh        The Event handler -- must not be \c NULL.
 * \param name      The name of the Event -- must not be \c NULL, empty string
 *                  or longer than EVENT_NAME_MAX characters.
 * \param type      The type of the Event -- must be an Effect Event.
 * \param process   The process function -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Event_handler_set_effect_process(
        Event_handler* eh,
        Event_type type,
        bool (*effect_process)(Effect*, Value*));


/**
 * Registers a DSP Event processor.
 *
 * \param eh        The Event handler -- must not be \c NULL.
 * \param name      The name of the Event -- must not be \c NULL, empty string
 *                  or longer than EVENT_NAME_MAX characters.
 * \param type      The type of the Event -- must be a DSP Event.
 * \param process   The process function -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Event_handler_set_dsp_process(
        Event_handler* eh,
        Event_type type,
        bool (*dsp_process)(DSP_conf*, Channel_state*, Value*));


/**
 * Triggers an Event with an expression parameter.
 *
 * \param eh       The Event handler -- must not be \c NULL.
 * \param index    The index number -- must be >= \c 0 and
 *                 < \c KQT_COLUMNS_MAX. This should match the channel where
 *                 the Event is triggered.
 * \param desc     The Event description in JSON format -- must not be \c NULL.
 * \param silent   Silent mode indicator.
 * \param meta     The meta variable, or \c NULL.
 *
 * \return   \c true if the Event was triggered successfully, otherwise
 *           \c false.
 */
bool Event_handler_trigger(
        Event_handler* eh,
        int index,
        char* desc, // FIXME: this should be const
        bool silent,
        Value* meta);


/**
 * Triggers an Event with a const parameter.
 *
 * \param eh       The Event handler -- must not be \c NULL.
 * \param index    The index number -- must be >= \c 0 and
 *                 < \c KQT_COLUMNS_MAX. This should match the channel where
 *                 the Event is triggered.
 * \param desc     The Event description in JSON format -- must not be \c NULL.
 * \param silent   Silent mode indicator.
 *
 * \return   \c true if the Event was triggered successfully, otherwise
 *           \c false.
 */
bool Event_handler_trigger_const(
        Event_handler* eh,
        int index,
        char* desc,
        bool silent);


/**
 * Receives an event from the Event handler.
 *
 * \param eh     The Event handler -- must not be \c NULL.
 * \param dest   The destination buffer -- must not be \c NULL.
 * \param size   The size of the destination buffer including the
 *               terminating byte -- must be positive.
 *
 * \return   \c true if an event was found, otherwise \c false.
 */
bool Event_handler_receive(Event_handler* eh, char* dest, int size);


/**
 * Receives a tracker-specific event from the Event handler.
 *
 * \param eh     The Event handler -- must not be \c NULL.
 * \param dest   The destination buffer -- must not be \c NULL.
 * \param size   The size of the destination buffer including the
 *               terminating byte -- must be positive.
 *
 * \return   \c true if an event was found, otherwise \c false.
 */
bool Event_handler_treceive(Event_handler* eh, char* dest, int size);


/**
 * Returns the global state inside the Event handler.
 *
 * \param eh   The Event handler -- must not be \c NULL.
 *
 * \return   The global state.
 */
Playdata* Event_handler_get_global_state(Event_handler* eh);


/**
 * Clears the buffer of the event filter.
 *
 * \param eh   The Event handler -- must not be \c NULL.
 */
void Event_handler_clear_buffers(Event_handler* eh);


/**
 * Adds a key into all Channel-specific generator parameter dictionaries.
 *
 * \param eh    The Event handler -- must not be \c NULL.
 * \param key   The key -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Event_handler_add_channel_gen_state_key(
        Event_handler* eh,
        const char* key);


/**
 * Destroys an existing Event handler.
 *
 * \param eh   The Event handler, or \c NULL.
 */
void del_Event_handler(Event_handler* eh);


#endif // K_EVENT_HANDLER_H


