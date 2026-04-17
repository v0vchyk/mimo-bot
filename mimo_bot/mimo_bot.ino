#include "robot_types.h"

#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif

#include <time.h>
#include <sys/time.h>

#include "event_queue.h"
#include "touch_manager.h"
#include "battery_manager.h"
#include "audio_manager.h"
#include "display_manager.h"
#include "robot_brain.h"
#include "settings_manager.h"
#include "web_manager.h"
#include "alarm_manager.h"

EventQueue gEventQueue;
TouchManager gTouch;
BatteryManager gBattery;
AudioManager gAudio;
DisplayManager gDisplay;
RobotBrain gBrain;
SettingsManager gSettings;
WebManager gWeb;
AlarmManager gAlarm;

uint32_t gLastWifiAttemptAt = 0;
uint32_t gLastNtpSyncCheckAt = 0;
uint32_t gWifiConnectStartedAt = 0;
bool gTimeSynced = false;
bool gWebStarted = false;
bool gApMode = false;
bool gAlarmSoundLatched = false;
bool gWifiConnecting = false;

float normalFxVolumeFromSettings() {
  // Базова “жива” гучність для емоцій Mimo.
  // Не робимо її напряму рівною alarmVolume, щоб звичайні реакції не були занадто гучними.
  const float alarmNorm = constrain(gSettings.data().alarmVolume, 0, 100) / 100.0f;
  return 0.22f + alarmNorm * 0.12f;   // ~0.22 .. 0.34
}

float alarmVolumeFromSettings() {
  // Для будильника даємо ширший і гучніший діапазон.
  const float alarmNorm = constrain(gSettings.data().alarmVolume, 0, 100) / 100.0f;
  return 0.30f + alarmNorm * 0.35f;   // ~0.30 .. 0.65
}

void applyNormalAudioProfile() {
  gAudio.setMasterVolume(normalFxVolumeFromSettings());
}

void applyAlarmAudioProfile() {
  gAudio.setMasterVolume(alarmVolumeFromSettings());
}

static constexpr const char* AP_SSID = "Mimo-Setup";
static constexpr const char* AP_PASS = "mimo1234";
static constexpr const char* NTP_1 = "pool.ntp.org";
static constexpr const char* NTP_2 = "time.google.com";
static constexpr const char* NTP_3 = "time.cloudflare.com";
static constexpr uint32_t WIFI_RETRY_INTERVAL_MS = 15000;
static constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 12000;
static constexpr uint32_t NTP_RESYNC_INTERVAL_MS = 21600000UL; // 6h

bool wifiConfigured() {
  return gSettings.data().wifiConfigured && gSettings.data().wifiSsid[0] != '\0';
}

void syncDisplayClockFromSystem(bool markAsNtpSynced) {
  time_t nowEpoch = time(nullptr);
  if (nowEpoch > 100000) {
    if (markAsNtpSynced && !gTimeSynced) {
      Serial.println("[NTP] time synchronized");
    }
    gTimeSynced = markAsNtpSynced;
    gDisplay.setClockFromEpoch(nowEpoch);
    gDisplay.setClockSynced(markAsNtpSynced);
  }
}

void applyManualEpoch(time_t epoch) {
  if (epoch <= 100000) return;

  struct timeval tv;
  tv.tv_sec = epoch;
  tv.tv_usec = 0;
  settimeofday(&tv, nullptr);

  gTimeSynced = false;
  gDisplay.setClockFromEpoch(epoch);
  gDisplay.setClockSynced(false);
  Serial.printf("[CLOCK] manual epoch applied: %lu\n", (unsigned long)epoch);
}

void startNtp() {
#if defined(ESP32) || defined(ESP8266)
  Serial.println("[NTP] configuring time");
  configTzTime(gSettings.data().timezone, NTP_1, NTP_2, NTP_3);
#endif
}

void ensureWebStarted() {
#if defined(ESP32) || defined(ESP8266)
  if (gWebStarted) return;

  const wifi_mode_t mode = WiFi.getMode();
  if (mode == WIFI_MODE_NULL) {
    Serial.println("[WEB] defer start until Wi-Fi stack is initialized");
    return;
  }
#endif

  if (gWeb.begin(&gSettings, &gBattery, &gAudio, &gDisplay, &gTimeSynced)) {
    gWeb.setManualTimeSetter(applyManualEpoch);
    gWeb.setNtpSyncTrigger(startNtp);
    gWeb.setWifiApplyTrigger([]() {
      gLastWifiAttemptAt = 0;
      gLastNtpSyncCheckAt = 0;
      gWifiConnectStartedAt = 0;
      gWifiConnecting = false;
      gTimeSynced = false;
#if defined(ESP32) || defined(ESP8266)
      WiFi.disconnect(true, true);
#endif
      Serial.println("[WEB] Wi-Fi settings saved, reconnect scheduled");
    });
    gWebStarted = true;
  }
}

