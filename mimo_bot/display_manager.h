#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <time.h>
#include "robot_types.h"

struct DisplayManager {
  static constexpr int SCREEN_W = 280;
  static constexpr int SCREEN_H = 240;

  static constexpr int CANVAS_X = 24;
  static constexpr int CANVAS_Y = 34;
  static constexpr int CANVAS_W = 232;
  static constexpr int CANVAS_H = 170;

  Adafruit_ST7789 tft = Adafruit_ST7789(Pins::TFT_CS, Pins::TFT_DC, Pins::TFT_RST);
  GFXcanvas16 canvas = GFXcanvas16(CANVAS_W, CANVAS_H);

  uint32_t lastRenderAt = 0;
  uint32_t lastBlinkAt = 0;
  uint32_t lastHudAt = 0;
  uint32_t lastDebugAt = 0;
  uint32_t lastSleepAnimAt = 0;
  uint32_t lastIdleMicroAt = 0;
  uint32_t lastLovePulseAt = 0;
  uint32_t lastMouthAnimAt = 0;
  uint32_t lastIdleDoubleBlinkAt = 0;
  uint32_t lastLookAt = 0;
  uint32_t lastGlowPulseAt = 0;

  uint32_t clockBaseMillis = 0;
  uint32_t stateEnteredAt = 0;
  bool clockHasExternalTime = false;
  bool ntpSynced = false;
  bool alarmRinging = false;
  uint32_t alarmStartedAt = 0;

  RobotState lastState = RobotState::BOOT;

  bool blinking = false;
  bool dirty = true;
  bool idleDoubleBlinkQueued = false;

  uint8_t blinkPhase = 0;
  uint8_t sleepZPhase = 0;
  uint8_t lovePulsePhase = 0;
  uint8_t mouthAnimPhase = 0;
  uint8_t glowPulsePhase = 0;

  int lookX = 0;
  int lookY = 0;
  int targetLookX = 0;
  int targetLookY = 0;
  int angryLevel = 0;

  int idleMicroX = 0;
  int idleMicroY = 0;

  int loveLookDownOffset = 0;
  int angrySquint = 0;
  int hyperZoom = 0;
  int sleepLidExtra = 0;
  int faceBobY = 0;
  int hudPulse = 0;

  uint16_t COLOR_BG         = ST77XX_BLACK;
  uint16_t COLOR_WHITE_EYE  = ST77XX_WHITE;
  uint16_t COLOR_PUPIL      = 0x4D9F;
  uint16_t COLOR_HAPPY      = ST77XX_YELLOW;
  uint16_t COLOR_ANGRY      = ST77XX_RED;
  uint16_t COLOR_LOVE       = 0xF81F;
  uint16_t COLOR_SLEEP      = ST77XX_CYAN;
  uint16_t COLOR_LOW_BAT    = 0xFD20;
  uint16_t COLOR_BLUSH      = 0xFB56;
  uint16_t COLOR_HYPER      = 0x07FF;
  uint16_t COLOR_SOFT       = 0x7BEF;
  uint16_t COLOR_BOOT       = 0x8410;
  uint16_t COLOR_MOUTH      = 0xC618;
  uint16_t COLOR_DIM        = 0x4208;

  int eyeW = 74;
  int eyeH = 90;
  int leftEyeX = 30;
  int rightEyeX = 128;
  int eyeY = 24;

static constexpr int FACE_CX = CANVAS_W / 2;
static constexpr int TALL_EYE_W = 74;
static constexpr int TALL_EYE_H = 90;
static constexpr int TALL_EYE_Y = 24;
static constexpr int WIDE_EYE_W = 72;
static constexpr int WIDE_EYE_H = 22;
static constexpr int WIDE_EYE_Y = 90;
static constexpr int MOUTH_Y = 130;

int centeredPairStartX(int itemW, int gap) const {
  return (CANVAS_W - (itemW * 2 + gap)) / 2;
}

int faceLeftX(int itemW, int gap) const {
  return centeredPairStartX(itemW, gap);
}

int faceRightX(int itemW, int gap) const {
  return centeredPairStartX(itemW, gap) + itemW + gap;
}

int centeredX(int w) const {
  return (CANVAS_W - w) / 2;
}

int mouthBaseY() const {
  return MOUTH_Y;
}

int mouthCenterX() const {
  return ((leftEyeX + eyeW / 2) + (rightEyeX + eyeW / 2)) / 2;
}

  float easeOutCubic(float t) const {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    float inv = 1.0f - t;
    return 1.0f - inv * inv * inv;
  }

  float easeInOutSine(float t) const {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return 0.5f - 0.5f * cosf(t * PI);
  }

  int blinkLidHeight(int h) const {
    switch (blinkPhase) {
      case 1: return h / 6;
      case 2: return h / 2;
      case 3: return h - 8;
      case 4: return h / 2;
      case 5: return h / 6;
      default: return 0;
    }
  }

  int glowOffset() const {
    switch (glowPulsePhase) {
      case 0: return 0;
      case 1: return 1;
      case 2: return 2;
      case 3: return 1;
      default: return 0;
    }
  }

void resetFaceLayout() {
  eyeW = TALL_EYE_W;
  eyeH = TALL_EYE_H;
  eyeY = TALL_EYE_Y;
  leftEyeX = faceLeftX(eyeW, 24);
  rightEyeX = faceRightX(eyeW, 24);
}

void useWideFaceLayout(int y = WIDE_EYE_Y, int h = WIDE_EYE_H) {
  eyeW = WIDE_EYE_W;
  eyeH = h;
  eyeY = y;
  leftEyeX = faceLeftX(eyeW, 28);
  rightEyeX = faceRightX(eyeW, 28);
}

  void begin() {
    pinMode(Pins::TFT_BL, OUTPUT);
    digitalWrite(Pins::TFT_BL, HIGH);

    SPI.begin(Pins::TFT_SCK, -1, Pins::TFT_MOSI, Pins::TFT_CS);

    Serial.println("[DISPLAY] before tft.init");
    tft.init(240, 280);
    Serial.println("[DISPLAY] after tft.init");

    tft.setRotation(3);
    tft.fillScreen(COLOR_BG);

    clearCanvas();
    pushCanvas();

    dirty = true;
    resetFaceLayout();

    Serial.println("[DISPLAY] ST7789 + canvas init ok");
  }

  void setBacklight(bool on) {
    digitalWrite(Pins::TFT_BL, on ? HIGH : LOW);
  }

  void markDirty() {
    dirty = true;
  }

  void chooseLook() {
    targetLookX = random(-10, 11);
    targetLookY = random(-8, 9);
    dirty = true;
  }

  void setLookTarget(int x, int y) {
    targetLookX = x;
    targetLookY = y;
    dirty = true;
  }

  void updateLook(uint32_t now) {
    if (now - lastLookAt < 16) return;
    lastLookAt = now;

    bool changed = false;

    int dx = targetLookX - lookX;
    int dy = targetLookY - lookY;

    if (dx != 0) {
      int stepX = (abs(dx) > 6) ? 2 : 1;
      lookX += (dx > 0) ? stepX : -stepX;
      if (abs(targetLookX - lookX) < stepX) lookX = targetLookX;
      changed = true;
    }

    if (dy != 0) {
      int stepY = (abs(dy) > 4) ? 2 : 1;
      lookY += (dy > 0) ? stepY : -stepY;
      if (abs(targetLookY - lookY) < stepY) lookY = targetLookY;
      changed = true;
    }

    if (changed) dirty = true;
  }

  void updateIdleMicroMotion(uint32_t now) {
    if (now - lastIdleMicroAt < 80) return;
    lastIdleMicroAt = now;

    int phase = (now / 320) % 8;
    int nx = 0;
    int ny = 0;

    switch (phase) {
      case 0: nx = 0;  ny = 0;  break;
      case 1: nx = 1;  ny = 0;  break;
      case 2: nx = 1;  ny = -1; break;
      case 3: nx = 0;  ny = -1; break;
      case 4: nx = -1; ny = 0;  break;
      case 5: nx = -1; ny = 1;  break;
      case 6: nx = 0;  ny = 1;  break;
      default:nx = 0;  ny = 0;  break;
    }

    int bobPhase = (now / 280) % 6;
    int nb = (bobPhase == 1 || bobPhase == 2) ? 1 : ((bobPhase == 4) ? -1 : 0);

    if (nx != idleMicroX || ny != idleMicroY || nb != faceBobY) {
      idleMicroX = nx;
      idleMicroY = ny;
      faceBobY = nb;
      dirty = true;
    }
  }

