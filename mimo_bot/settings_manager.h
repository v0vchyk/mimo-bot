#pragma once

#include <Arduino.h>

#if defined(ESP32)
  #include <Preferences.h>
#endif

struct MimoSettings {
  char adminUser[24] = "admin";
  char adminPass[32] = "mimo1234";

  char wifiSsid[33] = "";
  char wifiPass[65] = "";
  bool wifiConfigured = false;

  bool useNtp = true;
  char timezone[48] = "EET-2EEST,M3.5.0/3,M10.5.0/4";
  time_t manualEpoch = 0;

  bool alarmEnabled = false;
  uint8_t alarmHour = 7;
  uint8_t alarmMinute = 0;
  uint8_t alarmDaysMask = 0b01111110; // Mon..Fri when bit0=Sun
  uint8_t snoozeMinutes = 10;
  uint8_t alarmVolume = 80;
};

class SettingsManager {
public:
  bool begin() {
#if defined(ESP32)
    if (!prefs.begin("mimo", false)) {
      return false;
    }
#endif
    load();
    return true;
  }

  void end() {
#if defined(ESP32)
    prefs.end();
#endif
  }

  MimoSettings& data() { return settings; }
  const MimoSettings& data() const { return settings; }

  void load() {
#if defined(ESP32)
    readString("admin_u", settings.adminUser, sizeof(settings.adminUser), "admin");
    readString("admin_p", settings.adminPass, sizeof(settings.adminPass), "mimo1234");
    readString("wifi_ssid", settings.wifiSsid, sizeof(settings.wifiSsid), "");
    readString("wifi_pass", settings.wifiPass, sizeof(settings.wifiPass), "");
    settings.wifiConfigured = prefs.getBool("wifi_cfg", strlen(settings.wifiSsid) > 0);

    settings.useNtp = prefs.getBool("use_ntp", true);
    readString("tz", settings.timezone, sizeof(settings.timezone), "EET-2EEST,M3.5.0/3,M10.5.0/4");
    settings.manualEpoch = static_cast<time_t>(prefs.getULong64("man_epoch", 0));

    settings.alarmEnabled = prefs.getBool("alm_en", false);
    settings.alarmHour = prefs.getUChar("alm_h", 7);
    settings.alarmMinute = prefs.getUChar("alm_m", 0);
    settings.alarmDaysMask = prefs.getUChar("alm_days", 0b01111110);
    settings.snoozeMinutes = prefs.getUChar("snooze", 10);
    settings.alarmVolume = prefs.getUChar("alm_vol", 80);
#endif
  }

  void saveAll() {
#if defined(ESP32)
    prefs.putString("admin_u", settings.adminUser);
    prefs.putString("admin_p", settings.adminPass);
    prefs.putString("wifi_ssid", settings.wifiSsid);
    prefs.putString("wifi_pass", settings.wifiPass);
    prefs.putBool("wifi_cfg", settings.wifiConfigured);

    prefs.putBool("use_ntp", settings.useNtp);
    prefs.putString("tz", settings.timezone);
    prefs.putULong64("man_epoch", static_cast<uint64_t>(settings.manualEpoch));

    prefs.putBool("alm_en", settings.alarmEnabled);
    prefs.putUChar("alm_h", settings.alarmHour);
    prefs.putUChar("alm_m", settings.alarmMinute);
    prefs.putUChar("alm_days", settings.alarmDaysMask);
    prefs.putUChar("snooze", settings.snoozeMinutes);
    prefs.putUChar("alm_vol", settings.alarmVolume);
#endif
  }

  void setAdmin(const String& user, const String& pass) {
    copyString(user, settings.adminUser, sizeof(settings.adminUser));
    if (pass.length() > 0) {
      copyString(pass, settings.adminPass, sizeof(settings.adminPass));
    }
    saveAll();
  }

  void setWifi(const String& ssid, const String& pass) {
    copyString(ssid, settings.wifiSsid, sizeof(settings.wifiSsid));
    copyString(pass, settings.wifiPass, sizeof(settings.wifiPass));
    settings.wifiConfigured = ssid.length() > 0;
    saveAll();
  }

  void setClockMode(bool useNtp, const String& tz, time_t manualEpoch) {
    settings.useNtp = useNtp;
    copyString(tz, settings.timezone, sizeof(settings.timezone));
    if (manualEpoch > 0) {
      settings.manualEpoch = manualEpoch;
    }
    saveAll();
  }

  void setAlarm(bool enabled, uint8_t hour, uint8_t minute, uint8_t daysMask, uint8_t snooze, uint8_t volume) {
    settings.alarmEnabled = enabled;
    settings.alarmHour = constrain(hour, 0, 23);
    settings.alarmMinute = constrain(minute, 0, 59);
    settings.alarmDaysMask = daysMask;
    settings.snoozeMinutes = constrain(snooze, 1, 30);
    settings.alarmVolume = constrain(volume, 0, 100);
    saveAll();
  }

private:
  MimoSettings settings;

#if defined(ESP32)
  Preferences prefs;
#endif

  static void copyString(const String& src, char* dst, size_t dstSize) {
    if (dstSize == 0) return;
    src.substring(0, dstSize - 1).toCharArray(dst, dstSize);
  }

#if defined(ESP32)
  void readString(const char* key, char* dst, size_t dstSize, const char* fallback) {
    String value = prefs.getString(key, fallback);
    copyString(value, dst, dstSize);
  }
#endif
};
