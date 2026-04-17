#pragma once

#include <Arduino.h>

#if defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  #include <Update.h>
#endif

#include <time.h>
#include "settings_manager.h"
#include "battery_manager.h"
#include "audio_manager.h"
#include "display_manager.h"

class WebManager {
public:
  typedef void (*ManualTimeSetter)(time_t epoch);
  typedef void (*NtpSyncTrigger)();
  typedef void (*WifiApplyTrigger)();

  bool begin(SettingsManager* settings,
             BatteryManager* battery,
             AudioManager* audio,
             DisplayManager* display,
             bool* timeSyncedFlag) {
    this->settings = settings;
    this->battery = battery;
    this->audio = audio;
    this->display = display;
    this->timeSyncedFlag = timeSyncedFlag;

#if defined(ESP32)
    if (server) {
      Serial.println("[WEB] begin skipped: already started");
      return true;
    }

    server = new WebServer(80);
    const char* headerKeys[] = {"Cookie"};
    server->collectHeaders(headerKeys, 1);
    registerRoutes();
    server->begin();
    Serial.println("[WEB] server started on port 80");
    return true;
#else
    return false;
#endif
  }

  void update() {
#if defined(ESP32)
    if (server) server->handleClient();
    maybeRebootNow();
#endif
  }

  void setManualTimeSetter(ManualTimeSetter cb) { manualTimeSetter = cb; }
  void setNtpSyncTrigger(NtpSyncTrigger cb) { ntpSyncTrigger = cb; }
  void setWifiApplyTrigger(WifiApplyTrigger cb) { wifiApplyTrigger = cb; }

private:
  enum class PageId : uint8_t {
    LOGIN,
    HOME,
    CLOCK,
    WIFI,
    ALARM,
    SYSTEM,
    UPDATE_FW
  };

  SettingsManager* settings = nullptr;
  BatteryManager* battery = nullptr;
  AudioManager* audio = nullptr;
  DisplayManager* display = nullptr;
  bool* timeSyncedFlag = nullptr;

  ManualTimeSetter manualTimeSetter = nullptr;
  NtpSyncTrigger ntpSyncTrigger = nullptr;
  WifiApplyTrigger wifiApplyTrigger = nullptr;

#if defined(ESP32)
  WebServer* server = nullptr;
#endif

  String sessionToken;
  bool otaInProgress = false;
  bool otaHasError = false;
  String otaError;
  String otaLastOkMessage;
  uint32_t otaProgressPercent = 0;
  bool rebootScheduled = false;
  uint32_t rebootAtMs = 0;

#if defined(ESP32)
  void registerRoutes() {
    server->on("/", HTTP_GET, [this]() { handleRoot(); });
    server->on("/login", HTTP_GET, [this]() { handleLoginPage(); });
    server->on("/login", HTTP_POST, [this]() { handleLoginPost(); });
    server->on("/logout", HTTP_GET, [this]() { handleLogout(); });

    server->on("/clock", HTTP_GET, [this]() { handleClockPage(); });
    server->on("/clock", HTTP_POST, [this]() { handleClockSave(); });

    server->on("/wifi", HTTP_GET, [this]() { handleWifiPage(); });
    server->on("/wifi", HTTP_POST, [this]() { handleWifiSave(); });

    server->on("/alarm", HTTP_GET, [this]() { handleAlarmPage(); });
    server->on("/alarm", HTTP_POST, [this]() { handleAlarmSave(); });

    server->on("/system", HTTP_GET, [this]() { handleSystemPage(); });
    server->on("/system", HTTP_POST, [this]() { handleSystemSave(); });

    server->on("/update", HTTP_GET, [this]() { handleUpdatePage(); });
    server->on("/update", HTTP_POST,
      [this]() { handleUpdateFinished(); },
      [this]() { handleUpdateUpload(); }
    );

    server->on("/reboot", HTTP_POST, [this]() { handleRebootPost(); });
    server->onNotFound([this]() { server->send(404, "text/plain", "Not found"); });
  }