  void requestBlink(uint32_t now) {
    blinking = true;
    blinkPhase = 0;
    lastBlinkAt = now;
    dirty = true;
  }

  void updateBlink(uint32_t now) {
    if (!blinking) return;
    if (now - lastBlinkAt < 30) return;

    lastBlinkAt = now;
    blinkPhase++;
    dirty = true;

    if (blinkPhase > 5) {
      blinking = false;
      blinkPhase = 0;
      dirty = true;
    }
  }

  void updateIdleDoubleBlink(uint32_t now, RobotState state) {
    if (state != RobotState::IDLE) {
      idleDoubleBlinkQueued = false;
      return;
    }

    if (!idleDoubleBlinkQueued && (now - lastIdleDoubleBlinkAt) > 9000) {
      requestBlink(now);
      idleDoubleBlinkQueued = true;
      lastIdleDoubleBlinkAt = now;
    }

    if (idleDoubleBlinkQueued && !blinking && (now - lastIdleDoubleBlinkAt) > 220) {
      requestBlink(now);
      idleDoubleBlinkQueued = false;
      lastIdleDoubleBlinkAt = now;
    }
  }

  void updateLoveLookDown(uint32_t now, RobotState state) {
    if (state != RobotState::LOVE && state != RobotState::LOVE_TRANSITION) {
      if (loveLookDownOffset != 0) {
        loveLookDownOffset = 0;
        dirty = true;
      }
      return;
    }

    int phase = (now / 220) % 8;
    int target = (phase < 2) ? 4 : ((phase < 4) ? 2 : 0);

    if (loveLookDownOffset < target) {
      loveLookDownOffset++;
      dirty = true;
    } else if (loveLookDownOffset > target) {
      loveLookDownOffset--;
      dirty = true;
    }
  }

  void updateAngrySquint(uint32_t now, RobotState state) {
    if (state != RobotState::ANGRY_1 &&
        state != RobotState::ANGRY_2 &&
        state != RobotState::ANGRY_3) {
      if (angrySquint != 0) {
        angrySquint = 0;
        dirty = true;
      }
      return;
    }

    int phase = (now / 140) % 4;
    int target = (phase == 1 || phase == 2) ? (4 + angryLevel * 2) : 0;

    if (angrySquint < target) {
      angrySquint++;
      dirty = true;
    } else if (angrySquint > target) {
      angrySquint--;
      dirty = true;
    }
  }

  void updateHyperZoom(uint32_t now, RobotState state) {
    if (state != RobotState::HYPER) {
      if (hyperZoom != 0) {
        hyperZoom = 0;
        dirty = true;
      }
      return;
    }

    int phase = (now / 90) % 4;
    int target = (phase == 1 || phase == 2) ? 4 : 0;

    if (hyperZoom < target) {
      hyperZoom++;
      dirty = true;
    } else if (hyperZoom > target) {
      hyperZoom--;
      dirty = true;
    }
  }

  void updateSleepLid(uint32_t now, RobotState state) {
    if (state != RobotState::TIRED && state != RobotState::SLEEPY && state != RobotState::SLEEP) {
      if (sleepLidExtra != 0) {
        sleepLidExtra = 0;
        dirty = true;
      }
      return;
    }

    if (state == RobotState::TIRED) {
      int phase = (now / 260) % 6;
      int target = (phase < 3) ? (2 + phase * 2) : (2 + (5 - phase) * 2);

      if (sleepLidExtra < target) {
        sleepLidExtra++;
        dirty = true;
      } else if (sleepLidExtra > target) {
        sleepLidExtra--;
        dirty = true;
      }
    } else if (state == RobotState::SLEEPY) {
      int phase = (now / 220) % 6;
      int target = (phase < 3) ? (phase * 4) : ((5 - phase) * 4);

      if (sleepLidExtra < target) {
        sleepLidExtra++;
        dirty = true;
      } else if (sleepLidExtra > target) {
        sleepLidExtra--;
        dirty = true;
      }
    } else {
      if (sleepLidExtra < 10) {
        sleepLidExtra++;
        dirty = true;
      }
    }
  }

  void updateSleepAnim(uint32_t now) {
    if (now - lastSleepAnimAt < 500) return;
    lastSleepAnimAt = now;
    sleepZPhase++;
    if (sleepZPhase > 2) sleepZPhase = 0;
    dirty = true;
  }

  void updateLovePulse(uint32_t now) {
    if (now - lastLovePulseAt < 160) return;
    lastLovePulseAt = now;
    lovePulsePhase = (lovePulsePhase + 1) % 4;
    dirty = true;
  }

  void updateGlowPulse(uint32_t now, RobotState state) {
    bool active = (state == RobotState::IDLE ||
                   state == RobotState::CLOCK ||
                   state == RobotState::LOVE ||
                   state == RobotState::HAPPY ||
                   state == RobotState::SLEEP ||
                   state == RobotState::SLEEPY);
    if (!active) {
      if (glowPulsePhase != 0) {
        glowPulsePhase = 0;
        dirty = true;
      }
      return;
    }

    if (now - lastGlowPulseAt < 140) return;
    lastGlowPulseAt = now;
    glowPulsePhase = (glowPulsePhase + 1) % 4;
    dirty = true;
  }

  void updateMouthAnim(RobotState state, uint32_t now) {
    uint32_t interval = 0;

    switch (state) {
      case RobotState::HAPPY:
        interval = 180;
        break;
      case RobotState::LOVE:
        interval = 220;
        break;
      case RobotState::ANGRY_1:
      case RobotState::ANGRY_2:
      case RobotState::ANGRY_3:
        interval = 120;
        break;
      case RobotState::YAWN:
        interval = 220;
        break;
      case RobotState::OFFENDED:
        interval = 260;
        break;
      default:
        mouthAnimPhase = 0;
        return;
    }

    if (now - lastMouthAnimAt < interval) return;
    lastMouthAnimAt = now;
    mouthAnimPhase = (mouthAnimPhase + 1) % 4;
    dirty = true;
  }

  bool eyesClosed() const {
    return blinking;
  }

  int currentLoveHeartSize() const {
    switch (lovePulsePhase) {
      case 0: return 10;
      case 1: return 11;
      case 2: return 12;
      default: return 11;
    }
  }

  int currentSleepBreathOffset() const {
    switch (sleepZPhase) {
      case 0: return 0;
      case 1: return 2;
      default: return 4;
    }
  }

  int happyMouthLift() const {
    switch (mouthAnimPhase) {
      case 0: return 0;
      case 1: return 1;
      case 2: return 2;
      default: return 1;
    }
  }

  int loveMouthLift() const {
    switch (mouthAnimPhase) {
      case 0: return 0;
      case 1: return 1;
      case 2: return 1;
      default: return 0;
    }
  }

  int angryMouthShift() const {
    switch (mouthAnimPhase) {
      case 0: return 0;
      case 1: return -1;
      case 2: return 1;
      default: return 0;
    }
  }


  void setClockTime(uint8_t hours, uint8_t minutes, uint8_t seconds = 0, uint32_t now = 0) {
    uint32_t total = ((uint32_t)(hours % 24) * 3600UL) + ((uint32_t)(minutes % 60) * 60UL) + (seconds % 60);
    clockBaseMillis = now - (total * 1000UL);
    clockHasExternalTime = false;
    markDirty();
  }

  void setClockSynced(bool synced) {
    if (ntpSynced != synced) {
      ntpSynced = synced;
      markDirty();
    }
  }

  void setClockFromEpoch(time_t epochSeconds) {
    if (epochSeconds > 100000) {
      clockHasExternalTime = true;
      ntpSynced = true;
      markDirty();
    }
  }
  void setAlarmRinging(bool ringing, uint32_t now = 0) {
    if (alarmRinging != ringing) {
      alarmRinging = ringing;
      uint32_t stamp = now ? now : millis();
      if (ringing) {
        alarmStartedAt = stamp;
        stateEnteredAt = stamp;
      }
      lastRenderAt = 0;   // force immediate redraw on alarm edge
      lastHudAt = 0;
      markDirty();
    }
  }


  void getClockTime(uint32_t now, uint8_t& hours, uint8_t& minutes, uint8_t& seconds) const {
    if (clockHasExternalTime) {
      time_t epoch = time(nullptr);
      if (epoch > 100000) {
        struct tm timeinfo;
        localtime_r(&epoch, &timeinfo);
        hours = timeinfo.tm_hour;
        minutes = timeinfo.tm_min;
        seconds = timeinfo.tm_sec;
        return;
      }
    }

    uint32_t elapsed = (now - clockBaseMillis) / 1000UL;
    uint32_t daySeconds = elapsed % 86400UL;
    hours = daySeconds / 3600UL;
    minutes = (daySeconds % 3600UL) / 60UL;
    seconds = daySeconds % 60UL;
  }

