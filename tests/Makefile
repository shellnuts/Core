# Host-side unit tests for the portable code in lib/.
# The same .c files are compiled for the PC here and for the MCU by CMake.
#
#   make            core library tests: must always pass
#   make phase1     phase 1 exercise scoreboard (TODO until you implement them)

CC ?= cc
# -fno-sanitize-recover makes undefined behaviour (e.g. 1 << 31) a hard failure.
CFLAGS = -std=c11 -Wall -Wextra -Werror -Wshadow -g -O1 -I../lib \
         -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer
LDLIBS = -lm

TESTS = test_ringbuf test_pid
PHASE1 = test_bits test_moving_avg test_lpf test_debounce test_q15

.PHONY: all phase1 clean
all: $(TESTS)
	@for t in $(TESTS); do echo "== $$t"; ./$$t || exit 1; done

# Runs every exercise and prints a scoreboard. Never fails the build:
# use `make -C tests test_lpf && ./tests/test_lpf` to see one exercise's details.
phase1: $(PHASE1)
	@pass=0; \
	for t in $(PHASE1); do \
		if ./$$t > $$t.log 2>&1; then echo "  [x] $$t"; pass=$$((pass+1)); \
		else echo "  [ ] $$t   ($$(tail -n 1 $$t.log))"; fi; \
	done; \
	echo "Phase 1: $$pass/$(words $(PHASE1)) exercises passing"

test_%: test_%.c unit.h
	$(CC) $(CFLAGS) -o $@ $(filter %.c,$^) $(LDLIBS)

test_ringbuf: ../lib/ringbuf.c ../lib/ringbuf.h
test_pid: ../lib/pid.c ../lib/pid.h ../lib/plant.h
test_bits: ../lib/bits.h
test_moving_avg: ../lib/moving_avg.c ../lib/moving_avg.h
test_lpf: ../lib/lpf.c ../lib/lpf.h
test_debounce: ../lib/debounce.c ../lib/debounce.h
test_q15: ../lib/q15.c ../lib/q15.h ../lib/lpf.c ../lib/lpf.h

clean:
	rm -f $(TESTS) $(PHASE1) *.log
