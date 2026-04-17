# Mimo Bot

![Mimo Bot](images/mimo-cover.jpg)

**Mimo Bot** is a compact DIY desktop robot built around the **ESP32-S3**, featuring an animated TFT display, touch input, sound output, battery power, and a 3D printed enclosure.

Designed as a small interactive desk companion, Mimo Bot combines embedded hardware, expressive UI animation, and character-driven behavior in a portable form factor.

---

## Overview

Mimo Bot is more than a simple electronics project — it is a small digital companion designed to feel alive on your desk.

The project combines:

- embedded development
- animated user interface design
- sound feedback
- battery-powered portability
- 3D printed mechanical design

Mimo Bot can display animated eyes and expressions, react to touch input, play sounds, and operate as a small clock/alarm style desktop robot.

---

## Highlights

- Animated robot face and expressions
- 1.69" SPI TFT display
- Touch input via TTP223
- Audio output through MAX98357A
- Battery-powered portable design
- TP4056 USB-C charging
- DC-DC boosted power stage
- 3D printed body and enclosure
- Modular firmware structure for future expansion

---

## Hardware

### Core components

- **ESP32-S3 Zero Type-C (4MB)**
- **1.69" 240x280 SPI TFT display**
- **MAX98357A I2S mono amplifier**
- **Small speaker**
- **TP4056 Type-C charging module**
- **DC-DC step-up converter (0.9–4.2V to 5V)**
- **TTP223 touch sensor**
- **3.7V 1000mAh Li-Po battery (803040)**
- **Power switch**
- **Wiring and mounting hardware**
- **3D printed enclosure parts**

### Optional components

- Battery voltage monitoring divider
- Prototype board or custom PCB
- Additional sensors
- Decorative lighting or LEDs
- Wi-Fi based feature extensions

---

## Features

- Animated eyes and visual states
- Touch interaction
- Sound feedback
- Clock mode
- Alarm mode
- Battery-powered operation
- Expandable firmware architecture

---

## Repository structure

```text
mimo-bot/
├─ mimo_bot/
│  ├─ mimo_bot.ino
│  ├─ alarm_manager.h
│  ├─ audio_manager.cpp
│  ├─ audio_manager.h
│  ├─ battery_manager.h
│  ├─ display_manager.h
│  ├─ event_queue.h
│  ├─ robot_brain.h
│  ├─ robot_types.h
│  ├─ settings_manager.h
│  ├─ touch_manager.h
│  └─ web_manager.h
├─ images/
│  └─ mimo-cover.jpg
├─ LICENSE
├─ .gitignore
└─ README.md