  void clearCanvas() {
    canvas.fillScreen(COLOR_BG);
  }

  void pushCanvas() {
    tft.drawRGBBitmap(CANVAS_X, CANVAS_Y, canvas.getBuffer(), CANVAS_W, CANVAS_H);
  }

  int voltageToPercent(float v) {
    if (v >= 4.20f) return 100;
    if (v <= 3.30f) return 0;
    return (int)(((v - 3.30f) * 100.0f) / (4.20f - 3.30f));
  }

  void drawBattery(float batteryVoltage) {
    int pct = constrain(voltageToPercent(batteryVoltage), 0, 100);

    const int w = 22;
    const int h = 10;
    const int x = 12;
    const int y = 10;

    uint16_t frame = (pct <= 20) ? ((hudPulse) ? COLOR_LOW_BAT : ST77XX_RED) : COLOR_SOFT;
    uint16_t c = (pct > 50) ? ST77XX_GREEN : (pct > 20 ? ST77XX_YELLOW : ST77XX_RED);

    tft.fillRect(0, 0, 50, 24, COLOR_BG);
    tft.drawRoundRect(x, y, w, h, 3, frame);
    tft.fillRect(x + w, y + 3, 2, 4, frame);
    tft.fillRect(x + 2, y + 2, w - 4, h - 4, COLOR_BG);

    int fillW = map(pct, 0, 100, 0, w - 4);
    if (fillW > 0) {
      tft.fillRect(x + 2, y + 2, fillW, h - 4, c);
    }

    int bars = constrain((pct + 24) / 25, 0, 4);
    for (int i = 0; i < 4; ++i) {
      int bx = x + 3 + i * 4;
      if (i < bars) tft.fillRect(bx, y + 2, 2, h - 4, c);
    }
  }

  void renderHud(float batteryVoltage, uint32_t now, bool force = false) {
    if (!force && (now - lastHudAt < 300)) return;
    lastHudAt = now;

    hudPulse = ((now / 280UL) % 2UL) == 0 ? 1 : 0;
    drawBattery(batteryVoltage);
  }

  void fillSoftEye(int x, int y, int w, int h, uint16_t eyeColor, uint16_t shadeColor) {
    (void)shadeColor;
    canvas.fillRoundRect(x, y, w, h, 18, eyeColor);
  }

  void drawPupilRect(int x, int y, uint16_t color) {
    int w = 24 + hyperZoom;
    int h = 36 + hyperZoom;
    int rx = 8 + (hyperZoom / 2);

    canvas.fillRoundRect(x - hyperZoom / 2, y - hyperZoom / 2, w, h, rx, color);
    canvas.fillCircle(x + 8, y + 10, 4, ST77XX_WHITE);
  }

  void drawHeart(int cx, int cy, int size, uint16_t color) {
    int r = max(3, size / 3);
    canvas.fillCircle(cx - r, cy - r / 2, r, color);
    canvas.fillCircle(cx + r, cy - r / 2, r, color);
    canvas.fillRect(cx - r - 1, cy - r / 2, (r * 2) + 2, r, color);
    canvas.fillTriangle(cx - r - 2, cy + r / 3, cx, cy + size, cx, cy + r / 2, color);
    canvas.fillTriangle(cx + r + 2, cy + r / 3, cx, cy + size, cx, cy + r / 2, color);
    canvas.fillCircle(cx, cy + r / 2, (r / 2) + 1, color);
  }

  void drawBlushCentered(int cx, int y) {
    canvas.drawLine(cx - 5, y, cx + 4, y, COLOR_BLUSH);
    canvas.drawLine(cx - 4, y + 3, cx + 5, y + 3, COLOR_BLUSH);
  }

  int topIconCenterX() const {
    return (leftEyeX + eyeW / 2 + rightEyeX + eyeW / 2) / 2;
  }

  void drawIconExclaim() {
    canvas.setTextColor(COLOR_HAPPY);
    canvas.setTextSize(2);
    int cx = topIconCenterX();
    canvas.setCursor(cx - 4, 6);
    canvas.print("!");
  }

  void drawIconHeartTop() {
    drawHeart(topIconCenterX(), 16, 10, COLOR_LOVE);
  }

  void drawIconSparkles() {
    int cx = topIconCenterX();
    int cy = 11;
    canvas.drawLine(cx, cy - 5, cx, cy + 5, COLOR_HYPER);
    canvas.drawLine(cx - 5, cy, cx + 5, cy, COLOR_HYPER);
    canvas.drawLine(cx - 7, cy - 7, cx + 7, cy + 7, COLOR_HYPER);
    canvas.drawLine(cx + 7, cy - 7, cx - 7, cy + 7, COLOR_HYPER);
  }

  void drawIconSleepZ() {
    int cx = topIconCenterX() + 48;
    canvas.setTextColor(COLOR_SLEEP);
    canvas.setTextSize(2);
    canvas.setCursor(cx - 6, 8);
    canvas.print("Z");
  }

  void drawAlarmBell(int cx, int cy, uint32_t now) {
    static const int swingOffsets[8] = {-3, -2, 0, 2, 3, 2, 0, -2};
    int phase = (now / 100UL) % 8UL;
    int swing = swingOffsets[phase];
    int bx = cx + swing;
    int by = cy;

    if (phase <= 1 || phase >= 6) {
      canvas.drawLine(bx - 15, by + 7, bx - 11, by + 4, COLOR_SOFT);
      canvas.drawLine(bx + 11, by + 4, bx + 15, by + 7, COLOR_SOFT);
    } else if (phase >= 3 && phase <= 5) {
      canvas.drawLine(bx - 16, by + 10, bx - 11, by + 10, COLOR_SOFT);
      canvas.drawLine(bx + 11, by + 10, bx + 16, by + 10, COLOR_SOFT);
    }

    canvas.drawRoundRect(bx - 4, by - 8, 8, 4, 2, COLOR_SOFT);
    canvas.fillRoundRect(bx - 9, by - 1, 18, 12, 5, COLOR_SOFT);
    canvas.fillTriangle(bx - 11, by + 9, bx + 11, by + 9, bx, by - 1, COLOR_SOFT);
    canvas.drawFastHLine(bx - 10, by + 10, 20, COLOR_BG);
    canvas.fillCircle(bx + (swing / 2), by + 11, 2, COLOR_BG);
    canvas.drawPixel(bx - 3, by + 1, ST77XX_WHITE);
  }

  void drawIconDots() {
    canvas.setTextColor(COLOR_DIM);
    canvas.setTextSize(2);
    int cx = topIconCenterX();
    canvas.setCursor(cx - 10, 8);
    canvas.print("...");
  }

  void drawBrowsSurprised() {
    canvas.drawLine(36, eyeY - 10, 76, eyeY - 18, COLOR_SOFT);
    canvas.drawLine(144, eyeY - 18, 184, eyeY - 10, COLOR_SOFT);
  }

  void drawBrowsSleepy() {
    canvas.drawLine(34, eyeY - 4, 84, eyeY - 1, COLOR_SLEEP);
    canvas.drawLine(136, eyeY - 1, 186, eyeY - 4, COLOR_SLEEP);
  }

  void drawBrowsHappy() {
    canvas.drawLine(34, eyeY - 8, 74, eyeY - 14, COLOR_HAPPY);
    canvas.drawLine(146, eyeY - 14, 186, eyeY - 8, COLOR_HAPPY);
  }

  void drawBrowsLove() {
    canvas.drawLine(36, eyeY - 8, 76, eyeY - 12, COLOR_LOVE);
    canvas.drawLine(144, eyeY - 12, 184, eyeY - 8, COLOR_LOVE);
  }

  void drawBrowsOffended() {
    canvas.drawLine(34, eyeY - 2, 80, eyeY - 8, COLOR_DIM);
    canvas.drawLine(140, eyeY - 8, 186, eyeY - 2, COLOR_DIM);
  }


  void drawMouthNeutral() {
    int y = mouthBaseY();
    int x = mouthCenterX() - 13;
    canvas.drawRoundRect(x, y, 26, 7, 3, COLOR_MOUTH);
  }

