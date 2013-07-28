

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


#ifndef K_SONG_TABLE_H
#define K_SONG_TABLE_H


#include <stdint.h>
#include <stdbool.h>

#include <kunquat/limits.h>
#include <File_base.h>
#include <Song.h>


/**
 * Song table contains the Songs.
 */
typedef struct Song_table Song_table;


/**
 * Creates a new Song table.
 *
 * \return   The new Song table if successful, or \c NULL if memory
 *           allocation failed.
 */
Song_table* new_Song_table(void);


/**
 * Sets a Song in the Song table.
 *
 * \param table   The Song table -- must not be \c NULL.
 * \param index   The target index -- must be >= \c 0 and
 *                < \c KQT_SONGS_MAX.
 * \param song    The Song -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Song_table_set(Song_table* table, uint16_t index, Song* song);


/**
 * Gets a Song from the Song table.
 *
 * Note: Songs after an empty index are considered hidden and are not
 * returned.
 *
 * \param table   The Song table -- must not be \c NULL.
 * \param index   The song number -- must be >= \c 0 and
 *                < \c KQT_SONGS_MAX.
 *
 * \return   The Song if one exists, otherwise \c NULL.
 */
Song* Song_table_get(Song_table* table, uint16_t index);


/**
 * Sets existent status of a Song.
 *
 * \param table      The Song table -- must not be \c NULL.
 * \param index      The song number -- must be >= \c 0 and
 *                   < \c KQT_SONGS_MAX.
 * \param existent   The new existent status.
 */
void Song_table_set_existent(
        Song_table* table,
        uint16_t index,
        bool existent);


/**
 * Gets existent status of a Song.
 *
 * \param table   The Song table -- must not be \c NULL.
 * \param index   The song number -- must be >= \c 0 and
 *                < \c KQT_SONGS_MAX.
 *
 * \return   The existent status.
 */
bool Song_table_get_existent(Song_table* table, uint16_t index);


/**
 * Tells whether a song is empty.
 *
 * \param table   The Song table -- must not be \c NULL.
 * \param song    The song number -- must be >= \c 0 and
 *                < \c KQT_SONGS_MAX.
 *
 * \return   \c true if and only if \a song is empty.
 */
bool Song_table_is_song_empty(Song_table* table, uint16_t song_num);


/**
 * Destroys an existing Song table.
 *
 * \param table   The Song table, or \c NULL.
 */
void del_Song_table(Song_table* table);


#endif // K_SONG_TABLE_H


