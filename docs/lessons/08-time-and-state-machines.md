# Lesson 8 — Time and state machines

**You will learn:** how a microcontroller keeps time, why `delay()` is
usually the wrong tool, how to compare times so that it keeps working when
the counter wraps around, and how to structure behaviour as a **state machine**.
Also: why buttons "bounce".

---

## 1. The system tick

The STM32 has a timer in the CPU core called **SysTick**. This project
configures it (`firmware/drivers/systick.c`) to fire an interrupt every
**1 ms**, and the handler does one thing:

```c
void SysTick_Handler(void) { s_ticks++; }
```

`millis()` returns `s_ticks`: the number of milliseconds since start-up.
It's your clock.

## 2. Blocking vs. non-blocking

The beginner's way to blink an LED:

```c
for (;;) {
    board_led_toggle(LED0);
    delay_ms(500);           // CPU does nothing else for 500 ms
}
```

It works, until you also want to read a button. While `delay_ms` waits,
the button is **ignored**. Add a third task and everything gets slower and
slower. This style is called **blocking**: the code stops and waits.

The **non-blocking** way: never wait. Check the clock on every pass of the
loop and act only when it's time:

```c
uint32_t last_blink = millis();

for (;;) {
    if (millis() - last_blink >= 500u) {    // has 500 ms passed?
        last_blink += 500u;
        board_led_toggle(LED0);
    }
    handle_buttons();                        // runs thousands of times per second
    handle_uart();
}
```

