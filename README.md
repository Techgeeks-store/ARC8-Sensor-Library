# ARC8 Sensor Library

The official Arduino library for the **ARC8**, an 8-channel analog IR reflectance sensor array by TECHGEEKS.

Built for line following, maze solving, and edge or sumo-ring detection. It handles the fiddly parts for you: calibrating each sensor, working out exactly where the line is, and turning that into a steering number for your motors.

---

## Contents

- [What the sensor does](#what-the-sensor-does)
- [Which boards work](#which-boards-work)
- [Power and mounting](#power-and-mounting)
- [Installing the library](#installing-the-library)
- [Wiring](#wiring)
- [Your first sketch](#your-first-sketch)
- [How it all fits together](#how-it-all-fits-together)
- [Full function reference](#full-function-reference)
- [Example sketches](#example-sketches)
- [Warning messages and what to do about them](#warning-messages-and-what-to-do-about-them)
- [Tuning the PID](#tuning-the-pid)
- [Things to know](#things-to-know)
- [License](#license)

---

## What the sensor does

The ARC8 has 8 infrared sensors in a row. Each one shines invisible light at the floor and measures how much bounces back.

Each sensor gives you a number from **0 to 1023**:

| Reading | What it means |
|---|---|
| **High** (e.g. 900) | Dark surface. Black tape soaks up the light, so little comes back. |
| **Low** (e.g. 100) | Light surface. White floor bounces the light straight back. |

That is the one rule everything else is built on: **bigger number means blacker**.

---

## Which boards work

| Board | Works? | Why |
|---|---|---|
| **Arduino Nano** | Yes — recommended | Has 8 analog pins (A0–A7), exactly what you need |
| **Arduino Pro Mini** | Yes | Also has A0–A7 |
| **Arduino Uno** | Not with all 8 | The Uno only brings out A0–A5, so 2 sensors have nowhere to go |
| **Mega, ESP32, STM32, etc.** | Should work, not yet tested | See the note below |

**About other boards:** the library is written in plain, portable C++. There is no direct register poking and no AVR-only headers, so nothing stops it running on an ESP32 or similar. That said, it has only been built and verified for AVR so far, so treat other boards as untested rather than guaranteed. If you try one, do check that your board's `analogRead()` also returns 0–1023 — some return 0–4095, which would throw the calibration off.

---

## Power and mounting

| | |
|---|---|
| Input voltage | 3.3V to 5V DC |
| Input current | about 150 mA |
| Weight | 3 g |
| Working height | 3 to 10 mm above the track |
| Mounting | 2 × M2 screw holes |

150 mA is more than a lot of sensors draw, so give the ARC-8 a proper 5V supply rather than running it off a pin. An MP1584 or LM7805 regulator works well. If you are using the Blueprint 01 Robot Controller Board, its onboard LM7805 already handles this and the sensor rail gives you a 5V pin to plug into.

Mount the board with the two M2 screw holes, and set the height anywhere between 3 and 10 mm. Which height works best depends on your track, so try a few and watch the readings.

There is an indicator LED at each end of the board. Both light up steadily when the sensor has power.

---

## Installing the library

Pick whichever of these suits you. The first is easiest.

### Option 1 — Add .ZIP Library (recommended)

1. Download **`ARC8.zip`** from the [Releases page](https://github.com/Techgeeks-store/ARC8-Sensor-Library/releases).
2. Open the Arduino IDE.
3. Go to **Sketch → Include Library → Add .ZIP Library…**
4. Select the `ARC8.zip` file you downloaded.
5. You should see *"Library added to your libraries"* at the bottom.

Restart the IDE, and the examples appear under **File → Examples → ARC8**.

### Option 2 — Download the whole repository

1. Click the green **Code** button at the top of this page, then **Download ZIP**.
2. Unzip it. You will get a folder called **`ARC8-Sensor-Library-main`**.
3. **Rename that folder to just `ARC8`.** This step matters — the Arduino IDE wants the folder name to match the library name, and it does not like the dashes.
4. Move the `ARC8` folder into your sketchbook's `libraries` folder:
   - **Windows:** `Documents\Arduino\libraries\`
   - **macOS:** `~/Documents/Arduino/libraries/`
   - **Linux:** `~/Arduino/libraries/`
5. Restart the Arduino IDE.

### Option 3 — Git clone

```bash
git clone https://github.com/Techgeeks-store/ARC8-Sensor-Library.git ARC8
```

Run that inside your `Arduino/libraries` folder. Note the `ARC8` on the end — it clones into a correctly named folder for you.

### Checking it worked

Open **File → Examples → ARC8 → BasicRead**. If that sketch appears and compiles, you are ready.

---

## Wiring

Connect the 8 signal outputs to your Nano's analog pins, plus power and ground:

| ARC8 pin | Arduino Nano | Sensor number **in code** |
|---|---|---|
| Sensor 1 signal | A0 | `0` |
| Sensor 2 signal | A1 | `1` |
| Sensor 3 signal | A2 | `2` |
| Sensor 4 signal | A3 | `3` |
| Sensor 5 signal | A4 | `4` |
| Sensor 6 signal | A5 | `5` |
| Sensor 7 signal | A6 | `6` |
| Sensor 8 signal | A7 | `7` |
| VCC | 5V | — |
| GND | GND | — |

> Watch the numbering: your board may label the sensors **1 to 8**, but in code they are always **0 to 7**. Asking for sensor `8` gets you `ARC8: bad sensor index`.
>
> **The numbering starts from the right.** Sensor `0` is the right-hand end of the array and sensor `7` is the left-hand end. Wire them to A0–A7 in that order and the default weights line up correctly.

> Check the labels printed on your own ARC8 board before wiring — always trust the silkscreen over a table in a README.

You are not locked into A0–A7. Any analog-capable pins work; just list them in the same order you want the sensors numbered:

```cpp
int pins[8] = {A0, A1, A2, A3, A4, A5, A6, A7};
```

Sensor `0` in the library is whatever pin you put first.

---

## Your first sketch

```cpp
#include <ARC8.h>

ARC8 sensors;

int pins[8] = {A0, A1, A2, A3, A4, A5, A6, A7};
int values[8];

void setup() {
  Serial.begin(9600);      // you must do this yourself - the library never does
  sensors.begin(pins);
}

void loop() {
  sensors.read(values);    // read all 8 sensors
  sensors.printRaw();      // print them
  delay(200);
}
```

Open **Tools → Serial Monitor**, set it to **9600**, and slide a piece of black tape under the bar. Watch the numbers jump up under the tape.

### The one rule to remember

**`read(values)` comes first, every time round the loop.**

Almost every other function works from the batch of numbers that `read()` collected. Call them without reading first and you get `ARC8: call read() first` in the Serial Monitor and a safe `0` back, rather than nonsense.

```cpp
void loop() {
  sensors.read(values);     // <- always this first
  int error = sensors.getError();
  // ...
}
```

---

## How it all fits together

### 1. Calibrating

Every sensor is slightly different. One might read 120 on your white floor and 800 on the tape, while the sensor beside it reads 200 and 950. Compare those numbers directly and the brighter sensor always looks closer to the line — so the robot drifts towards it forever.

Calibrating fixes that. You sweep the bar across the line for a couple of seconds while `calibrate()` runs, and it remembers the lightest and darkest each sensor saw. After that, every sensor is stretched onto the **same 0 to 1000 scale**, where 0 is that sensor's own white and 1000 is its own black.

```cpp
unsigned long start = millis();
while (millis() - start < 3000) {   // 3 seconds
  sensors.read(values);
  sensors.calibrate();              // call it over and over while you sweep
}
```

Sweep so that **every** sensor sees both the floor and the tape. A sensor that only ever saw white cannot be calibrated, and the library will tell you so.

### 2. On the line, or not

Once calibrated, anything above the **threshold** (default 500, halfway up the scale) counts as being on the line:

```cpp
sensors.setLine(BLACK);       // black tape on a white floor (the default)
sensors.setThreshold(500);

bool onTheLine[8];
sensors.readDigital(onTheLine);      // true / false for each sensor
int howMany = sensors.countOnLine(); // e.g. 2
```

Got a white line on a dark floor instead? `sensors.setLine(WHITE);` flips the comparison. You can change it at any time, even mid-loop, and it takes effect on the very next read.

> **`readDigital()` is not `digitalRead()`.** The ARC-8 can be read with `digitalRead()` on the pin, but the manual warns that anywhere between about 1.5V and 3V the output is unpredictable, so on a poor track the value flickers. This library never does that. `readDigital()` takes the full analog reading, stretches it with your calibration, then compares it to the threshold. You get a steady yes or no even where `digitalRead()` would jump around.

### 3. Where is the line?

`readLine()` gives you a single number for the line's position. Each sensor has a **weight** saying where it sits on the bar:

```
sensor:   0    1    2    3    4    5    6    7
weight:   4    3    2    1   -1   -2   -3   -4
         <------- right    left -------->
```

**Sensor 0 is the one on the right and sensor 7 is on the left.** That is how the array is numbered, so the right-hand sensors carry the positive numbers and a positive position means the line went right.

The position is a weighted average of the sensors that can see the line. Because a sensor half-over the tape reads lower than one sitting right on it, sensors that barely touch the line only tug the answer a little. That is what makes the number slide smoothly as you drift, instead of jumping from sensor to sensor.

| `readLine()` returns | Meaning |
|---|---|
| about `-4000` | line is hard over to the **left** (sensor 7 end) |
| `0` | line is **dead centre** |
| about `+4000` | line is hard over to the **right** (sensor 0 end) |

`getError()` is the same thing measured from the centre point. With the default weights the centre is 0, so `getError()` and `readLine()` give the same answer — but if you set your own weights, `getError()` is the one that stays honest.

**If every sensor loses the line,** the position is left at the last value it was sure about. So the robot keeps turning the way it was already turning, which is usually what finds the line again. Use `lineFound()` to check whether you are actually still on it.

### 4. Steering

`getCorrection()` turns the error into one steering number using PID:

```cpp
sensors.setPID(0.03, 0.0, 0.02);   // kp, ki, kd

void loop() {
  sensors.read(values);
  int correction = sensors.getCorrection();

  int leftSpeed  = BASE_SPEED + correction;
  int rightSpeed = BASE_SPEED - correction;
  // send those to your motors
}
```

Positive means the line has drifted right, so speed the left wheel up. Negative means the opposite.

---

## Full function reference

### Setting up

| Function | What it does |
|---|---|
| `ARC8()` | Creates the object with sensible defaults. |
| `void begin(int pins[8])` | Tells the library which 8 pins your sensors are on. Warns if you list the same pin twice. |

### Reading

| Function | What it does |
|---|---|
| `void read(int values[8])` | Reads all 8 sensors into your array, and remembers them for every other function. **Call this first.** |
| `int read(int i)` | Reads sensor `i` right now and returns 0–1023. Independent of `read(values)`. Returns `-1` for a bad index. |

### Calibration

| Function | What it does |
|---|---|
| `void calibrate()` | Updates the lightest/darkest seen so far. Call repeatedly while sweeping. |
| `void readCalibrated(int values[8])` | Fills your array with the 0–1000 stretched values. |
| `int readCalibrated(int i)` | Reads sensor `i` right now, stretched to 0–1000. Returns `-1` for a bad index. |
| `int getMin(int i)` | The lightest reading sensor `i` has seen. `-1` for a bad index. |
| `int getMax(int i)` | The darkest reading sensor `i` has seen. `-1` for a bad index. |

### On the line or not

| Function | What it does |
|---|---|
| `void setThreshold(int value)` | Sets the dividing number, 0–1000. Default 500. Anything outside gets pulled into range. |
| `void readDigital(bool values[8])` | Fills your array with `true`/`false` per sensor. |
| `bool onLine(int i)` | Checks sensor `i` right now. `false` for a bad index. |
| `int countOnLine()` | How many sensors can see the line, 0–8. |

### Line position

| Function | What it does |
|---|---|
| `void setLine(int mode)` | `BLACK` or `WHITE`. Anything else is ignored and warns. Takes effect immediately. |
| `void setWeights(int w[8])` | Your own weights. Also recalculates the centre point for `getError()`. **Keep each weight between −32 and +32** — see below. |
| `int readLine()` | Where the line is. Roughly −4000 to +4000 with default weights. Positive means the line went right. |
| `int getError()` | How far off centre, which is what you steer on. |
| `bool lineFound()` | `true` if at least one sensor can see the line. |

### Steering

| Function | What it does |
|---|---|
| `void setPID(float kp, float ki, float kd)` | Sets the three steering numbers. |
| `int getCorrection()` | The steering number. Positive = line drifted right. |

### Printing (for debugging)

| Function | What it does |
|---|---|
| `void printRaw()` | Prints the 8 raw readings, space separated. |
| `void printCalibrated()` | Prints the 8 stretched 0–1000 readings. |
| `void printLine()` | Prints `line: <position> error: <error>`. |

> Because sensor `0` is the right-hand end, `printRaw()` and `printCalibrated()` print the array **right to left**. Slide tape under the right-hand end and the first number on screen is the one that moves.
>
> The print functions and every warning use `Serial`, but the library **never** calls `Serial.begin()` for you. Do that yourself in `setup()`, or you will see nothing at all.
>
> `printLine()` shows the last position that was worked out — it does not recalculate. Call `readLine()` or `getError()` first if you want it fresh.

### Constants

| Name | Value | Used with |
|---|---|---|
| `BLACK` | 1 | `setLine(BLACK)` — dark line on a light floor |
| `WHITE` | 0 | `setLine(WHITE)` — light line on a dark floor |

---

## Example sketches

All under **File → Examples → ARC8**.

| Example | What it teaches |
|---|---|
| **BasicRead** | Reading all 8 sensors and printing them. Start here. |
| **Calibration** | Sweeping to calibrate, then reading on the tidy 0–1000 scale. |
| **DigitalRead** | Turning readings into simple yes/no, and counting sensors on the line. |
| **LineFollowerPID** | A complete line follower with motor driver code. |
| **CustomWeights** | Changing the weights and seeing how the position number changes. |

---

## Warning messages and what to do about them

If something is wrong, the library says so in the Serial Monitor in plain English.

**Each message prints only once per run.** Without that, a single wiring mistake would fill your Serial Monitor thousands of times a second and hide everything useful.

| Message | What went wrong | How to fix it |
|---|---|---|
| `ARC8: call read() first` | You used a function that needs sensor data, but `read(values)` has never run. | Put `sensors.read(values);` at the top of your `loop()`. |
| `ARC8: pin used twice` | Two entries in your pins array are the same. | Check for a typo in `int pins[8] = {...}`. |
| `ARC8: bad sensor index` | You asked for a sensor that does not exist. | Sensors are numbered **0 to 7**, not 1 to 8. |
| `ARC8: not calibrated yet` | You used a calibrated reading before ever calling `calibrate()`. | Run the calibration sweep in `setup()`. |
| `ARC8: sensor not calibrated properly` | One sensor's light and dark readings are almost the same. | That sensor never saw both floor and tape — sweep wider. Or it may be unplugged, or too far from the ground. |
| `ARC8: threshold out of range` | `setThreshold()` got a value outside 0–1000. | The scale is 0–1000, not 0–1023. Try 500. |
| `ARC8: invalid line mode` | `setLine()` got something other than `BLACK` or `WHITE`. | Use the words `BLACK` or `WHITE`. |

### Something is wrong but there is no message

Not every mistake can be detected, so here are the usual suspects:

| What you see | Almost certainly |
|---|---|
| **All 8 numbers are identical** and change together | You forgot `sensors.begin(pins);` in `setup()`. Without it the library reads the same pin 8 times. |
| **Nothing at all in the Serial Monitor** | No `Serial.begin(9600);` in `setup()`, or the Serial Monitor's speed box is not set to the same number. |
| **All numbers stuck at 0, or stuck at 1023** | Power or wiring. Check VCC and GND, and that the signal wires reach the pins you listed. |
| **The indicator LEDs at the ends are off, or flickering** | Off means no power at all. Flickering means the supply is not steady: loose jumper wires, a low battery, or too little current. Solder the wires to the terminals and put a capacitor near the 5V supply. |
| **You think one sensor has died** | Point a phone camera at the board and look at the screen. Every sensor should show a faint purple dot. Infrared is invisible to your eye but the camera picks it up, so a sensor with no dot is not getting power. |
| **Numbers barely change** between tape and floor | The bar is at the wrong height. It works between 3 and 10 mm off the track. Move it up or down until the gap between white and black readings is widest. |
| **Position jumps around wildly** | You have not calibrated, or the sweep missed some sensors. Run the calibration sweep again, slower and wider. |
| **The robot steers the wrong way** | Your motors are mirrored. Swap the `+` and `−` in the two speed lines: `BASE_SPEED - correction` and `BASE_SPEED + correction`. |
| **It follows a white line badly** | You left it on the default. Add `sensors.setLine(WHITE);` in `setup()`. |

---

## Tuning the PID

Do this in order, and change **one number at a time**.

1. **Start with `setPID(0.03, 0.0, 0.0)`** — only `kp`, the other two at zero.
2. **Raise `kp`** until the robot follows the line. Too low and it drifts off corners; too high and it snakes violently from side to side.
3. **Back `kp` off slightly**, until it follows with only a gentle wobble.
4. **Add a little `kd`**, maybe `0.01` to `0.05`. This damps the wobble. Raise it until the robot runs smooth.
5. **Leave `ki` at 0.** You only need it if the robot consistently sits a little to one side of the line, and even then keep it tiny — something like `0.0001`.

| Symptom | Try |
|---|---|
| Drifts off on corners | Raise `kp` |
| Snakes wildly side to side | Lower `kp`, or raise `kd` |
| Wobbles gently but constantly | Raise `kd` |
| Always rides slightly off to one side | A tiny `ki` |
| Overshoots then swings back hard | Lower `ki` (or set it to 0) |

Faster robots need lower `kp`. If you raise your base speed, expect to retune.

---

## Things to know

- **Always exactly 8 sensors.** There is no sensor-count setting and no support for other array sizes.
- **Keep custom weights between −32 and +32.** The position is worked out as `weight × reading`, readings reach 1000, and the answer is stored in an `int` — which on an Uno or Nano tops out at 32767. A weight of 33 or more can tip it over and make the number flip sign, sending the robot the wrong way. The defaults (−4 to +4) and the CustomWeights example (−7 to +7) are well inside the limit. There is nothing wrong with small weights: what matters is the *ratio* between them, not their size.
- **`BLACK` and `WHITE` are global names.** Many display libraries (Adafruit GFX, most TFT and OLED libraries) define their own `BLACK` and `WHITE`. Using one of those alongside ARC8 will produce a "redefined" warning from the compiler.
- **Calibration only narrows.** The remembered light/dark values can only get further apart, never closer. If you move the robot to a completely different surface, restart the sketch before recalibrating.
- **The PID never resets.** The running total behind `ki` and the previous error both carry on for the whole time the sketch runs. There is no reset function.
- **No EEPROM.** Calibration is forgotten at power-off, so you calibrate each time you switch on. That is worth doing anyway. Room lighting does not bother the ARC-8 much, but the track surface, the sensor height and the battery voltage all change between sessions, and those do move the readings.
- **No memory tricks.** No `malloc`, no `new`, no `String`. Everything is fixed size, so it cannot fragment your RAM mid-race.
- **No `delay()` anywhere inside the library.** Nothing here will ever stall your loop.

### Memory use

Measured on an Arduino Nano, compiled with the real Arduino AVR core:

| Example | Flash | RAM |
|---|---|---|
| BasicRead | 3,264 bytes (10.0%) | 334 bytes (16.3%) |
| Calibration | 4,306 bytes (13.1%) | 334 bytes (16.3%) |
| DigitalRead | 4,388 bytes (13.4%) | 342 bytes (16.7%) |
| CustomWeights | 4,572 bytes (14.0%) | 350 bytes (17.1%) |
| LineFollowerPID | 5,440 bytes (16.6%) | 334 bytes (16.3%) |

Plenty of room left for the rest of your robot.

---

## License

MIT — see [LICENSE](LICENSE). Use it, change it, build products with it. Just keep the copyright notice.

Copyright © 2026 TECHGEEKS