void startAccessPoint() {
#if defined(ESP32) || defined(ESP8266)
  if (gApMode) {
    ensureWebStarted();
    return;
  }

  WiFi.mode(WIFI_AP_STA);
  bool ok = WiFi.softAP(AP_SSID, AP_PASS);
  if (ok) {
    gApMode = true;
    Serial.print("[WIFI] AP started: ");
    Serial.println(AP_SSID);
    Serial.print("[WIFI] AP IP: ");
    Serial.println(WiFi.softAPIP());
    ensureWebStarted();
  } else {
    Serial.println("[WIFI] failed to start AP");
  }
#endif
}

void stopAccessPointIfNeeded() {
#if defined(ESP32) || defined(ESP8266)
  if (!gApMode) return;

  if (WiFi.status() == WL_CONNECTED) {
    WiFi.softAPdisconnect(true);
    gApMode = false;
    Serial.println("[WIFI] AP stopped after STA connect");
  }
#endif
}

void beginWifiConnect(uint32_t now) {
#if defined(ESP32) || defined(ESP8266)
  gLastWifiAttemptAt = now;
  gWifiConnectStartedAt = now;
  gWifiConnecting = true;

  Serial.print("[WIFI] connecting to ");
  Serial.println(gSettings.data().wifiSsid);

  WiFi.mode(gApMode ? WIFI_AP_STA : WIFI_STA);
  ensureWebStarted();
  WiFi.begin(gSettings.data().wifiSsid, gSettings.data().wifiPass);
#endif
}

void finalizeWifiConnected() {
#if defined(ESP32) || defined(ESP8266)
  gWifiConnecting = false;
  gWifiConnectStartedAt = 0;

  Serial.print("[WIFI] connected, IP=");
  Serial.println(WiFi.localIP());

  ensureWebStarted();
  stopAccessPointIfNeeded();

  if (gSettings.data().useNtp) {
    startNtp();
    gLastNtpSyncCheckAt = 0;
  } else if (gSettings.data().manualEpoch > 100000) {
    applyManualEpoch(gSettings.data().manualEpoch);
  }
#endif
}

void handleWifiDisconnected(uint32_t now) {
#if defined(ESP32) || defined(ESP8266)
  gTimeSynced = false;
  gDisplay.setClockSynced(false);

  if (gWifiConnecting) {
    if ((now - gWifiConnectStartedAt) >= WIFI_CONNECT_TIMEOUT_MS) {
      gWifiConnecting = false;
      gWifiConnectStartedAt = 0;
      Serial.println("[WIFI] connect timeout, AP fallback stays active");
      startAccessPoint();
    }
    return;
  }

  if (gLastWifiAttemptAt == 0 || (now - gLastWifiAttemptAt) >= WIFI_RETRY_INTERVAL_MS) {
    beginWifiConnect(now);
  }
#endif
}

void ensureWifiAndTime(uint32_t now) {
#if !defined(ESP32) && !defined(ESP8266)
  (void)now;
  return;
#else
  if (!wifiConfigured()) {
    static bool warned = false;
    if (!warned) {
      warned = true;
      Serial.println("[WIFI] credentials not set, starting AP + local/manual clock mode");
    }
    gWifiConnecting = false;
    gWifiConnectStartedAt = 0;
    startAccessPoint();

    if (!gSettings.data().useNtp && gSettings.data().manualEpoch > 100000) {
      applyManualEpoch(gSettings.data().manualEpoch);
    } else {
      gDisplay.setClockSynced(false);
    }
    return;
  }

  wl_status_t status = WiFi.status();

  if (status == WL_CONNECTED) {
    if (gWifiConnecting) {
      finalizeWifiConnected();
    }

    ensureWebStarted();
    stopAccessPointIfNeeded();

    if (!gSettings.data().useNtp) {
      if (gSettings.data().manualEpoch > 100000) {
        syncDisplayClockFromSystem(false);
      }
      return;
    }

    if (!gTimeSynced || gLastNtpSyncCheckAt == 0 || (now - gLastNtpSyncCheckAt) >= NTP_RESYNC_INTERVAL_MS) {
      gLastNtpSyncCheckAt = now;
      if (!gTimeSynced) {
        Serial.println("[NTP] waiting for valid time...");
      }
      syncDisplayClockFromSystem(true);
    }
    return;
  }

  handleWifiDisconnected(now);
#endif
}

