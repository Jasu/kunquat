

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Work_buffers.h>

#include <debug/assert.h>
#include <memory.h>
#include <player/Work_buffer.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


struct Work_buffers
{
    Work_buffer* buffers[WORK_BUFFER_COUNT_];
};


Work_buffers* new_Work_buffers(uint32_t buf_size)
{
    //assert(buf_size >= 0);
    assert(buf_size <= WORK_BUFFER_SIZE_MAX);

    Work_buffers* buffers = memory_alloc_item(Work_buffers);
    if (buffers == NULL)
        return NULL;

    // Sanitise fields
    for (int i = 0; i < WORK_BUFFER_COUNT_; ++i)
        buffers->buffers[i] = NULL;

    // Allocate buffers
    for (int i = 0; i < WORK_BUFFER_COUNT_; ++i)
    {
        buffers->buffers[i] = new_Work_buffer(buf_size);
        if (buffers->buffers[i] == NULL)
        {
            del_Work_buffers(buffers);
            return NULL;
        }
    }

    return buffers;
}


bool Work_buffers_resize(Work_buffers* buffers, uint32_t new_size)
{
    assert(buffers != NULL);
    //assert(new_size >= 0);
    assert(new_size <= WORK_BUFFER_SIZE_MAX);

    for (int i = 0; i < WORK_BUFFER_COUNT_; ++i)
    {
        if (!Work_buffer_resize(buffers->buffers[i], new_size))
            return false;
    }

    return true;
}


const Work_buffer* Work_buffers_get_buffer(
        const Work_buffers* buffers, Work_buffer_type type)
{
    assert(buffers != NULL);
    assert(type < WORK_BUFFER_COUNT_);

    return buffers->buffers[type];
}


const float* Work_buffers_get_buffer_contents(
        const Work_buffers* buffers, Work_buffer_type type)
{
    assert(buffers != NULL);
    assert(type < WORK_BUFFER_COUNT_);

    return Work_buffer_get_contents(buffers->buffers[type]);
}


float* Work_buffers_get_buffer_contents_mut(
        const Work_buffers* buffers, Work_buffer_type type)
{
    assert(buffers != NULL);
    assert(type < WORK_BUFFER_COUNT_);

    return Work_buffer_get_contents_mut(buffers->buffers[type]);
}


int32_t* Work_buffers_get_buffer_contents_int_mut(
        const Work_buffers* buffers, Work_buffer_type type)
{
    assert(buffers != NULL);
    assert(type < WORK_BUFFER_COUNT_);

    return Work_buffer_get_contents_int_mut(buffers->buffers[type]);
}


void del_Work_buffers(Work_buffers* buffers)
{
    if (buffers == NULL)
        return;

    for (int i = 0; i < WORK_BUFFER_COUNT_; ++i)
        del_Work_buffer(buffers->buffers[i]);

    memory_free(buffers);

    return;
}


