

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


#ifndef K_WORK_BUFFER_H
#define K_WORK_BUFFER_H


#include <decl.h>
#include <kunquat/limits.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


#define WORK_BUFFER_SIZE_MAX ((KQT_AUDIO_BUFFER_SIZE_MAX) + 2)


/**
 * Create a new Work buffer.
 *
 * \param size   The buffer size -- must be >= \c 0 and
 *               <= \c WORK_BUFFER_SIZE_MAX.
 *
 * \return   The new Work buffer if successful, or \c NULL if memory allocation
 *           failed.
 */
Work_buffer* new_Work_buffer(int32_t size);


/**
 * Resize the Work buffer.
 *
 * \param buffer     The Work buffer -- must not be \c NULL.
 * \param new_size   The new buffer size -- must be >= \c 0 and
 *                   <= \c WORK_BUFFER_SIZE_MAX.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Work_buffer_resize(Work_buffer* buffer, int32_t new_size);


/**
 * Clear the Work buffer with floating-point zeroes.
 *
 * \param buffer      The Work buffer -- must not be \c NULL.
 * \param buf_start   The start index of the area to be cleared -- must be
 *                    >= \c -1 and less than or equal to the buffer size.
 * \param buf_stop    The stop index of the area to be cleared -- must be
 *                    >= \c -1 and less than or equal to buffer size + \c 1.
 */
void Work_buffer_clear(Work_buffer* buffer, int32_t buf_start, int32_t buf_stop);


/**
 * Get the size of the Work buffer.
 *
 * \param buffer   The Work buffer -- must not be \c NULL.
 *
 * \return   The size of the buffer.
 */
int32_t Work_buffer_get_size(const Work_buffer* buffer);


/**
 * Get the contents of the Work buffer.
 *
 * \param buffer   The Work buffer -- must not be \c NULL.
 *
 * \return   The address of the internal buffer, with a valid index range of
 *           [-1, Work_buffer_get_size(\a buffer)]. For devices that receive
 *           the buffer from a caller, this function never returns \c NULL.
 */
const float* Work_buffer_get_contents(const Work_buffer* buffer);


/**
 * Get the mutable contents of the Work buffer.
 *
 * \param buffer   The Work buffer -- must not be \c NULL.
 *
 * \return   The address of the internal buffer, with a valid index range of
 *           [-1, Work_buffer_get_size(\a buffer)]. For devices that receive
 *           the buffer from a caller, this function never returns \c NULL.
 */
float* Work_buffer_get_contents_mut(const Work_buffer* buffer);


/**
 * Get the mutable contents of the Work buffer as integer data.
 *
 * \param buffer   The Work buffer -- must not be \c NULL.
 *
 * \return   The address of the internal buffer, with a valid index range of
 *           [-1, Work_buffer_get_size(\a buffer)]. For devices that receive
 *           the buffer from a caller, this function never returns \c NULL.
 */
int32_t* Work_buffer_get_contents_int_mut(const Work_buffer* buffer);


/**
 * Copy contents of the Work buffer into another.
 *
 * \param dest        The destination Work buffer -- must not be \c NULL.
 * \param src         The source Work buffer -- must not be \c NULL or \a dest.
 * \param buf_start   The start index of the area to be copied -- must be
 *                    >= \c -1 and less than or equal to the buffer size.
 * \param buf_stop    The stop index of the area to be copied -- must be
 *                    >= \c -1 and less than or equal to buffer size + \c 1.
 */
void Work_buffer_copy(
        const Work_buffer* restrict dest,
        const Work_buffer* restrict src,
        int32_t buf_start,
        int32_t buf_stop);


/**
 * Mix the contents of a Work buffer into another as floating-point data.
 *
 * If the two buffers are the same Work buffer, this function does nothing.
 *
 * \param buffer      The Work buffer that will contain the end result -- must
 *                    not be \c NULL.
 * \param in          The input Work buffer -- must not be \c NULL and must
 *                    have the same size as \a buffer.
 * \param buf_start   The start index of the area to be mixed -- must be
 *                    >= \c -1 and less than or equal to the buffer size.
 * \param buf_stop    The stop index of the area to be mixed -- must be
 *                    >= \c -1 and less than or equal to buffer size + \c 1.
 */
void Work_buffer_mix(
        Work_buffer* buffer,
        const Work_buffer* in,
        int32_t buf_start,
        int32_t buf_stop);


/**
 * Destroy an existing Work buffer.
 *
 * \param buffer   The Work buffer, or \c NULL.
 */
void del_Work_buffer(Work_buffer* buffer);


#endif // K_WORK_BUFFER_H


