

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_WORK_BUFFERS_H
#define K_WORK_BUFFERS_H


#include <player/Work_buffer.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


typedef enum
{
    WORK_BUFFER_PITCH_PARAMS,
    WORK_BUFFER_ACTUAL_PITCHES,
    WORK_BUFFER_ACTUAL_FORCES,
    WORK_BUFFER_IMPL_1,
    WORK_BUFFER_IMPL_2,
    WORK_BUFFER_IMPL_3,
    WORK_BUFFER_TIME_ENV,
    WORK_BUFFER_COUNT_
} Work_buffer_type;


/**
 * A collection of Work buffers for various audio rendering purposes.
 */
typedef struct Work_buffers Work_buffers;


/**
 * Create new Work buffers.
 *
 * \param size   The buffer size -- must be >= \c 0 and
 *               <= \c WORK_BUFFER_SIZE_MAX.
 *
 * \return   The new Work buffers if successful, or \c NULL if memory
 *           allocation failed.
 */
Work_buffers* new_Work_buffers(uint32_t buf_size);


/**
 * Resize the Work buffers.
 *
 * \param buffers    The Work buffers -- must not be \c NULL.
 * \param new_size   The new buffer size -- must be >= \c 0 and
 *                   <= \c WORK_BUFFER_SIZE_MAX.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Work_buffers_resize(Work_buffers* buffers, uint32_t new_size);


/**
 * Get a Work buffer.
 *
 * \param buffers   The Work buffers -- must not be \c NULL.
 * \param type      The Work buffer type -- must be valid.
 *
 * \return   The Work buffer. This is never \c NULL.
 */
const Work_buffer* Work_buffers_get_buffer(
        const Work_buffers* buffers, Work_buffer_type type);


/**
 * Get contents of a Work buffer.
 *
 * This is a convenience function that uses \a Work_buffer_get_contents
 * directly.
 *
 * \param buffers   The Work buffers -- must not be \c NULL.
 * \param type      The Work buffer type -- must be valid.
 *
 * \return   The address of the internal buffer, with a valid index range of
 *           [-1, Work_buffer_get_size(\a buffer)]. For devices that receive
 *           the buffer from a caller, this function never returns \c NULL.
 */
const float* Work_buffers_get_buffer_contents(
        const Work_buffers* buffers, Work_buffer_type type);


/**
 * Get mutable contents of a Work buffer.
 *
 * This is a convenience function that uses \a Work_buffer_get_contents_mut
 * directly.
 *
 * \param buffers   The Work buffers -- must not be \c NULL.
 * \param type      The Work buffer type -- must be valid.
 *
 * \return   The address of the internal buffer, with a valid index range of
 *           [-1, Work_buffer_get_size(\a buffer)]. For devices that receive
 *           the buffer from a caller, this function never returns \c NULL.
 */
float* Work_buffers_get_buffer_contents_mut(
        const Work_buffers* buffers, Work_buffer_type type);


/**
 * Get mutable contents of a Work buffer as integer data.
 *
 * This is a convenience function that uses \a Work_buffer_get_contents_int_mut
 * directly.
 *
 * \param buffers   The Work buffers -- must not be \c NULL.
 * \param type      The Work buffer type -- must be valid.
 *
 * \return   The address of the internal buffer, with a valid index range of
 *           [-1, Work_buffer_get_size(\a buffer)]. For devices that receive
 *           the buffer from a caller, this function never returns \c NULL.
 */
int32_t* Work_buffers_get_buffer_contents_int_mut(
        const Work_buffers* buffers, Work_buffer_type type);


/**
 * Destroy existing Work buffers.
 *
 * \param buffers   The Work buffers, or \c NULL.
 */
void del_Work_buffers(Work_buffers* buffers);


#endif // K_WORK_BUFFERS_H


