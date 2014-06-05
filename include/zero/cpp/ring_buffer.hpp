#ifndef ZERO_RING_BUFFER_HPP
#define ZERO_RING_BUFFER_HPP

#include <zero/c/zero_ring_buffer.h>

namespace zero
{

template <class Type>
class RingBuffer
{
private:
    zero_ring_buffer* m_ring_buffer;

public:
    RingBuffer() :
        m_ring_buffer(0)
    {
    }

    ~RingBuffer()
    {
        destroy();
    }

    static size_t storage_size(sig_atomic_t slot_count)
    {
        return RING_BUFFER_STORAGE_SIZE(sizeof(Type), slot_count);
    }

    void create(
        void* storage, sig_atomic_t slot_count)
    {
        m_ring_buffer = zero_ring_buffer_create(
            storage, sizeof(Type), slot_count);
    }

    void attach(void* storage)
    {
        m_ring_buffer = zero_ring_buffer_attach(storage);
    }

    bool initialized() const
    {
        if (m_ring_buffer)
        {
            return zero_ring_buffer_initialized(m_ring_buffer);
        }

        return false;
    }

    sig_atomic_t used() const
    {
        return zero_ring_buffer_used(m_ring_buffer);
    }

    sig_atomic_t available() const
    {
        return zero_ring_buffer_available(m_ring_buffer);
    }

    sig_atomic_t size() const
    {
        return zero_ring_buffer_size(m_ring_buffer);
    }

    bool empty() const
    {
        return zero_ring_buffer_empty(m_ring_buffer);
    }

    bool full()
    {
        return zero_ring_buffer_full(m_ring_buffer);
    }

    Type* stage_read()
    {
        return static_cast<Type*>(zero_ring_buffer_stage_read(m_ring_buffer));
    }

    Type* stage_write()
    {
      return static_cast<Type*>(zero_ring_buffer_stage_write(m_ring_buffer));
    }

    bool commit_read()
    {
        return zero_ring_buffer_commit_read(m_ring_buffer);
    }

    bool commit_write()
    {
        return zero_ring_buffer_commit_write(m_ring_buffer);
    }

    void destroy()
    {
        if (m_ring_buffer)
        {
            zero_ring_buffer_destroy(m_ring_buffer);
            m_ring_buffer = 0;
        }
    }
};

} // namespace zero

#endif // ZERO_RING_BUFFER_HPP
