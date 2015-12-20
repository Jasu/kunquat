

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_MEMORY_H
#define K_MEMORY_H


#include <stdint.h>
#include <stdlib.h>


/**
 * A simple memory management interface that supports simulated failures.
 */


/**
 * Shorthands for common operations.
 */
#define memory_alloc_item(type) memory_alloc(sizeof(type))
#define memory_alloc_items(type, n) memory_alloc(sizeof(type) * (n))
#define memory_calloc_items(type, n) memory_calloc((n), sizeof(type))
#define memory_realloc_items(type, n, ptr) memory_realloc((ptr), sizeof(type) * (n))


/**
 * Allocate a contiguous block of memory.
 *
 * \param size   The amount of bytes to be allocated.
 *
 * \return   The starting address of the allocated memory block, or \c NULL if
 *           memory allocation failed or \a size was \c 0.
 */
void* memory_alloc(size_t size);


/**
 * Allocate an initialised block of memory.
 *
 * \param item_count   Number of items to be allocated.
 * \param item_size    Size of a single item in bytes.
 *
 * \return   The starting address of the allocated memory block, or \c NULL if
 *           memory allocation failed or one of the arguments was \c 0.
 */
void* memory_calloc(size_t item_count, size_t item_size);


/**
 * Resize a memory block.
 *
 * \param ptr    The starting address of the memory block, or \c NULL.
 * \param size   The new size of the memory block.
 *
 * \return   The starting address of the resized memory block, or \c NULL if
 *           memory allocation failed or \a size was \c 0.
 */
void* memory_realloc(void* ptr, size_t size);


/**
 * Free a block of memory.
 *
 * \param ptr   The starting address of the memory block, or \c NULL.
 */
void memory_free(void* ptr);


/**
 * Simulate a memory allocation error on a single allocation request.
 *
 * \param steps   Number of successful allocations that should be made before
 *                the simulated error. Negative value disables error simulation.
 */
void memory_fake_out_of_memory(int32_t steps);


/**
 * Get the total number of successful memory allocations made.
 *
 * \return   The number of allocations made.
 */
int32_t memory_get_alloc_count(void);


#endif // K_MEMORY_H


