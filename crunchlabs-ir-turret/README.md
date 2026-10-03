# CrunchLabs IR Turret: Fun Projects

| Sketch | What it does |
|---|---|
| `01_remote_explorer` | Shows the code for each remote button in the Serial Monitor. Upload this first. |
| `02_turret_fun_pack` | Aim and fire with the remote, plus 6 mini projects and 3 open slots for your own projects. |
| `03_servo_calibrator` | Type numbers in the Serial Monitor to find the best tuning values for your turret's servos. |

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

## The fun projects in detail

Each project below explains how to play, how the code works, and what you can
change. The numbers you can tweak are written exactly as they appear in
`02_turret_fun_pack.ino`, so you can search for them.

### Remote Explorer (`01_remote_explorer`)

**How to use it:** Upload it, open the Serial Monitor at 9600 baud, and press
buttons. Each press prints a line like `Button code: 0x1C`. Holding a button
down adds `(held down)`.

**How it works:** The IR receiver turns flashes of invisible light into a number
called the *command*. `IrReceiver.decode()` checks whether a new signal has
arrived, `decodedIRData.command` holds the number, and `IrReceiver.resume()`
gets the receiver ready for the next press.

**Try this:** Point a TV remote at the turret. Many TV remotes use the same NEC
signal type, so you can copy their codes into the `BTN_...` lines of the fun
pack and drive the turret with your TV remote.

---

### Manual control (arrows, OK, \*, #, 0)

**How to play:** Use the arrows to aim. Tap for small moves or hold to keep
moving. Press **OK** to fire one dart or **\*** to fire every dart that's left.
**#** brings the barrel back to straight ahead. After you push new darts in,
press **0** so the turret knows it has 6 again.

**How it works:**
- The **left/right** servo spins like a wheel and has no angle. The code turns
  it on for `YAW_STEP_MS` milliseconds and then stops it.
- The **up/down** servo moves to an exact angle. `tiltTo()` moves it one degree
  at a time so it moves smoothly, and it never goes past `PITCH_MIN` or
  `PITCH_MAX`.
- **Firing** spins the barrel just far enough for one dart (`ROLL_ONE_DART_MS`).
  The code counts darts in `dartsLeft`. When it reaches 0, the turret shakes
  its head "no" instead of firing.

**Tweak it:** `YAW_STEP_MS` and `PITCH_STEP` set how far each tap moves.
`YAW_SPEED` sets how fast it turns.

---

### 1. Dance Party

**How to play:** Press **1** and watch. This one is just for fun.

**What happens:** The turret wiggles left and right, nods "yes", wiggles the
other way, and shakes "no". It does that twice, then does a big spin move,
looks all the way up and down, and returns home.

**How it works:** `danceParty()` is a list of building blocks run in order:
`turnLeft()`, `turnRight()`, `nodYes()`, `shakeNo()` and `tiltTo()`. A `for`
loop repeats the first part twice.

**Tweak it:** Change the numbers (300 ms wiggles, the 1200 ms spin) or change the
order of the moves to make up your own dance. Try adding `fire()` at the end for
a confetti finish.

---

### 2. Patrol (sentry mode)

**How to play:** Press **2**. The turret sweeps slowly from side to side like a
guard. At the end of each sweep it nods, as if it's looking around, then turns
back. Press **OK** to fire at whatever it's facing. Press **#** to stop.

**Game idea:** One player has the remote. The others try to sneak past the
turret, and the player with the remote fires when someone is in front of the
barrel.

**How it works:** This project doesn't use `delay()` to time the sweep. It
checks the clock with `millis()` instead. The yaw servo keeps turning while
the `while (true)` loop keeps asking "was a button pressed?" and "has this sweep
lasted long enough?" That way the turret can react to OK in the middle of a
sweep. When it fires, it remembers how much of the sweep was already done
(`sweptSoFar`) so it doesn't drift off to one side.

**Tweak it:** `PATROL_SPEED` (how fast it sweeps) and `LEG_MS` (how long each
sweep lasts, 1500 = 1.5 seconds).

---

### 3. Roulette

**How to play:** Set up targets in a circle around the turret, each with
someone's name on it. Press **3**. The turret spins in a random direction for a
random time, tilts to a random height, counts down "3… 2… 1…" with little
dips, and fires. Whoever's target gets hit loses (or wins!).

**How it works:** `random(800, 2500)` picks a spin time between 0.8 and 2.5
seconds. `random(2)` flips a coin for left or right. `randomSeed(analogRead(A0))`
in `setup()` makes sure the random numbers are different every time you turn the
turret on.

**Tweak it:** Make the spin longer for more suspense, or change the `-25, 26`
tilt range. Use `random(-10, 11)` to keep it level.

---

### 4. Simon Says

**How to play:** Press **4** and watch the turret. It shows one move: up, down,
left or right. Copy it with the arrow keys. Each round adds one new move to the
end of the pattern, so it gets harder and harder. After each correct move, the
turret repeats your move back to you. It nods "yes" when you finish a round.

