#pragma once

#include <Arduino.h>

namespace Pins {
  constexpr int TFT_SCK  = 12;
  constexpr int TFT_MOSI = 11;
  constexpr int TFT_CS   = 10;
  constexpr int TFT_DC   = 9;
  constexpr int TFT_RST  = 8;
  constexpr int TFT_BL   = 7;

  constexpr int TOUCH = 6;

  constexpr int I2S_BCLK = 1;
  constexpr int I2S_LRCK = 2;
  constexpr int I2S_DOUT = 3;

  constexpr int BAT_ADC = 4;
}

namespace Config {
  constexpr uint32_t SERIAL_BAUD = 115200;
  constexpr uint32_t FRAME_INTERVAL_MS = 45;

  constexpr bool TOUCH_INVERTED = false;
  constexpr uint32_t TOUCH_MULTI_TAP_GAP_MS = 280;

  constexpr uint32_t HOLD_START_MS  = 800;
  constexpr uint32_t HOLD_LEVEL2_MS = 1500;
  constexpr uint32_t HOLD_LEVEL3_MS = 2200;
  constexpr uint32_t HOLD_VERY_LONG_MS = 4200;

  constexpr uint32_t IDLE_TO_SLEEPY_MS = 18000;
  constexpr uint32_t TIRED_TO_SLEEPY_MS = 2200;
  constexpr uint32_t SLEEPY_TO_SLEEP_MS = 2500;
  constexpr uint32_t SLEEP_TO_CLOCK_MS = 8000;
  constexpr uint32_t WAKE_SCAN_MS = 1600;

  constexpr float ADC_REF = 3.3f;
  constexpr float ADC_MAX = 4095.0f;
  constexpr float BATTERY_DIVIDER_RATIO = 2.0f;
  constexpr float LOW_BATTERY_THRESHOLD = 3.45f;

  constexpr const char* WIFI_SSID = "";
  constexpr const char* WIFI_PASS = "";
  constexpr const char* TZ_INFO = "EET-2EEST,M3.5.0/3,M10.5.0/4";

  constexpr const char* NTP_1 = "pool.ntp.org";
  constexpr const char* NTP_2 = "time.google.com";
  constexpr const char* NTP_3 = "time.cloudflare.com";

  constexpr uint32_t WIFI_RETRY_INTERVAL_MS   = 15000;
  constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS  = 12000;
  constexpr uint32_t NTP_RESYNC_INTERVAL_MS   = 21600000; // 6 годин
}

enum class RobotMoodProfile : uint8_t {
  CALM,
  ACTIVE
};

enum class RobotState : uint8_t {
  BOOT,
  IDLE,
  HAPPY,
  SURPRISED,
  LOVE_TRANSITION,
  LOVE,
  ANGRY_1,
  ANGRY_2,
  ANGRY_3,
  HYPER,
  TIRED,
  SLEEPY,
  SLEEP,
  CLOCK,
  WAKE_SCAN,
  LOW_BATTERY,
  YAWN,
  OFFENDED,
  REBOOTING,
  ALARM_RINGING
};

enum class RobotEvent : uint8_t {
  NONE,
  BOOT_DONE,
  TAP_SINGLE,
  TAP_DOUBLE,
  TAP_TRIPLE,
  TAP_RAPID,
  HOLD_START,
  HOLD_LEVEL_1,
  HOLD_LEVEL_2,
  HOLD_LEVEL_3,
  HOLD_RELEASE,
  HOLD_VERY_LONG,
  IDLE_TIMEOUT,
  SLEEP_TIMEOUT,
  CLOCK_TIMEOUT,
  WAKEUP,
  LOW_BATTERY,
  BATTERY_OK,
  IDLE_LOOK_UP,
  IDLE_DOUBLE_BLINK,
  IDLE_YAWN,
  OVERSTIMULATED,
  REBOOT_REQUEST
};

inline const char* stateName(RobotState state) {
  switch (state) {
    case RobotState::BOOT: return "BOOT";
    case RobotState::IDLE: return "IDLE";
    case RobotState::HAPPY: return "HAPPY";
    case RobotState::SURPRISED: return "SURPRISED";
    case RobotState::LOVE_TRANSITION: return "LOVE_TRANSITION";
    case RobotState::LOVE: return "LOVE";
    case RobotState::ANGRY_1: return "ANGRY_1";
    case RobotState::ANGRY_2: return "ANGRY_2";
    case RobotState::ANGRY_3: return "ANGRY_3";
    case RobotState::HYPER: return "HYPER";
    case RobotState::TIRED: return "TIRED";
    case RobotState::SLEEPY: return "SLEEPY";
    case RobotState::SLEEP: return "SLEEP";
    case RobotState::CLOCK: return "CLOCK";
    case RobotState::WAKE_SCAN: return "WAKE_SCAN";
    case RobotState::LOW_BATTERY: return "LOW_BATTERY";
    case RobotState::YAWN: return "YAWN";
    case RobotState::OFFENDED: return "OFFENDED";
    case RobotState::REBOOTING: return "REBOOTING";
    case RobotState::ALARM_RINGING: return "ALARM_RINGING";
    default: return "UNKNOWN_STATE";
  }
}

inline const char* eventName(RobotEvent event) {
  switch (event) {
    case RobotEvent::NONE: return "NONE";
    case RobotEvent::BOOT_DONE: return "BOOT_DONE";
    case RobotEvent::TAP_SINGLE: return "TAP_SINGLE";
    case RobotEvent::TAP_DOUBLE: return "TAP_DOUBLE";
    case RobotEvent::TAP_TRIPLE: return "TAP_TRIPLE";
    case RobotEvent::TAP_RAPID: return "TAP_RAPID";
    case RobotEvent::HOLD_START: return "HOLD_START";
    case RobotEvent::HOLD_LEVEL_1: return "HOLD_LEVEL_1";
    case RobotEvent::HOLD_LEVEL_2: return "HOLD_LEVEL_2";
    case RobotEvent::HOLD_LEVEL_3: return "HOLD_LEVEL_3";
    case RobotEvent::HOLD_RELEASE: return "HOLD_RELEASE";
    case RobotEvent::HOLD_VERY_LONG: return "HOLD_VERY_LONG";
    case RobotEvent::IDLE_TIMEOUT: return "IDLE_TIMEOUT";
    case RobotEvent::SLEEP_TIMEOUT: return "SLEEP_TIMEOUT";
    case RobotEvent::CLOCK_TIMEOUT: return "CLOCK_TIMEOUT";
    case RobotEvent::WAKEUP: return "WAKEUP";
    case RobotEvent::LOW_BATTERY: return "LOW_BATTERY";
    case RobotEvent::BATTERY_OK: return "BATTERY_OK";
    case RobotEvent::IDLE_LOOK_UP: return "IDLE_LOOK_UP";
    case RobotEvent::IDLE_DOUBLE_BLINK: return "IDLE_DOUBLE_BLINK";
    case RobotEvent::IDLE_YAWN: return "IDLE_YAWN";
    case RobotEvent::OVERSTIMULATED: return "OVERSTIMULATED";
    case RobotEvent::REBOOT_REQUEST: return "REBOOT_REQUEST";
    default: return "UNKNOWN_EVENT";
  }
}
