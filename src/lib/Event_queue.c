

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <assert.h>

#include <Event_queue.h>

#include <xmemory.h>


typedef struct Event_queue_node
{
    uint32_t pos;
    Event* event;
} Event_queue_node;


struct Event_queue
{
    /// The size of the queue.
    int size;
    /// The index of the first Event.
    int start;
    /// The number of Events inserted.
    int count;
    /// Storage for the Events and their positions in frames.
    Event_queue_node* queue;
};


Event_queue* new_Event_queue(int size)
{
    assert(size > 0);
    Event_queue* q = xalloc(Event_queue);
    if (q == NULL)
    {
        return NULL;
    }
    q->queue = xnalloc(Event_queue_node, size);
    if (q->queue == NULL)
    {
        xfree(q);
        return NULL;
    }
    q->size = size;
    q->start = 0;
    q->count = 0;
    return q;
}


bool Event_queue_ins(Event_queue* q, Event* event, uint32_t pos)
{
    assert(q != NULL);
    assert(event != NULL);
    if (q->count >= q->size)
    {
        assert(q->count == q->size);
        return false;
    }
    int i = q->count;
    for (i = q->count; i > 0; --i)
    {
        int cur = (q->start + i) % q->size;
        int prev = (q->start + (i - 1)) % q->size;
        if (q->queue[prev].pos <= pos)
        {
            break;
        }
        q->queue[cur].pos = q->queue[prev].pos;
        q->queue[cur].event = q->queue[prev].event;
    }
    q->queue[(q->start + i) % q->size].pos = pos;
    q->queue[(q->start + i) % q->size].event = event;
    ++q->count;
    return true;
}


bool Event_queue_get(Event_queue* q, Event** dest, uint32_t* pos)
{
    assert(q != NULL);
    assert(dest != NULL);
    assert(pos != NULL);
    if (q->count <= 0)
    {
        assert(q->count == 0);
        return false;
    }
    *dest = q->queue[q->start].event;
    *pos = q->queue[q->start].pos;
    --q->count;
    q->start = (q->start + 1) % q->size;
    return true;
}


bool Event_queue_peek(Event_queue* q, int index, Event** dest, uint32_t* pos)
{
    assert(q != NULL);
    assert(index >= 0);
    assert(dest != NULL);
    assert(pos != NULL);
    if (index >= q->count)
    {
        return false;
    }
    *dest = q->queue[(q->start + index) % q->size].event;
    *pos = q->queue[(q->start + index) % q->size].pos;
    return true;
}


void Event_queue_clear(Event_queue* q)
{
    assert(q != NULL);
    q->start = 0;
    q->count = 0;
    return;
}


bool Event_queue_resize(Event_queue* q, int new_size)
{
    assert(q != NULL);
    assert(new_size > 0);
    if (new_size != q->size)
    {
        Event_queue_node* new_queue = xrealloc(Event_queue_node, new_size, q->queue);
        if (new_queue == NULL)
        {
            return false;
        }
        q->queue = new_queue;
        q->size = new_size;
    }
    Event_queue_clear(q);
    return true;
}


void del_Event_queue(Event_queue* q)
{
    assert(q != NULL);
    xfree(q->queue);
    xfree(q);
    return;
}