  bool isAuthenticated() {
    if (sessionToken.isEmpty()) return false;
    if (!server->hasHeader("Cookie")) return false;
    const String cookie = server->header("Cookie");
    return cookie.indexOf("MIMOSESSID=" + sessionToken) >= 0;
  }

  bool requireAuth() {
    if (isAuthenticated()) return true;
    server->sendHeader("Location", "/login");
    server->send(302, "text/plain", "Redirecting");
    return false;
  }

  String navLink(const __FlashStringHelper* href, const __FlashStringHelper* label, PageId current, PageId item) {
    String s;
    s += F("<a");
    if (current == item) s += F(" class='active'");
    s += F(" href='");
    s += String(href);
    s += F("'>");
    s += String(label);
    s += F("</a>");
    return s;
  }

  String page(const String& title, const String& body, PageId currentPage) {
    String html;
    html.reserve(13000);
    html += F("<!doctype html><html><head><meta charset='utf-8'>");
    html += F("<meta name='viewport' content='width=device-width,initial-scale=1'>");
    html += F("<title>");
    html += title;
    html += F("</title><style>");
    html += F(
      "body{font-family:Arial,sans-serif;background:#0f172a;color:#e5e7eb;margin:0;padding:20px;}"
      ".wrap{max-width:760px;margin:0 auto;}"
      ".card{background:#111827;border:1px solid #1f2937;border-radius:16px;padding:18px;margin:0 0 16px 0;box-shadow:0 8px 24px rgba(0,0,0,.2);}"
      "h1,h2,h3{margin:0 0 14px 0;}"
      "p{line-height:1.45;margin:0 0 12px 0;}"
      "a{color:#60a5fa;text-decoration:none;}"
      ".nav{display:flex;gap:12px;flex-wrap:wrap;margin-bottom:16px;}"
      ".nav a{background:#1f2937;padding:10px 14px;border-radius:12px;}"
      ".nav a.active{background:#2563eb;color:#fff;}"
      "label{display:block;margin:12px 0 6px 0;color:#cbd5e1;}"
      "input,select{width:100%;box-sizing:border-box;background:#0b1220;color:#fff;border:1px solid #334155;border-radius:10px;padding:12px;}"
      "input[type=file]{padding:10px;background:#0b1220;border:1px dashed #475569;}"
      "button{background:#2563eb;color:#fff;border:0;border-radius:12px;padding:12px 16px;font-weight:bold;cursor:pointer;margin-top:14px;}"
      "button.warn{background:#b45309;}"
      "button.danger{background:#b91c1c;}"
      ".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:12px;}"
      ".muted{color:#94a3b8;font-size:14px;}"
      ".ok{color:#22c55e}.warn{color:#f59e0b}.bad{color:#ef4444}"
      ".row{display:flex;gap:10px;align-items:center}.row input[type=checkbox]{width:auto}"
      ".days{display:grid;grid-template-columns:repeat(auto-fit,minmax(110px,1fr));gap:10px;margin-top:8px;}"
      ".day{display:flex;gap:8px;align-items:center;background:#0b1220;border:1px solid #334155;border-radius:10px;padding:10px 12px;}"
      ".day input{width:auto;margin:0;}"
      ".progress{height:12px;background:#0b1220;border:1px solid #334155;border-radius:999px;overflow:hidden;margin-top:10px;}"
      ".bar{height:100%;width:0;background:linear-gradient(90deg,#2563eb,#60a5fa);transition:width .2s ease;}"
      ".ota-meta{display:flex;justify-content:space-between;gap:12px;flex-wrap:wrap;margin-top:10px;font-size:14px;color:#cbd5e1;}"
      ".pill{display:inline-block;padding:8px 10px;border-radius:999px;background:#0b1220;border:1px solid #334155;margin-right:8px;margin-bottom:8px;}"
      ".hidden{display:none;}"
    );
    html += F("</style></head><body><div class='wrap'>");
    if (isAuthenticated()) {
      html += F("<div class='nav'>");
      html += navLink(F("/"), F("Головна"), currentPage, PageId::HOME);
      html += navLink(F("/clock"), F("Час"), currentPage, PageId::CLOCK);
      html += navLink(F("/wifi"), F("Wi‑Fi"), currentPage, PageId::WIFI);
      html += navLink(F("/alarm"), F("Будильник"), currentPage, PageId::ALARM);
      html += navLink(F("/system"), F("Система"), currentPage, PageId::SYSTEM);
      html += navLink(F("/update"), F("Оновлення"), currentPage, PageId::UPDATE_FW);
      html += F("<a href='/logout'>Вийти</a></div>");
    }
    html += body;
    html += F("</div></body></html>");
    return html;
  }