  void drawMouthHappyWide() {
    int y = mouthBaseY() - happyMouthLift();
    int x = mouthCenterX() - 24;
    canvas.drawLine(x + 0,  y - 4, x + 8,  y + 3, COLOR_MOUTH);
    canvas.drawLine(x + 8,  y + 3, x + 16, y + 8, COLOR_MOUTH);
    canvas.drawLine(x + 16, y + 8, x + 24, y + 10, COLOR_MOUTH);
    canvas.drawLine(x + 24, y + 10, x + 32, y + 8, COLOR_MOUTH);
    canvas.drawLine(x + 32, y + 8, x + 40, y + 3, COLOR_MOUTH);
    canvas.drawLine(x + 40, y + 3, x + 48, y - 4, COLOR_MOUTH);
  }

  void drawMouthLoveSmile() {
    int y = mouthBaseY() - loveMouthLift();
    int x = mouthCenterX() - 20;
    canvas.drawLine(x + 0,  y - 2, x + 7,  y + 4, COLOR_MOUTH);
    canvas.drawLine(x + 7,  y + 4, x + 15, y + 8, COLOR_MOUTH);
    canvas.drawLine(x + 15, y + 8, x + 20, y + 9, COLOR_MOUTH);
    canvas.drawLine(x + 20, y + 9, x + 25, y + 8, COLOR_MOUTH);
    canvas.drawLine(x + 25, y + 8, x + 33, y + 4, COLOR_MOUTH);
    canvas.drawLine(x + 33, y + 4, x + 40, y - 2, COLOR_MOUTH);
    canvas.drawPixel(x + 19, y + 7, COLOR_SOFT);
    canvas.drawPixel(x + 20, y + 7, COLOR_SOFT);
  }

  void drawMouthSmallO() {
    int y = mouthBaseY();
    int x = mouthCenterX() - 5;
    canvas.drawRoundRect(x, y - 1, 10, 14, 4, COLOR_MOUTH);
  }

  void drawMouthBigYawn() {
    int y = mouthBaseY() - 4 + (mouthAnimPhase == 1 ? 1 : 0);
    int x = mouthCenterX() - 12;
    canvas.drawRoundRect(x, y, 24, 22, 8, COLOR_MOUTH);
    canvas.drawRoundRect(x + 1, y + 1, 22, 20, 8, COLOR_MOUTH);
  }

  void drawMouthSleep() {
    int y = mouthBaseY() + 3;
    int x = mouthCenterX() - 9;
    canvas.drawLine(x, y, x + 18, y, COLOR_MOUTH);
  }

  void drawMouthOffended() {
    int y = mouthBaseY() + 2;
    int x = mouthCenterX() - 8;
    canvas.drawLine(x, y, x + 16, y, COLOR_MOUTH);
  }

  void drawMouthAngryGrumble() {
    int y = mouthBaseY() + 2;
    int sx = angryMouthShift();
    int x = mouthCenterX() - 14 + sx;
    canvas.drawLine(x + 0,  y + 2, x + 7,  y - 1, COLOR_MOUTH);
    canvas.drawLine(x + 7,  y - 1, x + 14, y + 2, COLOR_MOUTH);
    canvas.drawLine(x + 14, y + 2, x + 21, y - 1, COLOR_MOUTH);
    canvas.drawLine(x + 21, y - 1, x + 28, y + 2, COLOR_MOUTH);
  }

  void drawEyelid(int x, int y, int w, int h, uint16_t lidColor) {
    int lidH = blinkLidHeight(h);
    if (lidH <= 0) return;
    canvas.fillRoundRect(x, y, w, lidH, 18, lidColor);
    canvas.fillRoundRect(x, y + h - lidH, w, lidH, 18, lidColor);
  }

  void drawIdleEyes(bool leftClosed, bool rightClosed) {
    fillSoftEye(leftEyeX, eyeY, eyeW, eyeH, COLOR_WHITE_EYE, COLOR_SOFT);
    fillSoftEye(rightEyeX, eyeY, eyeW, eyeH, COLOR_WHITE_EYE, COLOR_SOFT);

    int lpx = leftEyeX + eyeW / 2 - 12 + lookX + idleMicroX;
    int lpy = eyeY + eyeH / 2 - 18 + lookY + idleMicroY + loveLookDownOffset;
    int rpx = rightEyeX + eyeW / 2 - 12 + lookX + idleMicroX;
    int rpy = eyeY + eyeH / 2 - 18 + lookY + idleMicroY + loveLookDownOffset;

    drawPupilRect(lpx, lpy, COLOR_PUPIL);
    drawPupilRect(rpx, rpy, COLOR_PUPIL);

    if (leftClosed) drawEyelid(leftEyeX, eyeY, eyeW, eyeH, COLOR_BG);
    if (rightClosed) drawEyelid(rightEyeX, eyeY, eyeW, eyeH, COLOR_BG);
  }

  void drawIdleAura() {
    int pulse = glowOffset();
    canvas.drawRoundRect(12 - pulse, 12 - pulse, CANVAS_W - 24 + pulse * 2, CANVAS_H - 24 + pulse * 2, 26, COLOR_DIM);
    canvas.drawRoundRect(20, 20, CANVAS_W - 40, CANVAS_H - 40, 22, COLOR_DIM);
  }

  void drawStatusFooter(const char* text, uint16_t color) {
    (void)text;
    (void)color;
  }

  void renderBoot(float) {
    clearCanvas();

    uint32_t anim = millis() - stateEnteredAt;
    int pulse = (anim / 180) % 6;
    int y = 68 + ((pulse == 2 || pulse == 3) ? -2 : 0);

    canvas.setTextSize(4);
    canvas.setTextColor(COLOR_HYPER);
    canvas.setCursor(centeredX(24 * 4), y + 2);
    canvas.print("MIMO");

    canvas.setTextColor(ST77XX_WHITE);
    canvas.setCursor(centeredX(24 * 4), y);
    canvas.print("MIMO");
  }

  void renderIdle(float) {
    clearCanvas();
    resetFaceLayout();
    drawIdleEyes(eyesClosed(), eyesClosed());
    drawMouthNeutral();

    int s = glowOffset();
    canvas.fillRoundRect(leftEyeX + 14, eyeY + 12 + faceBobY, 16 + s, 8, 4, ST77XX_WHITE);
    canvas.fillRoundRect(rightEyeX + 14, eyeY + 12 + faceBobY, 16 + s, 8, 4, ST77XX_WHITE);
  }

  void renderHappy(float) {
    clearCanvas();
    useWideFaceLayout(WIDE_EYE_Y - 4, 24);

    int ly = eyeY;
    int lx = leftEyeX;
    int rx = rightEyeX;

    canvas.drawLine(lx,      ly, lx + 16, ly + 14, COLOR_HAPPY);
    canvas.drawLine(lx + 16, ly + 14, lx + 34, ly + 20, COLOR_HAPPY);
    canvas.drawLine(lx + 34, ly + 20, lx + 56, ly + 14, COLOR_HAPPY);
    canvas.drawLine(lx + 56, ly + 14, lx + 72, ly, COLOR_HAPPY);

    canvas.drawLine(rx,      ly, rx + 16, ly + 14, COLOR_HAPPY);
    canvas.drawLine(rx + 16, ly + 14, rx + 34, ly + 20, COLOR_HAPPY);
    canvas.drawLine(rx + 34, ly + 20, rx + 56, ly + 14, COLOR_HAPPY);
    canvas.drawLine(rx + 56, ly + 14, rx + 72, ly, COLOR_HAPPY);

    drawBrowsHappy();
    drawMouthHappyWide();
    drawBlushCentered(lx + eyeW / 2 - 16, eyeY + eyeH + 13);
    drawBlushCentered(rx + eyeW / 2 + 16, eyeY + eyeH + 13);
    drawIconSparkles();

    if (eyesClosed()) {
      canvas.fillRoundRect(lx, eyeY + 14, eyeW, 8, 4, COLOR_HAPPY);
      canvas.fillRoundRect(rx, eyeY + 14, eyeW, 8, 4, COLOR_HAPPY);
    }
  }

  void renderSurprised(float) {
    clearCanvas();
    resetFaceLayout();

    int w = eyeW, h = eyeH, y = eyeY, lx = leftEyeX, rx = rightEyeX;
    fillSoftEye(lx, y, w, h, COLOR_WHITE_EYE, COLOR_SOFT);
    fillSoftEye(rx, y, w, h, COLOR_WHITE_EYE, COLOR_SOFT);

    canvas.fillCircle(lx + w / 2, y + h / 2, 16, ST77XX_BLACK);
    canvas.fillCircle(rx + w / 2, y + h / 2, 16, ST77XX_BLACK);

    canvas.fillCircle(lx + w / 2 - 5, y + h / 2 - 5, 4, ST77XX_WHITE);
    canvas.fillCircle(rx + w / 2 - 5, y + h / 2 - 5, 4, ST77XX_WHITE);

    drawBrowsSurprised();
    drawMouthSmallO();
    drawIconExclaim();

    if (eyesClosed()) {
      drawEyelid(lx, y, w, h, COLOR_BG);
      drawEyelid(rx, y, w, h, COLOR_BG);
    }
  }

