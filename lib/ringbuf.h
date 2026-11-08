/*
 * ringbuf.h - Lock-free single-producer / single-consumer byte FIFO.
 *
 * Typical use: a UART RX interrupt pushes, the main loop (or a task) pops.
 * Safe without disabling interrupts as long as there is exactly ONE
 * producer and ONE consumer. Portable C11: unit-tested on the host.
 */
#ifndef RINGBUF_H
#define RINGBUF_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t *buf;
    uint32_t mask;         /* capacity - 1; capacity is a power of two */
    _Atomic uint32_t head; /* written only by the producer */
    _Atomic uint32_t tail; /* written only by the consumer */
} ringbuf_t;

/* 'storage_size' must be a power of two >= 2. Returns false otherwise. */
bool ringbuf_init(ringbuf_t *rb, uint8_t *storage, size_t storage_size);

bool ringbuf_push(ringbuf_t *rb, uint8_t byte);  /* false if full */
bool ringbuf_pop(ringbuf_t *rb, uint8_t *byte);  /* false if empty */
size_t ringbuf_count(const ringbuf_t *rb);
size_t ringbuf_capacity(const ringbuf_t *rb);

#endif
