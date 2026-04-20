# 🤖 Garbage Collector Robot — Arduino Project

An autonomous ground robot that navigates a fixed path using IR sensors to detect black tape checkpoints, pauses at each stop, and halts permanently at its final destination.

---

## 📦 Hardware Required

| Component | Quantity |
|---|---|
| Arduino Uno (or compatible) | 1 |
| L298N Motor Driver Module | 1 |
| IR Sensor Module | 2 |
| BO Motor (DC Gear Motor) | 2 |
| Robot Chassis | 1 |
| 9V / 12V Battery or Power Bank | 1 |
| Jumper Wires | As needed |
| Black Electrical Tape (for track) | 1 roll |

---

## 🔌 Wiring / Pin Connections

### IR Sensors → Arduino

| IR Sensor | Arduino Pin |
|---|---|
| Left IR — OUT | D2 |
| Right IR — OUT | D3 |
| Both IR — VCC | 5V |
| Both IR — GND | GND |

### L298N Motor Driver → Arduino

| L298N Pin | Arduino Pin | Purpose |
|---|---|---|
| ENA | D5 (PWM) | Left motor speed |
| IN1 | D6 | Left motor direction A |
| IN2 | D7 | Left motor direction B |
| ENB | D10 (PWM) | Right motor speed |
| IN3 | D8 | Right motor direction A |
| IN4 | D9 | Right motor direction B |
| GND | GND | Common ground |
| 12V | Battery +ve | Motor power supply |
| 5V (optional) | 5V | Logic power from Arduino |

### BO Motors → L298N

| Motor | L298N Terminal |
|---|---|
| Left Motor | OUT1 & OUT2 |
| Right Motor | OUT3 & OUT4 |

---

## 🗺️ How It Works

```
[START]
   ↓
Robot moves forward continuously
   ↓
Both IR sensors detect black tape?
   ├── NO  → Keep moving forward
   └── YES → Stop motors
                ↓
         Checkpoint count < 3?
         ├── YES → Wait 5 seconds → Resume → (loop)
         └── NO  → FINAL DESTINATION — halt forever
```

The robot uses **two IR sensors side by side** at the front. When **both sensors simultaneously detect a black bar**, it registers as a checkpoint or final stop. A single sensor hitting black (e.g. while turning slightly) is ignored.

### Checkpoint Logic

| Detection | Action |
|---|---|
| 1st black bar | Checkpoint 1 — pause 5s, resume |
| 2nd black bar | Checkpoint 2 — pause 5s, resume |
| 3rd black bar | Checkpoint 3 — pause 5s, resume |
| 4th black bar | Final destination — motors off permanently |

---

## ⚙️ Configurable Variables

Open the `.ino` file and edit these values at the top:

```cpp
const unsigned long CHECKPOINT_DELAY = 5000;  // Pause time in ms (5000 = 5 seconds)
const int TOTAL_CHECKPOINTS         = 3;      // Number of mid-route stops
const int MOTOR_SPEED               = 180;    // Motor speed: 0–255
```

---

## 🛠️ Troubleshooting

### Robot never detects black tape
- Check IR sensor indicator LEDs — they should light up when hovering over black surface
- Adjust the small blue potentiometer on the IR module to change sensitivity
- Try flipping the detection logic in `bothSensorsOnBlack()`:
  ```cpp
  // Change LOW to HIGH if your module outputs HIGH on black
  bool leftBlack = (digitalRead(IR_LEFT) == HIGH);
  ```

### One motor spins backwards
- Swap `IN1` and `IN2` (or `IN3` / `IN4`) for the affected side in `motorForward()` — no rewiring needed

### Robot triggers false checkpoints (too sensitive)
- Increase `CONFIRM_COUNT` inside `stableBlackDetection()` from 3 to 5
- Increase `CONFIRM_DELAY` from 10ms to 20ms

### Robot misses checkpoints (crosses tape without stopping)
- Slow down the robot by reducing `MOTOR_SPEED`
- Decrease `CONFIRM_COUNT` to 2

### Robot stops but doesn't resume after checkpoint
- Check that `TOTAL_CHECKPOINTS` matches the number of actual tape bars on your track (excluding the final one)

---

## 📁 File Structure

```
garbage-collector-robot/
├── garbage_collector.ino   ← Main Arduino sketch
└── README.md               ← This file
```

---

## 📸 Track Layout (from design diagram)

```
[Final Destination] ←──── [Checkpoint 3]
                                │
                                │
                          [Checkpoint 2]
                                │
                                │ (start here ↑)
                          [Checkpoint 1]
```

Place black electrical tape as solid horizontal bars across the robot's path at each checkpoint location.

---

## 📋 Quick Start

1. Wire all components as per the table above
2. Open `garbage_collector.ino` in the Arduino IDE
3. Adjust `CHECKPOINT_DELAY`, `TOTAL_CHECKPOINTS`, and `MOTOR_SPEED` if needed
4. Upload to your Arduino board
5. Place the robot at the start (before Checkpoint 1)
6. Power on — the robot starts moving after a 1-second settle delay
7. Open Serial Monitor at **9600 baud** to watch checkpoint logs

---

## 📄 License

MIT — free to use, modify, and share.
