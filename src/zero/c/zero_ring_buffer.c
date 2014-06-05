#include <stdlib.h>

#include "zero/c/zero_ring_buffer.h"

struct zero_ring_buffer* zero_ring_buffer_create(
    void* storage, sig_atomic_t slot_size, sig_atomic_t slot_count)
{
    struct zero_ring_buffer* ring_buffer = zero_ring_buffer_attach(storage);
    if (ring_buffer->m_initialized == 0)
    {
        ring_buffer->m_head = 1;
        ring_buffer->m_tail = 0;
        ring_buffer->m_slot_size = slot_size;
        ring_buffer->m_slot_count = slot_count;
        ring_buffer->m_initialized = 1;
    }
    else if (ring_buffer->m_slot_size != slot_size ||
             ring_buffer->m_slot_count != slot_count)
    {
        abort();
    }

    return ring_buffer;
}

struct zero_ring_buffer* zero_ring_buffer_attach(void* storage)
{
    return (struct zero_ring_buffer*)storage;
}
