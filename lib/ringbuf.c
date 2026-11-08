#include "ringbuf.h"

/*
 * head and tail are free-running counters; the slot is (counter & mask).
 * count = head - tail works across wrap-around thanks to unsigned
 * arithmetic, so all 'capacity' slots are usable (no "one empty slot" rule).
 *
 * Memory ordering: the producer must store the byte BEFORE publishing the
 * new head (release), and the consumer must read head (acquire) before
 * reading the byte. Plain 'volatile' does not order the non-volatile
 * buffer access, which is why C11 atomics are used here.
 */

bool ringbuf_init(ringbuf_t *rb, uint8_t *storage, size_t storage_size)
{
    if (storage_size < 2 || (storage_size & (storage_size - 1)) != 0)
        return false;
    rb->buf = storage;
    rb->mask = (uint32_t)storage_size - 1u;
    atomic_init(&rb->head, 0);
    atomic_init(&rb->tail, 0);
    return true;
}

bool ringbuf_push(ringbuf_t *rb, uint8_t byte)
{
    uint32_t head = atomic_load_explicit(&rb->head, memory_order_relaxed);
    uint32_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);
    if ((uint32_t)(head - tail) > rb->mask)
        return false;
    rb->buf[head & rb->mask] = byte;
    atomic_store_explicit(&rb->head, (uint32_t)(head + 1u), memory_order_release);
    return true;
}

bool ringbuf_pop(ringbuf_t *rb, uint8_t *byte)
{
    uint32_t tail = atomic_load_explicit(&rb->tail, memory_order_relaxed);
    uint32_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    if (head == tail)
        return false;
    *byte = rb->buf[tail & rb->mask];
    atomic_store_explicit(&rb->tail, (uint32_t)(tail + 1u), memory_order_release);
    return true;
}

size_t ringbuf_count(const ringbuf_t *rb)
{
    uint32_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    uint32_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);
    return head - tail;
}

size_t ringbuf_capacity(const ringbuf_t *rb)
{
    return (size_t)rb->mask + 1u;
}