  void renderLoveTransition(float) {
    clearCanvas();
    resetFaceLayout();
    drawIdleEyes(false, false);
    drawBrowsLove();
    drawIconHeartTop();
    drawMouthLoveSmile();
  }

  void renderLove(float) {
    clearCanvas();
    resetFaceLayout();

    int heartSize = currentLoveHeartSize();

    fillSoftEye(leftEyeX, eyeY, eyeW, eyeH, COLOR_WHITE_EYE, COLOR_SOFT);
    fillSoftEye(rightEyeX, eyeY, eyeW, eyeH, COLOR_WHITE_EYE, COLOR_SOFT);

    drawHeart(leftEyeX + eyeW / 2 + (lookX / 4), eyeY + eyeH / 2 + 10 + (lookY / 6), heartSize, COLOR_LOVE);
    drawHeart(rightEyeX + eyeW / 2 + (lookX / 4), eyeY + eyeH / 2 + 10 + (lookY / 6), heartSize, COLOR_LOVE);

    canvas.fillCircle(leftEyeX + 18, eyeY + 18, 4, COLOR_HYPER);
    canvas.fillCircle(rightEyeX + 18, eyeY + 18, 4, COLOR_HYPER);

    drawBlushCentered(leftEyeX + eyeW / 2 - 18, eyeY + eyeH + 10);
    drawBlushCentered(rightEyeX + eyeW / 2 + 18, eyeY + eyeH + 10);

    int topCx = topIconCenterX();
    drawHeart(topCx - 38, 18 + ((millis() / 180) % 10), 5, COLOR_LOVE);
    drawHeart(topCx + 38, 8 + ((millis() / 210) % 12), 4, COLOR_LOVE);

    drawBrowsLove();
    drawIconHeartTop();
    drawMouthLoveSmile();

    if (eyesClosed()) {
      drawEyelid(leftEyeX, eyeY, eyeW, eyeH, COLOR_BG);
      drawEyelid(rightEyeX, eyeY, eyeW, eyeH, COLOR_BG);
    }
  }

  void renderAngry(float) {
    clearCanvas();
    resetFaceLayout();

    int shakeX = random(-angryLevel, angryLevel + 1);
    int shakeY = random(-angryLevel, angryLevel + 1);

    int y = eyeY + 34 + shakeY + (angrySquint / 3);
    int lx = leftEyeX + shakeX;
    int rx = rightEyeX + shakeX;
    int h = 54 - angryLevel * 2 - angrySquint;
    if (h < 40) h = 40;

    fillSoftEye(lx, y, eyeW, h, COLOR_WHITE_EYE, COLOR_SOFT);
    fillSoftEye(rx, y, eyeW, h, COLOR_WHITE_EYE, COLOR_SOFT);

    canvas.fillRoundRect(lx + 26 + lookX, y + 14 + lookY, 20, 22, 6, COLOR_ANGRY);
    canvas.fillRoundRect(rx + 26 + lookX, y + 14 + lookY, 20, 22, 6, COLOR_ANGRY);

    int lift = 8 + angryLevel * 3;
    canvas.drawLine(lx + 4, y - 6, lx + eyeW - 4, y - lift, COLOR_ANGRY);
    canvas.drawLine(lx + 4, y - 5, lx + eyeW - 4, y - lift + 1, COLOR_ANGRY);
    canvas.drawLine(rx + 4, y - lift, rx + eyeW - 4, y - 6, COLOR_ANGRY);
    canvas.drawLine(rx + 4, y - lift + 1, rx + eyeW - 4, y - 5, COLOR_ANGRY);

    drawMouthAngryGrumble();

    if (eyesClosed()) {
      drawEyelid(lx, y, eyeW, h, COLOR_BG);
      drawEyelid(rx, y, eyeW, h, COLOR_BG);
    }
  }

  void renderHyper(float) {
    clearCanvas();
    resetFaceLayout();

    int lx = leftEyeX;
    int rx = rightEyeX;
    int y = eyeY;

    fillSoftEye(lx, y, eyeW, eyeH, COLOR_WHITE_EYE, COLOR_SOFT);
    fillSoftEye(rx, y, eyeW, eyeH, COLOR_WHITE_EYE, COLOR_SOFT);

    int tremX1 = random(-2, 3);
    int tremY1 = random(-2, 3);
    int tremX2 = random(-2, 3);
    int tremY2 = random(-2, 3);

    int px1 = lx + eyeW / 2 - 12 + tremX1;
    int py1 = y + eyeH / 2 - 18 + tremY1;
    int px2 = rx + eyeW / 2 - 12 + tremX2;
    int py2 = y + eyeH / 2 - 18 + tremY2;

    drawPupilRect(px1, py1, COLOR_HYPER);
    drawPupilRect(px2, py2, COLOR_HYPER);

    drawMouthSmallO();
    drawIconSparkles();

    if (eyesClosed()) {
      drawEyelid(lx, y, eyeW, eyeH, COLOR_BG);
      drawEyelid(rx, y, eyeW, eyeH, COLOR_BG);
    }
  }

  void renderTired(float) {
    clearCanvas();
    resetFaceLayout();

    int y = eyeY + 6;
    int lx = leftEyeX;
    int rx = rightEyeX;
    int w = eyeW;
    int h = eyeH - 10;

    // normal eyes first
    fillSoftEye(lx, y, w, h, COLOR_WHITE_EYE, COLOR_SOFT);
    fillSoftEye(rx, y, w, h, COLOR_WHITE_EYE, COLOR_SOFT);

    int lpx = lx + w / 2 - 12;
    int lpy = y + h / 2 - 18 + 2;
    int rpx = rx + w / 2 - 12;
    int rpy = y + h / 2 - 18 + 2;
    drawPupilRect(lpx, lpy, COLOR_PUPIL);
    drawPupilRect(rpx, rpy, COLOR_PUPIL);

    // heavy tired lids
    int lid = 11 + sleepLidExtra;
    canvas.fillRoundRect(lx, y, w, lid, 12, COLOR_BG);
    canvas.fillRoundRect(rx, y, w, lid, 12, COLOR_BG);

    // red irritated rim around the lower eye and side veins
    int lowerY = y + h - 3;
    canvas.drawFastHLine(lx + 10, lowerY, w - 20, COLOR_ANGRY);
    canvas.drawFastHLine(rx + 10, lowerY, w - 20, COLOR_ANGRY);
    canvas.drawLine(lx + 8,  lowerY - 2, lx + 18, lowerY - 6, COLOR_ANGRY);
    canvas.drawLine(lx + 14, lowerY - 1, lx + 26, lowerY - 7, COLOR_ANGRY);
    canvas.drawLine(lx + w - 9, lowerY - 2, lx + w - 19, lowerY - 6, COLOR_ANGRY);
    canvas.drawLine(lx + w - 15, lowerY - 1, lx + w - 27, lowerY - 7, COLOR_ANGRY);
    canvas.drawLine(rx + 8,  lowerY - 2, rx + 18, lowerY - 6, COLOR_ANGRY);
    canvas.drawLine(rx + 14, lowerY - 1, rx + 26, lowerY - 7, COLOR_ANGRY);
    canvas.drawLine(rx + w - 9, lowerY - 2, rx + w - 19, lowerY - 6, COLOR_ANGRY);
    canvas.drawLine(rx + w - 15, lowerY - 1, rx + w - 27, lowerY - 7, COLOR_ANGRY);

    // subtle tired brows
    canvas.drawLine(lx + 10, y - 2, lx + w - 12, y - 5, COLOR_DIM);
    canvas.drawLine(rx + 12, y - 5, rx + w - 10, y - 2, COLOR_DIM);

    // flat tired mouth
    int mx = mouthCenterX();
    int my = mouthBaseY() + 6;
    canvas.drawLine(mx - 11, my, mx + 11, my, COLOR_MOUTH);

    // faint under-eye fatigue marks
    canvas.drawPixel(lx + w / 2 - 20, y + h + 7, COLOR_DIM);
    canvas.drawPixel(lx + w / 2 - 17, y + h + 9, COLOR_DIM);
    canvas.drawPixel(lx + w / 2 - 14, y + h + 11, COLOR_DIM);
    canvas.drawPixel(rx + w / 2 + 14, y + h + 7, COLOR_DIM);
    canvas.drawPixel(rx + w / 2 + 17, y + h + 9, COLOR_DIM);
    canvas.drawPixel(rx + w / 2 + 20, y + h + 11, COLOR_DIM);

    if (eyesClosed()) {
      drawEyelid(lx, y, w, h, COLOR_BG);
      drawEyelid(rx, y, w, h, COLOR_BG);
    }
  }

