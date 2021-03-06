#ifndef ZERO_RING_BUFFER_H
#define ZERO_RING_BUFFER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <signal.h>
#include <stdbool.h>

// + Implementation
//
//     This ring buffer is constrained to fixed size slots. This
//     allows us to avoid all of the extra operations required to
//     check variable sized slot boundaries and such. If a need
//     arises in the future to have a zero copy lockless ring
//     buffer with variable sized slots, a new ring buffer should
//     be implemented.


// + Note
//     All of the header variables must be the same size as the
//     underlying registers for the target architectuire to ensure
//     that increments/decrements and reads/writes are performed
//     as a single instruction.
//#if defined(__x86_64__)
//    typedef uint64_t sig_atomic_t;
//#elif defined(__i386)
//    typedef uint32_t sig_atomic_t;
//#else
//    #error Unknown processor type. Cannot determine appropriate sized type for ring buffer.
//#endif

// Computes the overall storage size for a ring buffer which
// includes the ring buffer internals. It is strongly recommended
// that you use this macro for allocating/defining storage for
// your ring buffer. Otherwise, memory corruption may occur as
// you would be actually writing off of the end of your ring
// buffer storage.
#define RING_BUFFER_STORAGE_SIZE(slot_size, slot_count) \
    ((slot_size * slot_count) + sizeof(zero_ring_buffer))


// Representation of a zero copy lockless ring buffer. This ring
// buffer works on the premise that there will only be a single
// write and single reader ever operating on the ring buffer. It
// is also designed in such a way that the internals manipulated
// by the writer/reader provide a natural means of mutual exclusion.
// Thus, no locking is required. Furthermore, copying is avoided
// by this implementation by utilizing a two step read/write.
// First the slot to be read or written to is retrieved directly
// and then the user can modify it and commit it to the ring buffer.
//
// Note: The ring buffer will actually only be able to use
//         (m_slot_count - 1) slots because of the way the
//         tail can never be equal to the head. Be aware of
//         this if your ring buffer must be of an exact size.
struct zero_ring_buffer
{
    sig_atomic_t m_initialized; // Has the ring buffer been initialized?
    sig_atomic_t m_slot_size;   // The size in bytes of each slot in the ring
    sig_atomic_t m_slot_count;  // The number of slots in the ring
    sig_atomic_t m_head;        // The slot index to the head of the ring
    sig_atomic_t m_tail;        // The slot index to the tail of the ring
    uint8_t m_slots[0];         // Placeholder for the beginning of the ring
};


struct zero_ring_buffer* zero_ring_buffer_create(
    void* storage, sig_atomic_t slot_size, sig_atomic_t slot_count);

struct zero_ring_buffer* zero_ring_buffer_attach(void* storage);

static inline bool zero_ring_buffer_initialized(struct zero_ring_buffer* ring_buffer)
{
    return ring_buffer->m_initialized != 0;
}

static inline sig_atomic_t zero_ring_buffer_used(struct zero_ring_buffer* ring_buffer)
{
    return (ring_buffer->m_head - ring_buffer->m_tail) - 1;
}

static inline sig_atomic_t zero_ring_buffer_available(struct zero_ring_buffer* ring_buffer)
{
    return ring_buffer->m_slot_count - zero_ring_buffer_used(ring_buffer);
}

static inline sig_atomic_t zero_ring_buffer_size(struct zero_ring_buffer* ring_buffer)
{
    return ring_buffer->m_slot_count - 1;
}

static inline bool zero_ring_buffer_empty(struct zero_ring_buffer* ring_buffer)
{
    return zero_ring_buffer_used(ring_buffer) == 0;
}

static inline bool zero_ring_buffer_full(struct zero_ring_buffer* ring_buffer)
{
    return (ring_buffer->m_head - ring_buffer->m_tail) == ring_buffer->m_slot_count;
}

static inline void* zero_ring_buffer_stage_read(struct zero_ring_buffer* ring_buffer)
{
    if (!zero_ring_buffer_empty(ring_buffer))
    {
        sig_atomic_t tail_index = (ring_buffer->m_tail + 1) % ring_buffer->m_slot_count;
        return ring_buffer->m_slots + (tail_index * ring_buffer->m_slot_size);
    }

    return 0;
}

static inline void* zero_ring_buffer_stage_write(struct zero_ring_buffer* ring_buffer)
{
    if (!zero_ring_buffer_full(ring_buffer))
    {
        sig_atomic_t head_index = ring_buffer->m_head % ring_buffer->m_slot_count;
        return ring_buffer->m_slots + (head_index * ring_buffer->m_slot_size);
    }

    return 0;
}

static inline bool zero_ring_buffer_commit_read(struct zero_ring_buffer* ring_buffer)
{
    if (!zero_ring_buffer_empty(ring_buffer))
    {
        ++ring_buffer->m_tail;
        return true;
    }

    return false;
}

static inline bool zero_ring_buffer_commit_write(struct zero_ring_buffer* ring_buffer)
{
    if (!zero_ring_buffer_full(ring_buffer))
    {
        ++ring_buffer->m_head;
        return true;
    }

    return false;
}

static inline void zero_ring_buffer_destroy(struct zero_ring_buffer* ring_buffer)
{
    if (ring_buffer->m_initialized != 0)
    {
        ring_buffer->m_initialized = 0;
        ring_buffer->m_head = ring_buffer->m_tail + 1;
    }
}

#ifdef __cplusplus
}
#endif

#endif // ZERO_RING_BUFFER_H
