

/*
 * Author: Tomi Jylhä-Ollila, Finland 2012-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_TRACK_LIST_H
#define K_TRACK_LIST_H


#include <stdlib.h>

#include <string/Streader.h>


/**
 * Track list specifies the order of songs in an album.
 */
typedef struct Track_list Track_list;


/**
 * Creates a new Track list.
 *
 * \param sr   The Streader of the JSON data -- must not be \c NULL.
 *
 * \return   The new Track list if successful, otherwise \c NULL.
 */
Track_list* new_Track_list(Streader* sr);


/**
 * Returns the length of the Track list.
 *
 * \param tl   The Track list -- must not be \c NULL.
 *
 * \return   The length.
 */
size_t Track_list_get_len(const Track_list* tl);


/**
 * Returns a song index from the Track list.
 *
 * \param tl      The Track list -- must not be \c NULL.
 * \param index   The index -- must be >= \c 0 and
 *                < Track_list_get_len(\a tl).
 *
 * \return   The song index.
 */
int16_t Track_list_get_song_index(const Track_list* tl, size_t index);


/**
 * Returns a track number by song index.
 *
 * Current implementation does a linear-time search.
 *
 * \param tl           The Track list -- must not be \c NULL.
 * \param song_index   The song index -- must be >= \c 0.
 *
 * \return   The track that contains the song with \a song_index, or \c -1 if
 *           not found.
 */
int16_t Track_list_get_track_by_song(const Track_list* tl, int16_t song_index);


/**
 * Destroys an existing Track list.
 *
 * \param tl   The Track list, or \c NULL.
 */
void del_Track_list(Track_list* tl);


#endif // K_TRACK_LIST_H