- You have **5 seconds** for each press.
- A wrong press or running out of time ends the game. The Serial Monitor shows
  which round you reached. The turret shakes "no" and fires a penalty dart,
  unless `PENALTY_DART = false`.
- Beat all **20 rounds** and the turret throws a dance party.
- Press **#** to quit.

**How it works:** The pattern is stored in an **array** called `simonMoves[]`,
which is a row of boxes that each hold one move. Each round picks a new random
move, stores it in the next box, plays the whole row back, and then checks your
presses against the row one by one. `clearButtons()` throws away presses you
made while the turret was still showing the pattern.

**Tweak it:** `SIMON_MAX` (number of rounds), the `5000` in `waitForButton(5000)`
(time per press), and the `25` / `250` in `showMove()` (how big each move is).

---

### 5. Quick Draw

**How to play:** Press **5** and hold the remote ready. The turret nods (that
means "Ready…"), then waits a random time. The moment the barrel **dips**,
press **OK** as fast as you can.

- Faster than **400 ms**: you win and the turret nods.
- Slower than that, or no press within 3 seconds: the turret wins and fires a
  penalty dart.
- Press OK *before* the dip: **false start!** The turret shakes "no".

Your reaction time shows in the Serial Monitor. Play with friends and keep score.

**How it works:** The random wait (`random(1500, 5000)`, so 1.5 to 5 seconds)
stops you from guessing when it will happen. The dip uses `pitchServo.write()`
directly instead of the smooth `tiltTo()`, so it happens all at once. The code
writes down the time of the dip with `millis()`, and your reaction time is the
time of your press minus the time of the dip. The remote itself takes about
70 ms to send a signal, so nobody can score below about 70 ms.

**Tweak it:** Change `400` to make winning easier or harder, or change the
`1500, 5000` wait range.

---

### 6. Rocket Countdown

**How to play:** Load all 6 darts and press **6**. The turret counts down from 10
in the Serial Monitor while tilting a little higher on every number. At
"LIFTOFF!" it fires every dart. Press **#** during the countdown to abort the
launch.

**How it works:** `map(count, 10, 1, startAngle, topAngle)` converts the
countdown number into an angle: 10 means the starting angle, 1 means the top,
and the numbers in between are spread evenly. Each number waits with
`waitForButton(700)` instead of `delay(700)`, so it can notice # and abort.

**Tweak it:** The `700` sets the time between numbers. Change `topAngle` to aim
higher or lower at launch.

---

### 7, 8, 9. Your own projects

These are empty slots for you to fill in:

| Button | Function | What it does now |
|---|---|---|
| 7 | `myProject7()` | Looks left, looks right, then fires |
| 8 | `myProject8()` | Nods "yes" |
| 9 | `myProject9()` | Shakes "no" |

Replace the code inside the `{ }` with your own project using the building blocks
below. Ideas are in [Challenge ideas](#challenge-ideas).

## Calibrate first

Every turret is a little different. Change the **Tuning knobs** near the top of
the sketch:

- **Up arrow tilts down?** Set `PITCH_UP = -1`.
- **Turret creeps sideways when idle?** Adjust `YAW_STOP` a little at a time,
  for example to 89 or 91.
- **Barrel turns too far or not far enough for one dart?** Change
  `ROLL_ONE_DART_MS`.
- **Don't want a penalty dart when you lose a game?** Set `PENALTY_DART = false`.

### Using the servo calibrator (`03_servo_calibrator`)

Instead of guessing, measure the numbers. Upload `03_servo_calibrator`, open
the Serial Monitor at 9600 baud, and set the line ending (bottom of the Serial
Monitor) to **Newline**. Type a command and press Enter:

| Command | What it does |
|---|---|
| `y`, `p` or `r` | Choose a servo: **y**aw (left/right), **p**itch (up/down) or **r**oll (barrel) |
| a number, like `90` | Send that value to the chosen servo |
| `t` + a time, like `t158` | Spin the chosen yaw or roll servo at full speed for that many milliseconds, then stop |
| `?` | Show the help again |

**Find `YAW_STOP`:** Type `y`, then try `90`. If the turret creeps, try `89`,
`91`, `88`, `92` and so on until it stays perfectly still. Put that number in
`YAW_STOP` in the fun pack.

**Find `ROLL_STOP`:** Do the same after typing `r`, watching the barrel.

**Find `ROLL_ONE_DART_MS`:** **Take the darts out first.** Type `r`, then your
stop number, then `t158`. The barrel should turn exactly one chamber (1/6 of a
turn). If it goes too far, try a smaller number like `t150`. If it doesn't go
far enough, try `t165`. Put the best time in `ROLL_ONE_DART_MS`.

**Check pitch limits:** Type `p`, then try angles like `20` or `160` to see
how far the barrel can tilt before it hits something. Use those for `PITCH_MIN`
and `PITCH_MAX`. The calibrator never goes below 10 or above 175.

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
