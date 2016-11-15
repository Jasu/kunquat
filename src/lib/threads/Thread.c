

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


#include <threads/Thread.h>

#include <debug/assert.h>
#include <Error.h>

#ifdef WITH_PTHREAD
#include <errno.h>
#include <pthread.h>
#endif

#include <stdbool.h>
#include <stdlib.h>


bool Thread_init(Thread* thread, void* (*func)(void*), void* arg, Error* error)
{
    rassert(thread != NULL);
    rassert(!thread->initialised);
    rassert(func != NULL);
    rassert(error != NULL);

#ifndef ENABLE_THREADS
    rassert(false);
#endif

#ifdef WITH_PTHREAD
    const int status = pthread_create(&thread->thread, NULL, func, arg);

    switch (status)
    {
        case 0:
            break;

        case EAGAIN:
        {
            Error_set(
                    error,
                    ERROR_RESOURCE,
                    "Could not allocate resources for a new thread");
            return false;
        }
        break;

        case EINVAL:
        {
            rassert(false);
        }
        break;

        case EPERM:
        {
            Error_set(
                    error,
                    ERROR_RESOURCE,
                    "No required permissions to create a thread");
            return false;
        }
        break;

        default:
        {
            Error_set(
                    error,
                    ERROR_RESOURCE,
                    "Unexpected error when creating a thread: %d",
                    status);
            return false;
        }
        break;
    }

#else
    rassert(false);

#endif

    thread->arg = arg;

    return true;
}


bool Thread_is_initialised(const Thread* thread)
{
    rassert(thread != NULL);
    return thread->initialised;
}


void Thread_cancel(Thread* thread)
{
    rassert(thread != NULL);
    rassert(thread->initialised);

#ifdef WITH_PTHREAD
    const int status = pthread_cancel(thread->thread);
    rassert(status != ESRCH);
    rassert(status == 0);
#endif

    return;
}


void Thread_join(Thread* thread)
{
    rassert(thread != NULL);

    if (!thread->initialised)
        return;

#ifdef WITH_PTHREAD
    const int status = pthread_join(thread->thread, NULL);
    rassert(status != EDEADLK);
    rassert(status != EINVAL);
    rassert(status != ESRCH);
    rassert(status == 0);
#endif

    thread->initialised = false;

    return;
}