  String statusCard() {
    String s;
    s.reserve(1500);
    s += F("<div class='card'><h2>Статус Mimo</h2><div class='grid'>");

    s += F("<div><div class='muted'>Wi‑Fi</div><div>");
    s += (WiFi.status() == WL_CONNECTED) ? F("<span class='ok'>Підключено</span>") : F("<span class='warn'>Не підключено</span>");
    s += F("</div></div>");

    s += F("<div><div class='muted'>IP</div><div>");
    if (WiFi.status() == WL_CONNECTED) s += WiFi.localIP().toString();
    else s += F("—");
    s += F("</div></div>");

    s += F("<div><div class='muted'>Батарея</div><div>");
    s += String(battery ? battery->percent : 0);
    s += F("%</div></div>");

    s += F("<div><div class='muted'>Час</div><div>");
    time_t nowValue = time(nullptr);
    if (nowValue > 100000) {
      struct tm timeinfo;
      localtime_r(&nowValue, &timeinfo);
      char buf[24];
      strftime(buf, sizeof(buf), "%H:%M:%S", &timeinfo);
      s += buf;
    } else {
      s += F("--:--:--");
    }
    s += F("</div></div>");

    s += F("<div><div class='muted'>Синхронізація</div><div>");
    s += (timeSyncedFlag && *timeSyncedFlag) ? F("<span class='ok'>NTP</span>") : F("<span class='warn'>Локально</span>");
    s += F("</div></div>");

    s += F("<div><div class='muted'>Звук</div><div>");
    s += (audio && audio->isMuted()) ? F("Вимкнено") : F("Увімкнено");
    s += F("</div></div>");

    s += F("</div></div>");
    return s;
  }

  String dayCheckbox(uint8_t bit, const __FlashStringHelper* label, uint8_t mask) {
    String s;
    s += F("<label class='day'><input type='checkbox' name='day");
    s += String(bit);
    s += F("' value='1'");
    if (mask & (1 << bit)) s += F(" checked");
    s += F("><span>");
    s += String(label);
    s += F("</span></label>");
    return s;
  }

  uint8_t buildDaysMaskFromRequest() {
    uint8_t mask = 0;
    for (uint8_t i = 0; i < 7; ++i) {
      if (server->hasArg("day" + String(i))) {
        mask |= (1 << i);
      }
    }
    return mask;
  }

  const char* otaTargetText() const {
    return "firmware";
  }

  int otaCommandForTarget() const {
    return U_FLASH;
  }

  void scheduleReboot(uint32_t delayMs = 1200) {
    rebootScheduled = true;
    rebootAtMs = millis() + delayMs;
  }

  void maybeRebootNow() {
    if (rebootScheduled && (int32_t)(millis() - rebootAtMs) >= 0) {
      Serial.println("[WEB] reboot now");
      delay(100);
      ESP.restart();
    }
  }

