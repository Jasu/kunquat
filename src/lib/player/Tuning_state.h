

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_TUNING_STATE_H
#define K_TUNING_STATE_H


#include <decl.h>
#include <kunquat/limits.h>

#include <stdbool.h>
#include <stdlib.h>


typedef struct Tuning_state
{
    int note_count;
    int ref_note;
    double global_offset;
    double drift;
    double note_offsets[KQT_TUNING_TABLE_NOTES];
} Tuning_state;


/**
 * Create a new Tuning state.
 *
 * \return   The new Tuning state if successful, or \c NULL if memory allocation failed.
 */
Tuning_state* new_Tuning_state(void);


/**
 * Initialise the Tuning state using a Tuning table as a reference.
 *
 * \param ts      The Tuning state -- must not be \c NULL.
 * \param table   The Tuning table -- must not be \c NULL.
 */
void Tuning_state_reset(Tuning_state* ts, const Tuning_table* table);


/**
 * Get a retuned pitch from the Tuning state.
 *
 * \param ts      The Tuning state -- must not be \c NULL.
 * \param table   The Tuning table -- must not be \c NULL.
 * \param cents   The original pitch in cents -- must be finite.
 *
 * \return   The retuned pitch in cents.
 */
double Tuning_state_get_retuned_pitch(
        const Tuning_state* ts, const Tuning_table* table, double cents);


/**
 * Retune the Tuning state.
 *
 * \param ts        The Tuning state -- must not be \c NULL.
 * \param table     The Tuning table -- must not be \c NULL.
 * \param new_ref   The new reference pitch -- must be finite.
 * \param fixed     A pitch that should stay unchanged after retuning -- must be finite.
 */
void Tuning_state_retune(
        Tuning_state* ts, const Tuning_table* table, double new_ref, double fixed);


/**
 * Retune the Tuning state using a foreign Tuning table as a reference.
 *
 * \param ts       The Tuning state -- must not be \c NULL.
 * \param table    The Tuning table associated with \a ts -- must not be \c NULL.
 * \param source   The source Tuning table -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if the Tuning tables contain different
 *           amounts of notes.
 */
bool Tuning_state_retune_with_source(
        Tuning_state* ts, const Tuning_table* table, const Tuning_table* source);


/**
 * Get estimated pitch drift in the Tuning table.
 *
 * The estimate is most useful when the current reference note matches the initial one.
 *
 * \param ts    The Tuning state -- must not be \c NULL.
 *
 * \return   The estimated drift.
 */
double Tuning_state_get_estimated_drift(const Tuning_state* ts);


/**
 * Destroy an existing Tuning state.
 *
 * \param ts   The Tuning state, or \c NULL.
 */
void del_Tuning_state(Tuning_state* ts);


#endif // K_TUNING_STATE_H


