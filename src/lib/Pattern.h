

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


#ifndef K_PATTERN_H
#define K_PATTERN_H


#include <stdint.h>

#include <AAtree.h>
#include <Column.h>
#include <Connections.h>
#include <Channel.h>
#include <Pat_inst_ref.h>
#include <Tstamp.h>
#include <Event_handler.h>
#include <kunquat/limits.h>


/**
 * This object contains a (typically short) section of music.
 */
typedef struct Pattern Pattern;


#define PATTERN_DEFAULT_LENGTH (Tstamp_set(TSTAMP_AUTO, 16, 0))


/**
 * Creates a new Pattern object.
 * The caller shall eventually call del_Pattern() to destroy the Pattern
 * returned.
 *
 * \see del_Pattern()
 *
 * \return   The new Pattern object if successful, or \c NULL if memory
 *           allocation failed.
 */
Pattern* new_Pattern(void);


/**
 * Parses the header of a Pattern.
 *
 * \param pat     The Pattern -- must not be \c NULL.
 * \param str     The textual description -- must not be \c NULL.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Pattern_parse_header(Pattern* pat, char* str, Read_state* state);


/**
 * Sets existent status of a Pattern instance.
 *
 * \param pat        The Pattern -- must not be \c NULL
 * \param index      The instance index -- must be >= \c 0 and
 *                   < \c KQT_PAT_INSTANCES_MAX.
 * \param existent   The new existence status.
 */
void Pattern_set_inst_existent(Pattern* pat, int index, bool existent);


/**
 * Gets existent status of a Pattern instance.
 *
 * \param pat     The Pattern -- must not be \c NULL
 * \param index   The instance index -- must be >= \c 0 and
 *                < \c KQT_PAT_INSTANCES_MAX.
 *
 * \return   \c true if instance \a index of \a pat exists, otherwise \c false.
 */
bool Pattern_get_inst_existent(const Pattern* pat, int index);


/**
 * Sets a location where the Pattern is stored.
 *
 * This function must be called whenever the Pattern is placed in a new
 * location. Calling it with old locations does (successfully) nothing.
 *
 * \param pat     The Pattern -- must not be \c NULL.
 * \param song    The song number -- must be >= \c 0 and
 *                < \c KQT_SONGS_MAX.
 * \param piref   The Pattern instance reference -- must be valid.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Pattern_set_location(Pattern* pat, int subsong, Pat_inst_ref* piref);


/**
 * Gets the locations where the Pattern is potentially stored.
 *
 * \param pat    The Pattern -- must not be \c NULL.
 * \param iter   The location where the iterator will be stored --
 *               must not be \c NULL.
 *
 * \return   The tree of potential locations.
 */
AAtree* Pattern_get_locations(Pattern* pat, AAiter** iter);


/**
 * Sets the length of the Pattern.
 *
 * No Events will be deleted if the new length is shorter than the old length.
 *
 * \param pat      The Pattern -- must not be \c NULL.
 * \param length   The new length -- must not be \c NULL and must be
 *                 non-negative.
 */
void Pattern_set_length(Pattern* pat, Tstamp* length);


/**
 * Gets the length of the Pattern.
 *
 * \param pat      The Pattern -- must not be \c NULL.
 *
 * \return   The length -- must not be freed.
 */
const Tstamp* Pattern_get_length(const Pattern* pat);


/**
 * Replaces a Column of the Pattern.
 *
 * \param pat     The Pattern -- must not be \c NULL.
 * \param index   The Column index -- must be >= \c 0 and < \c KQT_COLUMNS_MAX.
 * \param col     The Column -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Pattern_set_col(Pattern* pat, int index, Column* col);


/**
 * Returns a Column of the Pattern.
 *
 * \param pat     The Pattern -- must not be \c NULL.
 * \param index   The Column index -- must be >= \c 0 and < \c KQT_COLUMNS_MAX.
 *
 * \return   The Column.
 */
Column* Pattern_get_col(Pattern* pat, int index);


/**
 * Replaces the global Column of the Pattern.
 *
 * \param pat   The Pattern -- must not be \c NULL.
 * \param col   The Column -- must not be \c NULL.
 */
void Pattern_set_global(Pattern* pat, Column* col);


/**
 * Gets the global Column of the Pattern.
 *
 * \param pat   The Pattern -- must not be \c NULL.
 *
 * \return   The global Column.
 */
Column* Pattern_get_global(Pattern* pat);


/**
 * Mixes a portion of the Pattern. TODO: params
 *
 * \param pat           The Pattern -- must not be \c NULL.
 * \param nframes       The amount of frames to be mixed.
 * \param offset        The mixing buffer offset to be used -- must be
 *                      < \a nframes.
 * \param eh            The Event handler -- must not be \c NULL.
 * \param channels      The Channels -- must not be \c NULL.
 * \param connections   The Connections, or \c NULL if not applicable.
 *
 * \return   The amount of frames actually mixed. This is always
 *           <= \a nframes. A value that is < \a nframes indicates that the
 *           mixing of the Pattern is complete.
 */
uint32_t Pattern_mix(Pattern* pat,
                     uint32_t nframes,
                     uint32_t offset,
                     Event_handler* eh,
                     Channel** channels,
                     Connections* connections);


/**
 * Destroys an existing Pattern.
 *
 * \param pat   The Pattern, or \c NULL.
 */
void del_Pattern(Pattern* pat);


#endif // K_PATTERN_H