  String otaClientScript(const char* formId, const char* fileId,
                         const char* progressWrapId, const char* progressBarId,
                         const char* percentId, const char* statusId) {
    String s;
    s.reserve(2400);
    s += F("<script>(function(){");
    s += F("const form=document.getElementById('"); s += formId; s += F("');");
    s += F("if(!form)return;");
    s += F("const file=document.getElementById('"); s += fileId; s += F("');");
    s += F("const wrap=document.getElementById('"); s += progressWrapId; s += F("');");
    s += F("const bar=document.getElementById('"); s += progressBarId; s += F("');");
    s += F("const percent=document.getElementById('"); s += percentId; s += F("');");
    s += F("const status=document.getElementById('"); s += statusId; s += F("');");
    s += F("form.addEventListener('submit',function(e){e.preventDefault();if(!file.files.length){alert('Вибери файл прошивки .bin');return;}const fd=new FormData();fd.append('firmware',file.files[0]);wrap.classList.remove('hidden');bar.style.width='0%';percent.textContent='0%';status.textContent='Початок завантаження...';const xhr=new XMLHttpRequest();xhr.open('POST','/update',true);");
    s += F("xhr.upload.onprogress=function(ev){if(ev.lengthComputable){const p=Math.min(100,Math.round((ev.loaded/ev.total)*100));bar.style.width=p+'%';percent.textContent=p+'%';status.textContent='Завантаження прошивки...';}};");
    s += F("xhr.onload=function(){bar.style.width='100%';percent.textContent='100%';document.open();document.write(xhr.responseText);document.close();};");
    s += F("xhr.onerror=function(){status.textContent='Помилка мережі під час оновлення';};xhr.send(fd);});})();</script>");
    return s;
  }

  void handleRoot() {
    if (!requireAuth()) return;
    String body;
    body.reserve(3200);
    body += F("<div class='card'><h1>Mimo Control Panel</h1><div class='muted'>Керування часом, Wi‑Fi, будильником та оновленням прошивки.</div></div>");
    body += statusCard();
    body += F("<div class='card'><h2>Швидкі дії</h2>");
    body += F("<form method='post' action='/clock'><input type='hidden' name='sync_now' value='1'><button type='submit'>Синхронізувати час зараз</button></form>");
    body += F("<form method='get' action='/update'><button class='warn' type='submit'>Оновити Mimo</button></form>");
    body += F("<form method='post' action='/reboot' onsubmit=\"return confirm('Перезавантажити Mimo зараз?');\"><button class='danger' type='submit'>Перезавантажити Mimo</button></form>");
    body += F("</div>");
    server->send(200, "text/html; charset=utf-8", page("Mimo", body, PageId::HOME));
  }

  void handleLoginPage() {
    String body;
    body += F("<div class='card'><h1>Вхід у Mimo</h1><form method='post' action='/login'>");
    body += F("<label>Логін</label><input name='user' autocomplete='username'>");
    body += F("<label>Пароль</label><input name='pass' type='password' autocomplete='current-password'>");
    body += F("<button type='submit'>Увійти</button></form></div>");
    server->send(200, "text/html; charset=utf-8", page("Login", body, PageId::LOGIN));
  }

  void handleLoginPost() {
    String user = server->arg("user");
    String pass = server->arg("pass");
    if (settings && user == settings->data().adminUser && pass == settings->data().adminPass) {
      sessionToken = randomToken();
      server->sendHeader("Set-Cookie", "MIMOSESSID=" + sessionToken + "; Path=/; HttpOnly; SameSite=Lax");
      server->sendHeader("Location", "/");
      server->send(302, "text/plain", "OK");
      return;
    }
    server->send(403, "text/html; charset=utf-8", page("Login", "<div class='card'><h1>Помилка входу</h1><p>Неправильний логін або пароль.</p><a href='/login'>Назад</a></div>", PageId::LOGIN));
  }

  void handleLogout() {
    sessionToken = "";
    server->sendHeader("Set-Cookie", "MIMOSESSID=deleted; Path=/; Max-Age=0; SameSite=Lax");
    server->sendHeader("Location", "/login");
    server->send(302, "text/plain", "Bye");
  }

  void handleClockPage() {
    if (!requireAuth()) return;
    String body;
    body += F("<div class='card'><h2>Налаштування часу</h2><form method='post' action='/clock'>");
    body += F("<div class='row'><input type='checkbox' name='use_ntp' value='1'");
    if (settings->data().useNtp) body += F(" checked");
    body += F("><span>Використовувати NTP</span></div>");
    body += F("<label>Таймзона</label><input name='timezone' value='");
    body += settings->data().timezone;
    body += F("'>");
    body += F("<label>Ручний час (Unix epoch, якщо без NTP)</label><input name='manual_epoch' value='");
    body += String((uint32_t)settings->data().manualEpoch);
    body += F("'>");
    body += F("<button type='submit'>Зберегти час</button></form>");
    body += F("<form method='post' action='/clock'><input type='hidden' name='sync_now' value='1'><button type='submit'>Синхронізувати зараз</button></form></div>");
    server->send(200, "text/html; charset=utf-8", page("Clock", body, PageId::CLOCK));
  }