  void renderSleepy(float) {
    clearCanvas();
    useWideFaceLayout();

    int y = eyeY;
    int lx = leftEyeX;
    int rx = rightEyeX;
    int w = eyeW;

    canvas.fillRoundRect(lx, y, w, eyeH, 12, COLOR_SLEEP);
    canvas.fillRoundRect(rx, y, w, eyeH, 12, COLOR_SLEEP);

    canvas.fillRoundRect(lx + 24, y + 5, 20, 10, 5, ST77XX_BLACK);
    canvas.fillRoundRect(rx + 24, y + 5, 20, 10, 5, ST77XX_BLACK);

    if (sleepLidExtra > 0) {
      canvas.fillRoundRect(lx, y, w, sleepLidExtra, 10, COLOR_BG);
      canvas.fillRoundRect(rx, y, w, sleepLidExtra, 10, COLOR_BG);
    }

    drawBrowsSleepy();
    drawMouthSleep();
    drawIconSleepZ();

    if (eyesClosed()) {
      drawEyelid(lx, y, w, eyeH, COLOR_BG);
      drawEyelid(rx, y, w, eyeH, COLOR_BG);
    }
  }

  void renderSleep(float) {
    clearCanvas();
    useWideFaceLayout(WIDE_EYE_Y + 8, 8 + sleepLidExtra);

    int breath = currentSleepBreathOffset();
    int lx = leftEyeX;
    int rx = rightEyeX;
    int y = eyeY + breath;
    int w = eyeW;

    canvas.fillRoundRect(lx, y, w, eyeH, 4, COLOR_SLEEP);
    canvas.fillRoundRect(rx, y, w, eyeH, 4, COLOR_SLEEP);

    drawBrowsSleepy();
    drawMouthSleep();
    drawIconSleepZ();

    canvas.setTextColor(COLOR_SLEEP);
    canvas.setTextSize(2);
    canvas.setCursor(CANVAS_W - 44, 18);
    canvas.print("Z");
    if (sleepZPhase >= 1) {
      canvas.setTextSize(1);
      canvas.setCursor(CANVAS_W - 24, 10);
      canvas.print("z");
    }
    if (sleepZPhase >= 2) {
      canvas.setCursor(CANVAS_W - 12, 4);
      canvas.print("z");
    }

  }

  void renderWakeScan(float, uint32_t now) {
    clearCanvas();
    resetFaceLayout();

    int phase = (now / 180UL) % 4UL;
    int scanX = 0;
    int scanY = 0;
    if (phase == 0) scanX = -5;
    else if (phase == 1) scanX = 5;
    else if (phase == 2) scanY = -2;

    fillSoftEye(leftEyeX, eyeY, eyeW, eyeH, COLOR_WHITE_EYE, COLOR_SOFT);
    fillSoftEye(rightEyeX, eyeY, eyeW, eyeH, COLOR_WHITE_EYE, COLOR_SOFT);

    drawPupilRect(leftEyeX + eyeW / 2 - 12 + scanX, eyeY + eyeH / 2 - 18 + scanY, COLOR_PUPIL);
    drawPupilRect(rightEyeX + eyeW / 2 - 12 + scanX, eyeY + eyeH / 2 - 18 + scanY, COLOR_PUPIL);

    drawBrowsSurprised();
    drawMouthSmallO();
    drawAlarmBell(CANVAS_W / 2, 18, now);
  }
  void drawPixelDigitReveal(int x, int y, uint8_t digit, int scale, uint16_t color, int revealHeight) {
    static const uint8_t glyphs[10][5] = {
      {0b111,0b101,0b101,0b101,0b111},
      {0b010,0b110,0b010,0b010,0b111},
      {0b111,0b001,0b111,0b100,0b111},
      {0b111,0b001,0b111,0b001,0b111},
      {0b101,0b101,0b111,0b001,0b001},
      {0b111,0b100,0b111,0b001,0b111},
      {0b111,0b100,0b111,0b101,0b111},
      {0b111,0b001,0b001,0b001,0b001},
      {0b111,0b101,0b111,0b101,0b111},
      {0b111,0b101,0b111,0b001,0b111}
    };

    if (digit > 9 || revealHeight <= 0) return;
    for (int row = 0; row < 5; ++row) {
      for (int col = 0; col < 3; ++col) {
        if (glyphs[digit][row] & (1 << (2 - col))) {
          int py = y + row * scale;
          int visible = revealHeight - (row * scale);
          if (visible <= 0) continue;
          int h = min(scale - 1, visible);
          if (h > 0) canvas.fillRect(x + col * scale, py, scale - 1, h, color);
        }
      }
    }
  }

  void drawPixelColonReveal(int x, int y, int scale, uint16_t color, bool on, int revealHeight) {
    if (!on || revealHeight <= 0) return;
    int dot = max(2, scale - 2);
    int dotY1 = scale;
    int dotY2 = scale * 3;
    int visible1 = revealHeight - dotY1;
    int visible2 = revealHeight - dotY2;
    if (visible1 > 0) canvas.fillRect(x, y + dotY1, dot, min(dot, visible1), color);
    if (visible2 > 0) canvas.fillRect(x, y + dotY2, dot, min(dot, visible2), color);
  }

  void drawPixelDigit(int x, int y, uint8_t digit, int scale, uint16_t color) {
    static const uint8_t glyphs[10][5] = {
      {0b111,0b101,0b101,0b101,0b111},
      {0b010,0b110,0b010,0b010,0b111},
      {0b111,0b001,0b111,0b100,0b111},
      {0b111,0b001,0b111,0b001,0b111},
      {0b101,0b101,0b111,0b001,0b001},
      {0b111,0b100,0b111,0b001,0b111},
      {0b111,0b100,0b111,0b101,0b111},
      {0b111,0b001,0b001,0b001,0b001},
      {0b111,0b101,0b111,0b101,0b111},
      {0b111,0b101,0b111,0b001,0b111}
    };

    if (digit > 9) return;
    for (int row = 0; row < 5; ++row) {
      for (int col = 0; col < 3; ++col) {
        if (glyphs[digit][row] & (1 << (2 - col))) {
          canvas.fillRect(x + col * scale, y + row * scale, scale - 1, scale - 1, color);
        }
      }
    }
  }

  void drawPixelColon(int x, int y, int scale, uint16_t color, bool on) {
    if (!on) return;
    int dot = max(2, scale - 2);
    canvas.fillRect(x, y + scale, dot, dot, color);
    canvas.fillRect(x, y + scale * 3, dot, dot, color);
  }

  void drawPixelBattery(int x, int y, int pct) {
    canvas.drawRect(x, y, 22, 10, COLOR_SOFT);
    canvas.fillRect(x + 22, y + 3, 2, 4, COLOR_SOFT);
    int bars = constrain((pct + 19) / 20, 0, 5);
    for (int i = 0; i < 5; ++i) {
      int bx = x + 2 + i * 4;
      if (i < bars) canvas.fillRect(bx, y + 2, 3, 6, (pct > 20) ? COLOR_SOFT : COLOR_LOW_BAT);
      else canvas.drawRect(bx, y + 2, 3, 6, COLOR_DIM);
    }
  }

  void drawScanlines() {
    for (int y = 0; y < CANVAS_H; y += 4) {
      canvas.drawFastHLine(0, y, CANVAS_W, COLOR_DIM);
    }
  }

  void drawClockAlarmOverlay(uint32_t now) {
    drawAlarmBell(CANVAS_W / 2, 16, now);
    int y = CANVAS_H - 12;
    int phase = (now / 180UL) % 4UL;
    int pulse = (phase == 1 || phase == 3) ? 2 : 0;
    canvas.drawFastHLine(26, y, CANVAS_W - 52, COLOR_DIM);
    canvas.drawFastHLine(26, y + 1, CANVAS_W - 52, COLOR_DIM);
    canvas.drawFastHLine(26 + pulse, y, 18, COLOR_HAPPY);
    canvas.drawFastHLine(CANVAS_W - 44 - pulse, y, 18, COLOR_HAPPY);
  }

