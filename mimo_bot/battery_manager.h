#pragma once

#include "robot_types.h"
#include "event_queue.h"

struct BatteryManager {
  uint32_t lastSampleAt = 0;
  bool lowBatteryLatched = false;

  float voltage = 4.00f;
  int percent = 100;

  void begin() {
    analogReadResolution(12);
  }

  float readVoltageRaw() const {
    const int raw = analogRead(Pins::BAT_ADC);
    return (raw / Config::ADC_MAX) * Config::ADC_REF * Config::BATTERY_DIVIDER_RATIO;
  }

  int voltageToPercent(float v) const {
    if (v >= 4.20f) return 100;
    if (v <= 3.30f) return 0;
    return static_cast<int>(((v - 3.30f) * 100.0f) / (4.20f - 3.30f));
  }

  bool emit(EventQueue& queue, RobotEvent event) {
    if (event == RobotEvent::NONE) return true;

    if (!queue.push(event)) {
      Serial.println("[BAT] event queue overflow");
      return false;
    }

    Serial.print("[BAT EVENT] ");
    Serial.println(eventName(event));
    return true;
  }

  void update(EventQueue& queue, uint32_t now) {
    if (now - lastSampleAt < 3000) {
      return;
    }

    lastSampleAt = now;

    voltage = readVoltageRaw();
    percent = voltageToPercent(voltage);

    Serial.print("[BAT] ");
    Serial.print(voltage, 2);
    Serial.print(" V, ");
    Serial.print(percent);
    Serial.println("%");

    if (!lowBatteryLatched && voltage <= Config::LOW_BATTERY_THRESHOLD) {
      lowBatteryLatched = true;
      emit(queue, RobotEvent::LOW_BATTERY);
    } else if (lowBatteryLatched && voltage > (Config::LOW_BATTERY_THRESHOLD + 0.12f)) {
      lowBatteryLatched = false;
      emit(queue, RobotEvent::BATTERY_OK);
    }
  }
};