  void handleClockSave() {
    if (!requireAuth()) return;
    if (server->hasArg("sync_now")) {
      if (ntpSyncTrigger) ntpSyncTrigger();
      redirectWithMessage("/clock");
      return;
    }
    bool useNtp = server->hasArg("use_ntp");
    String tz = server->arg("timezone");
    time_t manualEpoch = (time_t)server->arg("manual_epoch").toInt();
    settings->setClockMode(useNtp, tz, manualEpoch);
    if (!useNtp && manualEpoch > 0 && manualTimeSetter) manualTimeSetter(manualEpoch);
    if (useNtp && ntpSyncTrigger) ntpSyncTrigger();
    redirectWithMessage("/clock");
  }

  void handleWifiPage() {
    if (!requireAuth()) return;
    String body;
    body += F("<div class='card'><h2>Wi‑Fi</h2><form method='post' action='/wifi'>");
    body += F("<label>SSID</label><input name='ssid' value='");
    body += settings->data().wifiSsid;
    body += F("'>");
    body += F("<label>Пароль</label><input name='pass' type='password' value=''>");
    body += F("<div class='muted'>Якщо поле порожнє, старий пароль буде збережено.</div>");
    body += F("<button type='submit'>Зберегти Wi‑Fi</button></form></div>");
    server->send(200, "text/html; charset=utf-8", page("WiFi", body, PageId::WIFI));
  }

  void handleWifiSave() {
    if (!requireAuth()) return;
    String ssid = server->arg("ssid");
    String pass = server->arg("pass");
    if (pass.length() == 0) pass = settings->data().wifiPass;
    settings->setWifi(ssid, pass);
    if (wifiApplyTrigger) wifiApplyTrigger();
    redirectWithMessage("/wifi");
  }

  void handleAlarmPage() {
    if (!requireAuth()) return;
    const uint8_t mask = settings->data().alarmDaysMask;
    String body;
    body.reserve(3000);
    body += F("<div class='card'><h2>Будильник</h2><form method='post' action='/alarm'>");
    body += F("<div class='row'><input type='checkbox' name='enabled' value='1'");
    if (settings->data().alarmEnabled) body += F(" checked");
    body += F("><span>Увімкнено</span></div>");
    body += F("<label>Година</label><input name='hour' type='number' min='0' max='23' value='");
    body += String(settings->data().alarmHour);
    body += F("'>");
    body += F("<label>Хвилина</label><input name='minute' type='number' min='0' max='59' value='");
    body += String(settings->data().alarmMinute);
    body += F("'>");
    body += F("<label>Дні</label><div class='days'>");
    body += dayCheckbox(1, F("Пн"), mask);
    body += dayCheckbox(2, F("Вт"), mask);
    body += dayCheckbox(3, F("Ср"), mask);
    body += dayCheckbox(4, F("Чт"), mask);
    body += dayCheckbox(5, F("Пт"), mask);
    body += dayCheckbox(6, F("Сб"), mask);
    body += dayCheckbox(0, F("Нд"), mask);
    body += F("</div>");
    body += F("<label>Snooze (хв)</label><input name='snooze' type='number' min='1' max='30' value='");
    body += String(settings->data().snoozeMinutes);
    body += F("'>");
    body += F("<label>Гучність 0-100</label><input name='volume' type='number' min='0' max='100' value='");
    body += String(settings->data().alarmVolume);
    body += F("'>");
    body += F("<button type='submit'>Зберегти будильник</button></form></div>");
    server->send(200, "text/html; charset=utf-8", page("Alarm", body, PageId::ALARM));
  }

