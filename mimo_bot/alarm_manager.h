#pragma once

#include <Arduino.h>
#include <time.h>
#include "settings_manager.h"

class AlarmManager {
public:
  void begin(SettingsManager* settings) {
    this->settings = settings;
  }

  void update(uint32_t now) {
    if (!settings || !settings->data().alarmEnabled) {
      stop();
      return;
    }

    time_t ts = time(nullptr);
    if (ts < 100000) {
      stop();
      return;
    }

    const uint32_t minuteKey = (uint32_t)(ts / 60);

    if (snoozeUntilEpoch > 100000) {
      if (ts < snoozeUntilEpoch) {
        return;
      }
      snoozeUntilEpoch = 0;
      active = true;
      ringing = true;
      activeMinuteKey = minuteKey;
      ringingStartedAt = now;
      lastTriggeredMinuteKey = minuteKey;
      Serial.println("[ALARM] snooze trigger");
      return;
    }

    struct tm info;
    localtime_r(&ts, &info);

    const uint8_t weekdayBit = 1 << info.tm_wday;
    if ((settings->data().alarmDaysMask & weekdayBit) == 0) {
      return;
    }

    if (info.tm_hour == settings->data().alarmHour &&
        info.tm_min == settings->data().alarmMinute &&
        minuteKey != lastTriggeredMinuteKey) {
      active = true;
      ringing = true;
      activeMinuteKey = minuteKey;
      ringingStartedAt = now;
      lastTriggeredMinuteKey = minuteKey;
      Serial.printf("[ALARM] trigger at %02d:%02d\n", info.tm_hour, info.tm_min);
    }
  }

  void stop() {
    active = false;
    ringing = false;
    activeMinuteKey = 0;
    ringingStartedAt = 0;
  }

  void snooze() {
    if (!settings) {
      stop();
      return;
    }

    time_t ts = time(nullptr);
    if (ts > 100000) {
      uint8_t snoozeMin = settings->data().snoozeMinutes;
      if (snoozeMin == 0) snoozeMin = 5;
      snoozeUntilEpoch = ts + (time_t)snoozeMin * 60;
      Serial.printf("[ALARM] snooze until %lu\n", (unsigned long)snoozeUntilEpoch);
    }
    stop();
  }

  bool isActive() const { return active; }
  bool isRinging() const { return ringing; }
  uint32_t ringingFor(uint32_t now) const { return ringingStartedAt ? (now - ringingStartedAt) : 0; }
  time_t snoozeUntil() const { return snoozeUntilEpoch; }

private:
  SettingsManager* settings = nullptr;
  bool active = false;
  bool ringing = false;
  uint32_t activeMinuteKey = 0;
  uint32_t lastTriggeredMinuteKey = 0;
  uint32_t ringingStartedAt = 0;
  time_t snoozeUntilEpoch = 0;
};
