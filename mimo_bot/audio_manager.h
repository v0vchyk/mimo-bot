#pragma once

#include <Arduino.h>
#include "robot_types.h"

enum class SoundEffect : uint8_t {
  NONE,
  BOOT,
  HAPPY,
  SURPRISED,
  LOVE_INTRO,
  LOVE_LOOP,
  LOVE_SOFT,
  ANGRY_1,
  ANGRY_2,
  ANGRY_3,
  HYPER,
  SLEEPY,
  SLEEP,
  SLEEP_SOFT,
  WAKEUP,
  ALARM_SOFT,
  ALARM_MEDIUM,
  ALARM_STRONG,
  LOW_BATTERY,
  YAWN,
  OFFENDED,
  REBOOTING
};

const char* soundName(SoundEffect s);

class AudioManager {
public:
  struct PatternStep {
    float freqStartHz;
    float freqEndHz;
    uint16_t durationMs;
    float amplitude;      // 0.0 .. 1.0
    uint16_t attackMs;    // fade-in
    uint16_t releaseMs;   // fade-out
    bool silence;
  };

  void begin();
  void update(uint32_t now);

  void setMuted(bool value);
  bool isMuted() const { return muted; }
  bool isPlaying() const { return playing; }
  bool isBusy() const { return playing; }

  void setMasterVolume(float value);   // 0.0 .. 1.0
  float getMasterVolume() const { return masterVolume; }

  void play(SoundEffect sound);
  void playForState(RobotState state);
  void playWakeup();
  void playAlarmSoft()   { play(SoundEffect::ALARM_SOFT); }
  void playAlarmMedium() { play(SoundEffect::ALARM_MEDIUM); }
  void playAlarmStrong() { play(SoundEffect::ALARM_STRONG); }
  void stop();

  SoundEffect stateToSound(RobotState state) const;

private:
  bool initialized = false;
  bool muted = false;

  SoundEffect currentSound = SoundEffect::NONE;
  SoundEffect lastPlayed = SoundEffect::NONE;

  bool playing = false;
  bool looped = false;

  uint32_t soundStartedAt = 0;
  uint32_t patternStepAt = 0;

  int patternStep = 0;
  uint16_t stepRemainingMs = 0;
  uint16_t stepElapsedMs = 0;

  float phase = 0.0f;
  float masterVolume = 0.34f;

  const PatternStep* activePattern = nullptr;
  uint8_t activePatternLength = 0;

  void startPattern(SoundEffect sound);
  void advanceToNextStep();
  void writeToneChunk(const PatternStep& step, uint16_t durationMs);
  void writeSilenceChunk(uint16_t durationMs);
};