  void handleAlarmSave() {
    if (!requireAuth()) return;
    settings->setAlarm(server->hasArg("enabled"),
                       (uint8_t)server->arg("hour").toInt(),
                       (uint8_t)server->arg("minute").toInt(),
                       buildDaysMaskFromRequest(),
                       (uint8_t)server->arg("snooze").toInt(),
                       (uint8_t)server->arg("volume").toInt());
    redirectWithMessage("/alarm");
  }

  void handleSystemPage() {
    if (!requireAuth()) return;
    String body;
    body += statusCard();
    body += F("<div class='card'><h2>Система</h2><form method='post' action='/system'>");
    body += F("<label>Логін</label><input name='user' value='");
    body += settings->data().adminUser;
    body += F("'>");
    body += F("<label>Новий пароль</label><input name='pass' type='password' value=''>");
    body += F("<div class='muted'>Якщо поле порожнє, пароль не змінюється.</div>");
    body += F("<button type='submit'>Зберегти доступ</button></form>");
    body += F("<form method='post' action='/reboot' onsubmit=\"return confirm('Перезавантажити Mimo зараз?');\"><button class='danger' type='submit'>Перезавантажити Mimo</button></form>");
    body += F("</div>");
    server->send(200, "text/html; charset=utf-8", page("System", body, PageId::SYSTEM));
  }

  void handleSystemSave() {
    if (!requireAuth()) return;
    settings->setAdmin(server->arg("user"), server->arg("pass"));
    redirectWithMessage("/system");
  }

  void handleUpdatePage() {
    if (!requireAuth()) return;

    String body;
    body.reserve(6500);
    body += F("<div class='card'><h2>Оновлення Mimo</h2>");
    body += F("<p class='muted'>Оберіть файл <b>основної прошивки (.bin)</b> і завантажте його. Після успішного оновлення Mimo перезавантажиться автоматично.</p>");
    body += F("<div class='pill'>Використовуйте лише файл основної прошивки</div>");
    body += F("</div>");

    body += F("<div class='card'><h3>Залити прошивку</h3>");
    body += F("<form id='fwForm' method='post' action='/update' enctype='multipart/form-data'>");
    body += F("<label>Файл прошивки (.bin)</label><input id='fwFile' type='file' name='firmware' accept='.bin,application/octet-stream' required>");
    body += F("<button class='warn' type='submit'>Залити прошивку</button></form>");
    body += F("<div id='fwProgressWrap' class='hidden'><div class='progress'><div id='fwBar' class='bar'></div></div><div class='ota-meta'><span id='fwStatus'>Очікування...</span><strong id='fwPercent'>0%</strong></div></div>");
    body += F("</div>");

    if (otaInProgress) {
      body += F("<div class='card'><h3>Остання OTA-сесія</h3><p class='muted'>Зараз виконується запис. Тип: <b>");
      body += otaTargetText();
      body += F("</b></p><div class='progress'><div class='bar' style='width:");
      body += String(otaProgressPercent);
      body += F("%'></div></div><div class='ota-meta'><span>Стан сервера OTA</span><strong>");
      body += String(otaProgressPercent);
      body += F("%</strong></div></div>");
    }

    if (otaHasError && otaError.length()) {
      body += F("<div class='card'><h3>Помилка оновлення</h3><p class='bad'><b>");
      body += otaError;
      body += F("</b></p></div>");
    }

    if (otaLastOkMessage.length()) {
      body += F("<div class='card'><h3>Останній результат</h3><p class='ok'><b>");
      body += otaLastOkMessage;
      body += F("</b></p></div>");
    }

    body += F("<div class='card'><h3>Перезавантаження</h3><p class='muted'>Після успішного оновлення Mimo перезавантажиться автоматично. За потреби можна зробити це вручну.</p><form method='post' action='/reboot' onsubmit=\"return confirm('Перезавантажити Mimo зараз?');\"><button class='danger' type='submit'>Перезавантажити Mimo</button></form></div>");

    body += otaClientScript("fwForm", "fwFile", "fwProgressWrap", "fwBar", "fwPercent", "fwStatus");
    server->send(200, "text/html; charset=utf-8", page("Update", body, PageId::UPDATE_FW));
  }

