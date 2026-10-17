/*
 * syscalls.c - The minimum newlib needs to run printf/malloc on bare metal.
 *
 * newlib's printf eventually calls _write(); here that goes to USART1.
 * _sbrk() hands out heap memory between the end of .bss and the stack.
 */
#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>

#include "uart.h"

int _write(int fd, const char *buf, int len)
{
    (void)fd; /* stdout and stderr both go to the UART */
    uart1_write(buf, (size_t)len);
    return len;
}

int _read(int fd, char *buf, int len)
{
    (void)fd;
    (void)buf;
    (void)len;
    return 0; /* use uart1_getc() instead of scanf */
}

void *_sbrk(int incr)
{
    extern uint8_t end;              /* from the linker script: end of .bss */
    extern uint8_t _estack;
    extern uint32_t _min_stack_size; /* linker symbol: its ADDRESS is the value */
    static uint8_t *heap_end = &end;

    uint8_t *limit = &_estack - (uint32_t)&_min_stack_size;
    if (heap_end + incr > limit) {
        errno = ENOMEM;
        return (void *)-1;
    }
    uint8_t *prev = heap_end;
    heap_end += incr;
    return prev;
}

int _close(int fd) { (void)fd; return -1; }
int _fstat(int fd, struct stat *st) { (void)fd; st->st_mode = S_IFCHR; return 0; }
int _isatty(int fd) { (void)fd; return 1; }
int _lseek(int fd, int ptr, int dir) { (void)fd; (void)ptr; (void)dir; return 0; }
int _getpid(void) { return 1; }
int _kill(int pid, int sig) { (void)pid; (void)sig; errno = EINVAL; return -1; }
void _exit(int status) { (void)status; for (;;) { } }