  void renderClock(float, uint32_t now) {
    clearCanvas();
    uint8_t hh, mm, ss;
    getClockTime(now, hh, mm, ss);

    const int scale = 12;
    const int digitW = 3 * scale;
    const int digitGap = scale;
    const int middleGap = scale * 3;
    const int totalW = digitW * 4 + digitGap * 2 + middleGap;
    const int startX = (CANVAS_W - totalW) / 2;
    const int digitH = 5 * scale;
    const int colonDot = max(2, scale - 2);

    uint32_t clockAnimMs = now - stateEnteredAt;

    if (clockAnimMs < 420) {
      float p = easeOutCubic(min(1.0f, clockAnimMs / 420.0f));

      const int eyeStartW = 72;
      const int eyeStartH = 24;
      const int eyeEndW = (CANVAS_W - 44) / 2;
      const int gapStart = 28;
      const int gapEnd = 4;

      int h = max(2, (int)(eyeStartH * (1.0f - p)) + 1);
      int w = (int)(eyeStartW + (eyeEndW - eyeStartW) * p);
      int gap = (int)(gapStart + (gapEnd - gapStart) * p);
      int y = (CANVAS_H / 2) - h / 2 + 2;

      int lx = CANVAS_W / 2 - gap / 2 - w;
      int rx = CANVAS_W / 2 + gap / 2;

      uint16_t eyeColor = (clockAnimMs < 180) ? COLOR_SLEEP : COLOR_SOFT;
      canvas.fillRoundRect(lx, y, w, h, 4, eyeColor);
      canvas.fillRoundRect(rx, y, w, h, 4, eyeColor);

      if (clockAnimMs < 130 && h > 8) {
        canvas.fillRoundRect(lx + w / 2 - 8, y + h / 2 - 2, 16, 4, 2, ST77XX_BLACK);
        canvas.fillRoundRect(rx + w / 2 - 8, y + h / 2 - 2, 16, 4, 2, ST77XX_BLACK);
      }

      return;
    }

    float t = easeOutCubic(min(1.0f, (clockAnimMs - 420) / 720.0f));
    int revealHeight = max(1, (int)(digitH * t));
    int rise = (int)((1.0f - t) * 12.0f);
    const int y = (CANVAS_H - digitH) / 2 - 4 + rise;

    bool colonOn = ((now / 500UL) % 2UL) == 0;
    bool glitch = clockAnimMs > 1300 && ((now / 137UL) % 211UL) == 0;
    int gx = glitch ? 1 : 0;

    uint16_t mainColor = ST77XX_WHITE;
    uint16_t glowColor = COLOR_DIM;

    uint8_t d1 = hh / 10;
    uint8_t d2 = hh % 10;
    uint8_t d3 = mm / 10;
    uint8_t d4 = mm % 10;

    int d1x = startX + gx;
    int d2x = d1x + digitW + digitGap;
    int d3x = d2x + digitW + middleGap;
    int d4x = d3x + digitW + digitGap;

    int colonX = ((d2x + digitW) + d3x) / 2 - (colonDot / 2);

    drawPixelDigitReveal(d1x + 2, y + 2, d1, scale, glowColor, revealHeight);
    drawPixelDigitReveal(d1x, y, d1, scale, mainColor, revealHeight);

    drawPixelDigitReveal(d2x + 2, y + 2, d2, scale, glowColor, revealHeight);
    drawPixelDigitReveal(d2x, y, d2, scale, mainColor, revealHeight);

    drawPixelColonReveal(colonX + 1, y + 3, scale, glowColor, colonOn, revealHeight);
    drawPixelColonReveal(colonX, y + 1, scale, mainColor, colonOn, revealHeight);

    drawPixelDigitReveal(d3x + 2, y + 2, d3, scale, glowColor, revealHeight);
    drawPixelDigitReveal(d3x, y, d3, scale, mainColor, revealHeight);

    drawPixelDigitReveal(d4x + 2, y + 2, d4, scale, glowColor, revealHeight);
    drawPixelDigitReveal(d4x, y, d4, scale, mainColor, revealHeight);

    if (clockAnimMs > 880) {
      int secTrackW = CANVAS_W - 46;
      int secBarW = map(ss, 0, 59, 0, secTrackW);
      int secY = CANVAS_H - 11;

      canvas.fillRoundRect(23, secY, secTrackW, 6, 3, COLOR_DIM);
      if (secBarW > 0) {
        int shown = (int)(secBarW * min(1.0f, t * 1.15f));
        canvas.fillRoundRect(23, secY, shown, 6, 3, ntpSynced ? COLOR_HYPER : COLOR_SOFT);
      }
    }
  }


  void renderAlarmScreen(float, uint32_t now) {
    clearCanvas();

    uint8_t hh, mm, ss;
    getClockTime(now, hh, mm, ss);

    // clean top zone: bell + time without collisions
    drawAlarmBell(CANVAS_W / 2, 8, now);

    const int scale = 5;
    const int digitW = 3 * scale;
    const int digitGap = scale;
    const int middleGap = scale * 2;
    const int totalW = digitW * 4 + digitGap * 2 + middleGap;
    const int startX = (CANVAS_W - totalW) / 2;
    const int yTime = 22;
    const int colonDot = max(2, scale - 2);

    uint8_t d1 = hh / 10;
    uint8_t d2 = hh % 10;
    uint8_t d3 = mm / 10;
    uint8_t d4 = mm % 10;

    int d1x = startX;
    int d2x = d1x + digitW + digitGap;
    int d3x = d2x + digitW + middleGap;
    int d4x = d3x + digitW + digitGap;
    int colonX = ((d2x + digitW) + d3x) / 2 - (colonDot / 2);
    bool colonOn = ((now / 450UL) % 2UL) == 0;

    drawPixelDigit(d1x + 1, yTime + 1, d1, scale, COLOR_DIM);
    drawPixelDigit(d1x, yTime, d1, scale, ST77XX_WHITE);
    drawPixelDigit(d2x + 1, yTime + 1, d2, scale, COLOR_DIM);
    drawPixelDigit(d2x, yTime, d2, scale, ST77XX_WHITE);
    drawPixelColon(colonX + 1, yTime + 1, scale, COLOR_DIM, colonOn);
    drawPixelColon(colonX, yTime, scale, COLOR_HYPER, colonOn);
    drawPixelDigit(d3x + 1, yTime + 1, d3, scale, COLOR_DIM);
    drawPixelDigit(d3x, yTime, d3, scale, ST77XX_WHITE);
    drawPixelDigit(d4x + 1, yTime + 1, d4, scale, COLOR_DIM);
    drawPixelDigit(d4x, yTime, d4, scale, ST77XX_WHITE);

    // separator kept above face, not crossing any element
    int sepY = 50;
    int sepPulse = (((now / 180UL) % 2UL) == 0) ? 0 : 3;
    canvas.drawFastHLine(26, sepY, CANVAS_W - 52, COLOR_DIM);
    canvas.drawFastHLine(26, sepY + 1, 42 + sepPulse, COLOR_HAPPY);
    canvas.drawFastHLine(CANVAS_W - 26 - (42 + sepPulse), sepY + 1, 42 + sepPulse, COLOR_HAPPY);

    // face sits lower to leave breathing room
    resetFaceLayout();
    eyeY = 62;
    int breath = ((now / 240UL) % 4UL == 1 || (now / 240UL) % 4UL == 2) ? 1 : 0;
    int openBoost = ((now / 160UL) % 2UL) ? 2 : 0;
    int lx = leftEyeX;
    int rx = rightEyeX;
    int y = eyeY + breath;
    int h = eyeH + openBoost;

    fillSoftEye(lx, y, eyeW, h, COLOR_WHITE_EYE, COLOR_SOFT);
    fillSoftEye(rx, y, eyeW, eyeH + openBoost, COLOR_WHITE_EYE, COLOR_SOFT);

    int pulseLook = ((now / 220UL) % 2UL) ? 1 : -1;
    drawPupilRect(lx + eyeW / 2 - 12 + pulseLook, y + h / 2 - 18 + 1, COLOR_PUPIL);
    drawPupilRect(rx + eyeW / 2 - 12 + pulseLook, y + h / 2 - 18 + 1, COLOR_PUPIL);

    // brows with safe spacing from time strip
    canvas.drawLine(lx + 10, y - 8, lx + eyeW - 12, y - 12, COLOR_HAPPY);
    canvas.drawLine(rx + 12, y - 12, rx + eyeW - 10, y - 8, COLOR_HAPPY);

    // compact smile with enough room for footer
    int mx = mouthCenterX();
    int my = 146 + ((now / 280UL) % 2UL);
    canvas.drawLine(mx - 16, my - 1, mx - 9, my + 4, COLOR_MOUTH);
    canvas.drawLine(mx - 9, my + 4, mx, my + 6, COLOR_MOUTH);
    canvas.drawLine(mx, my + 6, mx + 9, my + 4, COLOR_MOUTH);
    canvas.drawLine(mx + 9, my + 4, mx + 16, my - 1, COLOR_MOUTH);

    // footer hint fully clear of mouth
    uint16_t hintColor = (((now / 300UL) % 2UL) == 0) ? COLOR_DIM : COLOR_SOFT;
    canvas.setTextColor(hintColor);
    canvas.setTextSize(1);
    canvas.setCursor(centeredX(11 * 6), CANVAS_H - 9);
    canvas.print("tap to stop");
  }