  void handleUpdateFinished() {
    if (!requireAuth()) return;

    if (otaHasError) {
      String body;
      body += F("<div class='card'><h2>Оновлення не вдалося</h2><p class='bad'>");
      body += otaError;
      body += F("</p><a href='/update'>Назад</a></div>");
      server->send(500, "text/html; charset=utf-8", page("Update failed", body, PageId::UPDATE_FW));
      return;
    }

    String body;
    body += F("<div class='card'><h2>Оновлення завершено</h2><p>");
    body += otaLastOkMessage.length() ? otaLastOkMessage : F("Mimo перезавантажиться за кілька секунд.");
    body += F("</p></div>");
    server->send(200, "text/html; charset=utf-8", page("Update OK", body, PageId::UPDATE_FW));
    scheduleReboot(1200);
  }

  void handleUpdateUpload() {
    if (!requireAuth()) return;

    HTTPUpload& upload = server->upload();

    if (upload.status == UPLOAD_FILE_START) {
      otaInProgress = true;
      otaHasError = false;
      otaError = "";
      otaLastOkMessage = "";
      otaProgressPercent = 0;

      Serial.printf("[OTA] upload start: %s (%s)\n", upload.filename.c_str(), otaTargetText());

      String lowerName = upload.filename;
      lowerName.toLowerCase();
      if (lowerName.indexOf("littlefs") >= 0 || lowerName.indexOf("spiffs") >= 0 || lowerName.indexOf("filesystem") >= 0 || lowerName.indexOf("fs.bin") >= 0) {
        otaHasError = true;
        otaError = F("Схоже, це файл filesystem / LittleFS. Тут потрібно завантажувати лише основну прошивку .bin");
        Serial.println("[OTA] rejected non-firmware bin");
        return;
      }

      if (!Update.begin(UPDATE_SIZE_UNKNOWN, otaCommandForTarget())) {
        otaHasError = true;
        otaError = String("Update.begin failed: ") + Update.errorString();
        Serial.println("[OTA] begin failed");
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (!otaHasError) {
        size_t written = Update.write(upload.buf, upload.currentSize);
        if (written != upload.currentSize) {
          otaHasError = true;
          otaError = String("Write failed: ") + Update.errorString();
          Serial.println("[OTA] write failed");
        } else if (upload.totalSize > 0) {
          uint32_t pct = (uint32_t)(((uint64_t)Update.progress() * 100ULL) / (uint64_t)upload.totalSize);
          if (pct > 100) pct = 100;
          otaProgressPercent = pct;
        }
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      otaInProgress = false;
      otaProgressPercent = 100;
      if (!otaHasError) {
        if (!Update.end(true)) {
          otaHasError = true;
          otaError = String("Finalize failed: ") + Update.errorString();
          Serial.println("[OTA] finalize failed");
        } else {
          otaLastOkMessage = F("Прошивку успішно оновлено. Mimo перезавантажиться.");
          Serial.printf("[OTA] success, size=%u, target=%s\n", upload.totalSize, otaTargetText());
        }
      } else {
        Update.abort();
      }
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
      otaInProgress = false;
      otaHasError = true;
      otaError = F("Upload aborted");
      otaProgressPercent = 0;
      Update.abort();
      Serial.println("[OTA] upload aborted");
    }
  }

  void handleRebootPost() {
    if (!requireAuth()) return;
    String body;
    body += F("<div class='card'><h2>Перезавантаження</h2><p>Mimo перезавантажиться за мить.</p></div>");
    server->send(200, "text/html; charset=utf-8", page("Reboot", body, PageId::SYSTEM));
    scheduleReboot(800);
  }

  void redirectWithMessage(const char* path) {
    server->sendHeader("Location", path);
    server->send(302, "text/plain", "Saved");
  }

  String randomToken() {
    char buf[33];
    for (int i = 0; i < 32; ++i) {
      const uint8_t v = random(0, 16);
      buf[i] = (v < 10) ? ('0' + v) : ('a' + v - 10);
    }
    buf[32] = '\0';
    return String(buf);
  }
#endif
};
