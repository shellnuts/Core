# Host-side unit tests for the portable code in lib/.
# The same .c files are compiled for the PC here and for the MCU by CMake.

CC ?= cc
CFLAGS = -std=c11 -Wall -Wextra -Werror -Wshadow -g -O1 -I../lib \
         -fsanitize=address,undefined -fno-omit-frame-pointer
LDLIBS = -lm

TESTS = test_ringbuf test_pid

.PHONY: all clean
all: $(TESTS)
	@for t in $(TESTS); do echo "== $$t"; ./$$t || exit 1; done

test_ringbuf: test_ringbuf.c ../lib/ringbuf.c unit.h ../lib/ringbuf.h
	$(CC) $(CFLAGS) -o $@ test_ringbuf.c ../lib/ringbuf.c $(LDLIBS)

test_pid: test_pid.c ../lib/pid.c unit.h ../lib/pid.h ../lib/plant.h
	$(CC) $(CFLAGS) -o $@ test_pid.c ../lib/pid.c $(LDLIBS)

clean:
	rm -f $(TESTS)