  void renderLowBattery(float) {
    clearCanvas();
    useWideFaceLayout(WIDE_EYE_Y + 2, 20);

    int y = eyeY;
    int lx = leftEyeX;
    int rx = rightEyeX;
    int w = eyeW;

    canvas.fillRoundRect(lx, y, w, eyeH, 10, COLOR_LOW_BAT);
    canvas.fillRoundRect(rx, y, w, eyeH, 10, COLOR_LOW_BAT);

    canvas.fillRoundRect(lx + 24, y + 6, 18, 8, 4, ST77XX_BLACK);
    canvas.fillRoundRect(rx + 24, y + 6, 18, 8, 4, ST77XX_BLACK);

    drawMouthSleep();
    drawIconDots();
  }

  void renderYawn(float) {
    clearCanvas();
    useWideFaceLayout(WIDE_EYE_Y + 2, 20);

    int sleepyY = eyeY;
    canvas.fillRoundRect(leftEyeX, sleepyY, eyeW, eyeH, 10, COLOR_SLEEP);
    canvas.fillRoundRect(rightEyeX, sleepyY, eyeW, eyeH, 10, COLOR_SLEEP);

    canvas.fillRoundRect(leftEyeX + 22, sleepyY + 6, 18, 8, 4, ST77XX_BLACK);
    canvas.fillRoundRect(rightEyeX + 22, sleepyY + 6, 18, 8, 4, ST77XX_BLACK);

    drawBrowsSleepy();
    drawMouthBigYawn();
    drawIconSleepZ();
  }

  void renderOffended(float) {
    clearCanvas();
    resetFaceLayout();

    fillSoftEye(leftEyeX, eyeY + 4, eyeW, eyeH - 14, COLOR_WHITE_EYE, COLOR_SOFT);
    fillSoftEye(rightEyeX, eyeY + 4, eyeW, eyeH - 14, COLOR_WHITE_EYE, COLOR_SOFT);

    int lpx = leftEyeX + eyeW / 2 - 18;
    int lpy = eyeY + eyeH / 2 - 16;
    int rpx = rightEyeX + eyeW / 2 - 18;
    int rpy = eyeY + eyeH / 2 - 16;

    drawPupilRect(lpx, lpy, COLOR_PUPIL);
    drawPupilRect(rpx, rpy, COLOR_PUPIL);

    canvas.fillRoundRect(leftEyeX, eyeY + 4, eyeW, 12, 10, COLOR_BG);
    canvas.fillRoundRect(rightEyeX, eyeY + 4, eyeW, 12, 10, COLOR_BG);

    drawBrowsOffended();
    drawMouthOffended();
    drawIconDots();
  }

  void renderRebooting(float) {
    clearCanvas();

    int panelW = 164;
    int panelH = 108;
    int panelX = centeredX(panelW);
    int panelY = 38;
    int pulse = ((millis() / 180) % 2) ? 2 : 0;

    canvas.drawRoundRect(panelX - pulse, panelY - pulse, panelW + pulse * 2, panelH + pulse * 2, 18, COLOR_DIM);
    canvas.drawRoundRect(panelX, panelY, panelW, panelH, 18, COLOR_BOOT);

    int eyeY2 = panelY + 34;
    canvas.fillRoundRect(panelX + 28, eyeY2, 34, 12, 5, COLOR_WHITE_EYE);
    canvas.fillRoundRect(panelX + 102, eyeY2, 34, 12, 5, COLOR_WHITE_EYE);
    canvas.fillRoundRect(panelX + 40 + pulse, eyeY2 + 2, 10, 8, 4, COLOR_HYPER);
    canvas.fillRoundRect(panelX + 114 + pulse, eyeY2 + 2, 10, 8, 4, COLOR_HYPER);

    canvas.drawLine(panelX + 68, panelY + 74, panelX + 82, panelY + 80, COLOR_SOFT);
    canvas.drawLine(panelX + 82, panelY + 80, panelX + 96, panelY + 74, COLOR_SOFT);

    canvas.fillRoundRect(centeredX(108), 164, 108, 8, 4, COLOR_DIM);
    int p = (millis() / 120) % 100;
    int pw = map(p, 0, 99, 10, 100);
    canvas.fillRoundRect(centeredX(108) + 4, 166, pw, 4, 2, COLOR_HYPER);
  }

  void render(RobotState state, uint32_t now, float batteryVoltage) {
    uint32_t frameNow = millis();
    (void)now; // use live frame time so alarm/clock animation never freezes on stale caller timestamps

    if (state != lastState) {
      stateEnteredAt = frameNow;
      lastState = state;
      dirty = true;
    }

    resetFaceLayout();
    updateLook(frameNow);
    updateBlink(frameNow);
    updateGlowPulse(frameNow, state);
    updateMouthAnim(state, frameNow);
    updateIdleDoubleBlink(frameNow, state);
    updateLoveLookDown(frameNow, state);
    updateAngrySquint(frameNow, state);
    updateHyperZoom(frameNow, state);
    updateSleepLid(frameNow, state);

    if (state == RobotState::IDLE) updateIdleMicroMotion(frameNow);
    if (state == RobotState::SLEEP) updateSleepAnim(frameNow);
    if (state == RobotState::LOVE) updateLovePulse(frameNow);
    if (state == RobotState::CLOCK || state == RobotState::WAKE_SCAN || alarmRinging) dirty = true;

    if (!dirty) return;
    if (frameNow - lastRenderAt < Config::FRAME_INTERVAL_MS) return;

    lastRenderAt = frameNow;
    dirty = false;

    if (alarmRinging) {
      renderAlarmScreen(batteryVoltage, frameNow);
    } else {
      switch (state) {
        case RobotState::BOOT: renderBoot(batteryVoltage); break;
        case RobotState::IDLE: renderIdle(batteryVoltage); break;
        case RobotState::HAPPY: renderHappy(batteryVoltage); break;
        case RobotState::SURPRISED: renderSurprised(batteryVoltage); break;
        case RobotState::LOVE_TRANSITION: renderLoveTransition(batteryVoltage); break;
        case RobotState::LOVE: renderLove(batteryVoltage); break;
        case RobotState::ANGRY_1:
        case RobotState::ANGRY_2:
        case RobotState::ANGRY_3: renderAngry(batteryVoltage); break;
        case RobotState::HYPER: renderHyper(batteryVoltage); break;
        case RobotState::TIRED: renderTired(batteryVoltage); break;
        case RobotState::SLEEPY: renderSleepy(batteryVoltage); break;
        case RobotState::SLEEP: renderSleep(batteryVoltage); break;
        case RobotState::CLOCK: renderClock(batteryVoltage, frameNow); break;
        case RobotState::WAKE_SCAN: renderWakeScan(batteryVoltage, frameNow); break;
        case RobotState::LOW_BATTERY: renderLowBattery(batteryVoltage); break;
        case RobotState::YAWN: renderYawn(batteryVoltage); break;
        case RobotState::OFFENDED: renderOffended(batteryVoltage); break;
        case RobotState::REBOOTING: renderRebooting(batteryVoltage); break;
        case RobotState::ALARM_RINGING: renderAlarmScreen(batteryVoltage, frameNow); break;
      }
    }

    pushCanvas();
    if (!alarmRinging && state != RobotState::BOOT) {
      renderHud(batteryVoltage, frameNow);
    } else {
      tft.fillRect(0, 0, 50, 24, COLOR_BG);
    }

    if (frameNow - lastDebugAt > 1000) {
      lastDebugAt = frameNow;
      Serial.print("[DISPLAY] state=");
      Serial.print(stateName(state));
      Serial.print(" look=(");
      Serial.print(lookX);
      Serial.print(",");
      Serial.print(lookY);
      Serial.print(") blinkPhase=");
      Serial.println(blinkPhase);
    }
  }
};