The loop spins very fast, and each task does a tiny bit of work when it's
due. That's exactly how `firmware/src/main.c` is written. It's called
**cooperative multitasking**: the tasks share the CPU by never hogging it.
(Phase 4's RTOS does the same thing more powerfully.)

## 3. Wrap-safe time comparison

`millis()` is a `uint32_t`. After 2³² ms ≈ **49.7 days** it wraps back to 0
(Lesson 2). A device that runs for two months will hit this. Does the
blink code break?

Let's shrink the problem to an **8-bit** clock (wraps every 256 ms) so the
numbers are easy to follow. Say the last event was at `start = 250` and the
interval is 20.

**❌ The tempting way:** compute a deadline.

```c
if (now >= start + interval)     // deadline = 250 + 20 = 270 → wraps to 14!
```

At `now = 251`, the check is `251 >= 14`, which is **true** after only 1 ms.
Wrong!

**✅ The right way:** compute the **elapsed time** by subtraction.

```c
if (now - start >= interval)
```

| now | `now − start` (8-bit) | elapsed | ≥ 20? |
|---|---|---|---|
| 251 | 251 − 250 = 1 | 1 ms | no ✓ |
| 255 | 5 | 5 ms | no ✓ |
| 4 (wrapped) | 4 − 250 = −246 → +256 = **10** | 10 ms | no ✓ |
| 14 | 14 − 250 → **20** | 20 ms | **yes** ✓ |

Unsigned subtraction wraps too, and the two wraps **cancel out**. The
difference is always the true elapsed time, as long as the elapsed time
itself is less than one full wrap (49 days for 32 bits).

> **Rule: always compare times as `(now - start) >= interval`, with
> unsigned types. Never compute a deadline with `start + interval`.**

## 4. `enum`: naming the possibilities

An `enum` gives names to a set of related integer constants:

```c
typedef enum {
    LIGHT_RED,       // = 0
    LIGHT_GREEN,     // = 1
    LIGHT_YELLOW,    // = 2
} light_t;

light_t current = LIGHT_RED;
```

It's much clearer than writing `current = 2` and hoping everyone remembers
what 2 means.

## 5. `switch`: choosing by value

```c
switch (current) {
case LIGHT_RED:
    /* ... */
    break;           // without break, execution "falls through" into the next case!
case LIGHT_GREEN:
    /* ... */
    break;
default:             // any value not listed
    break;
}
```

Compile with `-Wall`, and GCC warns if you forget to handle one of the enum's
values in the switch. That's a nice safety net.

## 6. State machines

Many embedded behaviours are "what should I do **now**, given what has
happened **so far**?" A **finite state machine (FSM)** answers that in a
structured way:

- a fixed set of **states**, where the system is always in exactly one of them,
- **inputs** (a button level, the current time...),
- **transitions**: rules like "in state A, if X happens, go to state B",
- **outputs/actions**, which happen in a state or on a transition.

Draw it first, then code it. A traffic light:

```
         after 5 s              after 4 s
   RED ────────────► GREEN ────────────► YELLOW
    ▲                                       │
    └───────────────── after 1 s ───────────┘
```

```c
typedef struct {
    light_t state;
    uint32_t since;          // when we entered the current state
} traffic_t;

void traffic_update(traffic_t *t, uint32_t now)
{
    uint32_t in_state = now - t->since;          // wrap-safe!

    switch (t->state) {
    case LIGHT_RED:
        if (in_state >= 5000u) { t->state = LIGHT_GREEN;  t->since = now; }
        break;
    case LIGHT_GREEN:
        if (in_state >= 4000u) { t->state = LIGHT_YELLOW; t->since = now; }
        break;
    case LIGHT_YELLOW:
        if (in_state >= 1000u) { t->state = LIGHT_RED;    t->since = now; }
        break;
    }
}
```

Things to notice in this pattern:
- **One `case` per state.** Each case checks only the transitions that
  leave that state.
- **The state lives in a struct**, so you can run several independent
  machines (one per traffic light, or one per button).
- **Time is passed in as a parameter** (`now`) instead of calling `millis()`
  inside. That means a test on your PC can feed in any times it likes,
  including times near the 32-bit wrap, with no hardware and no waiting.
  This is called **designing for testability**, and Exercise 4's tests rely
  on it.
- **It never blocks.** The main loop calls `traffic_update(&t, millis())`
  over and over, and the function returns immediately.

### Events vs. levels

Two kinds of information come out of a state machine:
- A **level** is a condition that's true for a while: "the light is green",
  "the button is held down".
- An **event** happens once, at a moment: "the light just turned green",
  "the button was just pressed".

They're useful for different things. "Toggle the LED once per press" needs
the **event**; "run the motor while the button is held" needs the **level**.
A good module gives you both, for example `traffic_update()` returning
an event while `traffic_is_green()` reports the level.

## 7. Button bounce

When you press a mechanical button, the metal contacts don't close
cleanly. They **bounce**, making and breaking contact several times over
about 1–10 ms (sometimes more on cheap buttons):

```
 what you did:      ────────┐_____________________
 what the pin sees: ────────┐_┌┐_┌─┐__┌┐__________
                            └─ bounces (≈ 5 ms) ─┘
```

The CPU checks the pin millions of times per second, so it sees each bounce
as a separate press. If KEY0 toggles an LED, one press might toggle it 4
times, leaving it in a random state.

The fix is **debouncing**: only accept a new level once it has been
**stable** for some time (say 20 ms). That's naturally a state machine:
the "not sure yet" situations become states of their own. Exercise 4's
header (`lib/debounce.h`) draws the diagram for you. Use the traffic light
code above as a model for how to structure it.

---

## Try it

Wrap-around, demonstrated with an 8-bit clock:

```c
#include <stdio.h>
#include <stdint.h>

int main(void)
{
    uint8_t start = 250, interval = 20;
    for (int t = 250; t < 280; t += 2) {
        uint8_t now = (uint8_t)t;                      /* simulated 8-bit clock */
        int bad  = now >= (uint8_t)(start + interval);
        int good = (uint8_t)(now - start) >= interval;
        printf("now=%3u  deadline-way=%d  subtraction-way=%d\n", now, bad, good);
    }
    return 0;
}
```

(With 8-bit values the `(uint8_t)` casts are needed, because integer promotion
(Lesson 3) would otherwise do the maths in 32-bit `int`. With `uint32_t`, as
`millis()` uses, no cast is needed.)

---

## Check yourself

1. Why is `delay_ms(500)` in the main loop a problem when you also want to
   read buttons?
2. A `uint32_t` millisecond counter: after how long does it wrap?
3. `start = 0xFFFFFFF0`, `now = 0x00000010`. What is `now - start`?
4. What's the difference between an event and a level? Give a button example of each.
5. Why does `traffic_update` take `now` as a parameter instead of calling `millis()`?

<details><summary>Answers</summary>

1. The CPU waits inside the delay and can't notice the button. It's blocking.
2. 2³² ms ≈ **49.7 days**.
3. **0x20** = 32 ms. The wrap cancels out.
4. Level = "the button is held down" (true for a while); event = "the button was just pressed" (happens once).
5. So the logic can be tested on a PC with any time values, including wrap-around, without hardware or waiting.
</details>
