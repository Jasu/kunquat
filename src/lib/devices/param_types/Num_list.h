

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_NUM_LIST_H
#define K_NUM_LIST_H


#include <stdint.h>

#include <string/Streader.h>


/**
 * This is a collection of (floating-point) numbers.
 */
typedef struct Num_list Num_list;


/**
 * Creates a new Number list from a textual description.
 *
 * \param sr   The Streader of the JSON data -- must not be \c NULL.
 *
 * \return   The new Number list if successful, otherwise \c NULL.
 */
Num_list* new_Num_list_from_string(Streader* sr);


/**
 * Returns the length of the Number list.
 *
 * \param nl   The Number list -- must not be \c NULL.
 *
 * \return   The length of the list.
 */
int32_t Num_list_length(const Num_list* nl);


/**
 * Returns a number from the Number list.
 *
 * \param ml      The Number list -- must not be \c NULL.
 * \param index   The index of the number -- must be >= \c 0 and less than the
 *                length of the list.
 *
 * \return   The number.
 */
double Num_list_get_num(const Num_list* nl, int32_t index);


/**
 * Destroys an existing Number list.
 *
 * \param nl   The Number list, or \c NULL.
 */
void del_Num_list(Num_list* nl);


#endif // K_NUM_LIST_H


