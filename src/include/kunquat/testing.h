

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_TESTING_H
#define KQT_TESTING_H


#ifdef __cplusplus
extern "C" {
#endif


/**
 * \defgroup Testing Testing support functions
 * \{
 *
 * \brief
 * This module describes the Kunquat testing support functions.
 */


/**
 * Simulate a memory allocation error on a single allocation request.
 *
 * \param steps   Number of successful allocations that should be made before
 *                the simulated error. Negative value disables error simulation.
 */
void kqt_fake_out_of_memory(long steps);


/**
 * Get the total number of successful memory allocations made.
 *
 * \return   The number of allocations made.
 */
long kqt_get_memory_alloc_count(void);


/**
 * Suppress assert message printing to standard error output.
 *
 * This function may be useful in tests that check for the abort signal
 * emitted by assertion failures.
 */
void kqt_suppress_assert_messages(void);


/* \} */


#ifdef __cplusplus
}
#endif


#endif // KQT_TESTING_H


