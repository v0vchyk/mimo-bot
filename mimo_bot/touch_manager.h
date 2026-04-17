#pragma once

#include "robot_types.h"
#include "event_queue.h"

struct TouchManager {
  bool lastRaw = false;
  bool pressed = false;

  bool holdStarted = false;
  bool holdLevel2Sent = false;
  bool holdLevel3Sent = false;
  bool veryLongHoldSent = false;

  uint32_t pressStartedAt = 0;
  uint32_t lastTapAt = 0;
  uint8_t tapCount = 0;

  bool readRaw() const {
    bool state = digitalRead(Pins::TOUCH);
    return Config::TOUCH_INVERTED ? !state : state;
  }

  void begin() {
    pinMode(Pins::TOUCH, INPUT);
  }

  bool emit(EventQueue& queue, RobotEvent event) {
    if (event == RobotEvent::NONE) return true;

    if (!queue.push(event)) {
      Serial.println("[TOUCH] event queue overflow");
      return false;
    }

    Serial.print("[TOUCH EVENT] ");
    Serial.println(eventName(event));
    return true;
  }

  void update(EventQueue& queue, uint32_t now) {
    bool raw = readRaw();

    if (raw && !lastRaw) {
      pressed = true;
      holdStarted = false;
      holdLevel2Sent = false;
      holdLevel3Sent = false;
      veryLongHoldSent = false;
      pressStartedAt = now;

      Serial.println("[TOUCH] pressed");
    }

    if (pressed && raw) {
      const uint32_t heldMs = now - pressStartedAt;

      if (!holdStarted && heldMs >= Config::HOLD_START_MS) {
        holdStarted = true;
        emit(queue, RobotEvent::HOLD_START);
        emit(queue, RobotEvent::HOLD_LEVEL_1);
      }

      if (holdStarted && !holdLevel2Sent && heldMs >= Config::HOLD_LEVEL2_MS) {
        holdLevel2Sent = true;
        emit(queue, RobotEvent::HOLD_LEVEL_2);
      }

      if (holdStarted && !holdLevel3Sent && heldMs >= Config::HOLD_LEVEL3_MS) {
        holdLevel3Sent = true;
        emit(queue, RobotEvent::HOLD_LEVEL_3);
      }

      if (holdStarted && !veryLongHoldSent && heldMs >= Config::HOLD_VERY_LONG_MS) {
        veryLongHoldSent = true;
        emit(queue, RobotEvent::HOLD_VERY_LONG);
      }
    }

    if (!raw && lastRaw) {
      const uint32_t heldMs = now - pressStartedAt;

      Serial.print("[TOUCH] released, ms=");
      Serial.println(heldMs);

      if (holdStarted) {
        emit(queue, RobotEvent::HOLD_RELEASE);
        tapCount = 0;
      } else {
        tapCount++;
        lastTapAt = now;

        Serial.print("[TOUCH] tapCount=");
        Serial.println(tapCount);
      }

      pressed = false;
    }

    if (!pressed && tapCount > 0 && (now - lastTapAt) > Config::TOUCH_MULTI_TAP_GAP_MS) {
      if (tapCount == 1) emit(queue, RobotEvent::TAP_SINGLE);
      else if (tapCount == 2) emit(queue, RobotEvent::TAP_DOUBLE);
      else if (tapCount == 3) emit(queue, RobotEvent::TAP_TRIPLE);
      else emit(queue, RobotEvent::TAP_RAPID);

      tapCount = 0;
    }

    lastRaw = raw;
  }
};