void handleAlarm(uint32_t now) {
  static uint32_t lastReplayAt = 0;
  const bool wasRinging = gAlarm.isRinging();

  gAlarm.update(now);
  const bool ringing = gAlarm.isRinging();

  // Keep display state in sync with the alarm manager every loop.
  gDisplay.setAlarmRinging(ringing, now);

  if (ringing) {
    applyAlarmAudioProfile();

    if (!gAlarmSoundLatched) {
      gAlarmSoundLatched = true;
      lastReplayAt = 0;
      Serial.println("[ALARM] ringing start");

      // Wake brain state once so dismissal returns to a sensible animation,
      // but keep the actual visuals driven by DisplayManager::alarmRinging.
      if (gBrain.currentState == RobotState::SLEEP || gBrain.currentState == RobotState::CLOCK) {
        gEventQueue.push(RobotEvent::WAKEUP);
      }
    }

    SoundEffect nextSound = SoundEffect::ALARM_SOFT;
    const uint32_t ringingFor = gAlarm.ringingFor(now);

    if (ringingFor >= 25000UL) nextSound = SoundEffect::ALARM_STRONG;
    else if (ringingFor >= 10000UL) nextSound = SoundEffect::ALARM_MEDIUM;

    // Повтор із невеликим інтервалом, щоб будильник не “зависав” після одного програвання.
    if (!gAudio.isBusy() || lastReplayAt == 0 || (now - lastReplayAt) >= 1400UL) {
      gAudio.play(nextSound);
      lastReplayAt = now;
      Serial.print("[ALARM] replay sound -> ");
      Serial.print(soundName(nextSound));
      Serial.print(", volume=");
      Serial.println(alarmVolumeFromSettings(), 2);
    }
  } else {
    if (wasRinging || gAlarmSoundLatched) {
      Serial.println("[ALARM] ringing stop");
      gAudio.stop();
      gDisplay.setAlarmRinging(false, now);
    }

    gAlarmSoundLatched = false;
    applyNormalAudioProfile();
  }
}

bool isAlarmDismissTap(RobotEvent event) {
  return event == RobotEvent::TAP_SINGLE ||
         event == RobotEvent::TAP_DOUBLE ||
         event == RobotEvent::TAP_TRIPLE ||
         event == RobotEvent::TAP_RAPID;
}

void dismissAlarmByTouch(uint32_t now) {
  Serial.println("[ALARM] dismissed by touch");
  gAlarm.stop();
  gAudio.stop();
  gAlarmSoundLatched = false;
  applyNormalAudioProfile();

  // Wake Mimo visually after dismiss, but do not restart the alarm sound.
  gDisplay.requestBlink(now);
  if (gBrain.currentState == RobotState::SLEEP || gBrain.currentState == RobotState::CLOCK) {
    gDisplay.setLookTarget(0, 0);
    gBrain.changeState(RobotState::WAKE_SCAN, Config::WAKE_SCAN_MS, now);
  } else {
    gBrain.changeState(RobotState::HAPPY, 900, now);
  }
  gBrain.stateSoundPlayed = true;
}

void processEventQueue(uint32_t now) {
  while (!gEventQueue.isEmpty()) {
    RobotEvent event = gEventQueue.pop();
    if (event == RobotEvent::NONE) {
      break;
    }

    if (gAlarm.isRinging() && isAlarmDismissTap(event)) {
      dismissAlarmByTouch(now);
      gDisplay.setAlarmRinging(false, now);
      continue;
    }

    Serial.print("[MAIN] process -> ");
    Serial.println(eventName(event));

    gBrain.processEvent(event, gEventQueue, gDisplay, gAudio, now);
  }
}

void setup() {
  Serial.begin(Config::SERIAL_BAUD);
  delay(200);
  Serial.println();
  Serial.println("=== MIMO START ===");

  randomSeed(micros());

  Serial.println("[SETUP] touch.begin");
  gTouch.begin();

  Serial.println("[SETUP] battery.begin");
  gBattery.begin();

  Serial.println("[SETUP] audio.begin");
  gAudio.begin();
  applyNormalAudioProfile();

  Serial.println("[SETUP] display.begin");
  gDisplay.begin();

  Serial.println("[SETUP] display.setClockTime");
  gDisplay.setClockTime(12, 0, 0, millis());

  Serial.println("[SETUP] settings.begin");
  gSettings.begin();

  Serial.println("[SETUP] alarm.begin");
  gAlarm.begin(&gSettings);

  Serial.println("[SETUP] wifi init");
#if defined(ESP32) || defined(ESP8266)
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
  gWifiConnecting = false;
  gWifiConnectStartedAt = 0;
#endif

  Serial.println("[SETUP] ensureWifiAndTime");
  ensureWifiAndTime(millis());

  Serial.println("[SETUP] brain.begin");
  gBrain.begin(millis());

  Serial.println("[SYSTEM] init complete");
}

void loop() {
  const uint32_t now = millis();

  ensureWifiAndTime(now);
  if (gWebStarted) {
    gWeb.update();
  }

  gTouch.update(gEventQueue, now);
  gBattery.update(gEventQueue, now);
  handleAlarm(now);

  processEventQueue(now);

  gBrain.update(gEventQueue, gDisplay, gAudio, now);

  processEventQueue(now);

if (!gAlarm.isRinging()) {
  // Якщо користувач змінив alarmVolume через веб,
  // звичайний sound profile теж плавно підлаштується без ребута.
  applyNormalAudioProfile();
}

gAudio.update(now);
  gDisplay.render(gBrain.currentState, now, gBattery.voltage);
}
