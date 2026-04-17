# Mimo Bot

**Mimo Bot** is a compact DIY desktop robot based on the **ESP32-S3**, featuring an animated TFT display, touch input, sound output, battery power, and a 3D printed body.

The project is designed as a small interactive desk companion with expressive eyes, animated UI, alarm/clock features, and a clean futuristic look.

---

## Features

- Animated robot face and expressions
- 1.69" SPI TFT display
- Touch input with TTP223
- Sound output via MAX98357A
- Battery-powered portable design
- Charging via TP4056 Type-C
- DC-DC boost power stage
- 3D printed enclosure
- Expandable firmware structure

---

## Hardware

### Main components

- **ESP32-S3 Zero Type-C (4MB)** — main controller board
- **1.69" 240x280 SPI TFT display** — main screen for animations and UI
- **MAX98357A I2S mono amplifier** — audio output module
- **Small speaker** — for sound effects, notifications, and alarm
- **TP4056 Type-C charging module** — Li-Po charging board
- **DC-DC step-up converter (0.9–4.2V to 5V)** — power boost module
- **TTP223 touch sensor** — touch input
- **3.7V 1000mAh Li-Po battery (803040)** — portable power source
- **Power switch**
- **Wires, connectors, screws, and mounting hardware**
- **3D printed enclosure parts**

### Optional

- Battery voltage monitoring divider
- Prototype board or custom PCB
- Rubber feet / soft pads
- Additional sensors
- Wi-Fi based extensions

---

## Main functions

- Animated eyes and expressions
- Interactive visual states
- Clock mode
- Alarm mode
- Touch interaction
- Audio feedback
- Battery-powered operation

---

## Repository structure

```text
mimo-bot/
├─ firmware/
│  └─ mimo_bot/
│     ├─ mimo_bot.ino
│     ├─ display_manager.h
│     ├─ audio_manager.h
│     ├─ battery_manager.h
│     ├─ touch_manager.h
│     ├─ robot_brain.h
│     ├─ robot_types.h
│     └─ ...
├─ images/
│  ├─ cover.jpg
│  ├─ assembled.jpg
│  └─ wiring.png
├─ docs/
│  ├─ pinout.md
│  └─ assembly.md
├─ models/
│  └─ STL files or link to Thingiverse
├─ .gitignore
├─ LICENSE
└─ README.md
