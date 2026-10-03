# CrunchLabs IR Turret: Fun Projects

| Sketch | What it does |
|---|---|
| `01_remote_explorer` | Shows the code for each remote button in the Serial Monitor. Upload this first. |
| `02_turret_fun_pack` | Aim and fire with the remote, plus 6 mini projects and 3 open slots for your own projects. |

## Getting started

1. Install the **IRremote** library (version 4.x). In the Arduino IDE, open
   **Tools → Manage Libraries**, search for `IRremote` by shirriff/z3t0/ArminJo,
   and click Install. `Servo` already comes with the Arduino IDE.
2. Choose **Board: Arduino Nano**. Under **Processor**, pick *ATmega328P (Old
   Bootloader)* if the upload fails with the normal option.
3. Open the sketch, plug in the USB cable, and click **Upload**.
4. Open the **Serial Monitor** at **9600 baud** to see messages and game scores.

## Fun Pack buttons

| Button | Action |
|---|---|
| Arrows | Aim (hold to keep moving) |
| OK / \* | Fire one dart / fire all darts |
| # | Go home and stop the current mode |
| 0 | "I reloaded": resets the dart count to 6 |
| 1 | **Dance Party**: the turret busts a move |
| 2 | **Patrol**: sweeps back and forth like a guard. OK fires. |
| 3 | **Roulette**: spins to a random spot, counts down, fires |
| 4 | **Simon Says**: copy the turret's moves with the arrows. Each round adds one more move. |
| 5 | **Quick Draw**: press OK as soon as the barrel dips. Your reaction time shows in the Serial Monitor. |
| 6 | **Rocket Countdown**: tilts up while counting 10 → 1, then fires all darts |
| 7, 8, 9 | **Your projects**: edit `myProject7()`, `myProject8()` and `myProject9()` |

## Calibrate first

Every turret is a little different. Change the **Tuning knobs** near the top of
the sketch:

- **Up arrow tilts down?** Set `PITCH_UP = -1`.
- **Turret creeps sideways when idle?** Adjust `YAW_STOP` a little at a time,
  for example to 89 or 91.
- **Barrel turns too far or not far enough for one dart?** Change
  `ROLL_ONE_DART_MS`.
- **Don't want a penalty dart when you lose a game?** Set `PENALTY_DART = false`.

## Writing your own project

All the projects are built from these building blocks:

| Block | What it does |
|---|---|
| `turnLeft(ms)` / `turnRight(ms)` | Turn for that many milliseconds (1000 ms = 1 second) |
| `tiltUp(deg)` / `tiltDown(deg)` | Tilt the barrel by that many degrees |
| `tiltTo(angle)` | Tilt to an exact angle (`PITCH_HOME` means straight ahead) |
| `fire()` / `fireAll()` | Shoot one dart / all darts |
| `nodYes()` / `shakeNo()` | Expressions |
| `goHome()` | Stop turning and look straight ahead |
| `waitForButton(ms)` | Waits for a button press and returns its code, or `NO_BUTTON` if time runs out |
| `readButton()` | Checks for a button press without waiting. Use it inside loops. |
| `random(a, b)` | A random number from `a` to `b - 1` |
| `delay(ms)` | Pause |
| `Serial.println("hi")` | Print a message to the Serial Monitor |

**Example: "Guard Dog".** The turret growls (shakes) 3 times, then waits for
you to say "sorry" (OK) within 3 seconds. If you don't, it fires.

```cpp
void myProject8() {
  for (int i = 0; i < 3; i++) shakeNo();
  Serial.println("Say sorry! (press OK)");
  if (waitForButton(3000) == BTN_OK) {
    nodYes();               // forgiven
  } else {
    fire();                 // too late!
  }
}
```

## Challenge ideas

1. **Secret code lock:** the turret only fires after you press `1 3 3 7`.
2. **Tic-tac-toe referee:** number keys 1–9 aim at 9 targets on a grid.
3. **Hot potato:** pass the remote around. Whoever is holding it when the random
   timer runs out gets a "shakeNo" (or a dart at their target).
4. **Record and replay:** save the arrow presses into an array, then press a
   number key to replay the whole path.
5. **Lazy patrol:** make the patrol stop and "look around" at random times.

**Safety:** aim at targets like cups, cardboard or paper, never at faces, eyes or pets.
