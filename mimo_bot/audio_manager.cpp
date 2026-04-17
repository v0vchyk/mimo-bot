#include "audio_manager.h"

#include <math.h>
#include <driver/i2s.h>

static constexpr i2s_port_t AUDIO_I2S_PORT = I2S_NUM_0;
static constexpr uint32_t SAMPLE_RATE = 16000;
static constexpr float PI_F = 3.14159265f;
static constexpr uint16_t AUDIO_SLICE_MS = 8;

namespace {
using Step = AudioManager::PatternStep;

constexpr Step PATTERN_BOOT[] = {
  {620.0f, 820.0f, 70, 0.18f, 10, 18, false},
  {0.0f,   0.0f,   16, 0.0f,  0,  0,  true },
  {860.0f, 1180.0f,82, 0.20f, 10, 20, false},
  {0.0f,   0.0f,   18, 0.0f,  0,  0,  true },
  {1260.0f,1560.0f,98, 0.22f, 12, 24, false},
};

constexpr Step PATTERN_HAPPY[] = {
  {1040.0f,1200.0f, 52, 0.18f, 8, 14, false},
  {0.0f,   0.0f,    12, 0.0f,  0,  0,  true },
  {1320.0f,1480.0f, 52, 0.18f, 8, 14, false},
  {0.0f,   0.0f,    12, 0.0f,  0,  0,  true },
  {1560.0f,1880.0f, 72, 0.22f,10, 18, false},
};

constexpr Step PATTERN_SURPRISED[] = {
  {640.0f, 720.0f,  34, 0.13f, 6,  8, false},
  {0.0f,   0.0f,    12, 0.0f,  0,  0, true },
  {1480.0f,1780.0f,118, 0.20f,10, 26, false},
};

constexpr Step PATTERN_LOVE_INTRO[] = {
  {760.0f, 920.0f,  60, 0.14f,10, 18, false},
  {0.0f,   0.0f,    18, 0.0f, 0,  0,  true },
  {980.0f, 1160.0f, 66, 0.16f,10, 18, false},
  {0.0f,   0.0f,    24, 0.0f, 0,  0,  true },
  {1180.0f,1360.0f, 74, 0.18f,12, 20, false},
  {0.0f,   0.0f,    18, 0.0f, 0,  0,  true },
  {1320.0f,1540.0f, 98, 0.19f,12, 26, false},
};

constexpr Step PATTERN_LOVE_SOFT[] = {
  {780.0f, 860.0f,  54, 0.10f,10, 18, false},
  {0.0f,   0.0f,    26, 0.0f, 0,  0,  true },
  {980.0f, 1080.0f, 62, 0.11f,10, 18, false},
};

constexpr Step PATTERN_ANGRY_1[] = {
  {520.0f, 420.0f,  84, 0.20f, 6, 14, false},
  {0.0f,   0.0f,    10, 0.0f,  0,  0, true },
  {440.0f, 340.0f,  76, 0.19f, 6, 14, false},
};

constexpr Step PATTERN_ANGRY_2[] = {
  {460.0f, 360.0f,  78, 0.22f, 6, 14, false},
  {0.0f,   0.0f,     8, 0.0f,  0,  0, true },
  {400.0f, 300.0f,  78, 0.22f, 6, 14, false},
  {0.0f,   0.0f,     8, 0.0f,  0,  0, true },
  {340.0f, 260.0f,  88, 0.23f, 6, 18, false},
};

constexpr Step PATTERN_ANGRY_3[] = {
  {420.0f, 300.0f,  84, 0.24f, 6, 14, false},
  {0.0f,   0.0f,     8, 0.0f,  0,  0, true },
  {340.0f, 240.0f,  84, 0.24f, 6, 14, false},
  {0.0f,   0.0f,     8, 0.0f,  0,  0, true },
  {280.0f, 190.0f,  94, 0.25f, 6, 20, false},
};

constexpr Step PATTERN_HYPER[] = {
  {1420.0f,1680.0f, 30, 0.16f, 4,  8, false},
  {0.0f,   0.0f,     8, 0.0f,  0,  0, true },
  {1760.0f,2040.0f, 30, 0.17f, 4,  8, false},
  {0.0f,   0.0f,     8, 0.0f,  0,  0, true },
  {2100.0f,2360.0f, 30, 0.18f, 4,  8, false},
  {0.0f,   0.0f,     8, 0.0f,  0,  0, true },
  {1840.0f,2100.0f, 34, 0.17f, 4, 10, false},
};

constexpr Step PATTERN_SLEEPY[] = {
  {520.0f, 460.0f,  90, 0.12f,14, 28, false},
  {0.0f,   0.0f,    20, 0.0f, 0,  0,  true },
  {430.0f, 370.0f, 108, 0.11f,14, 34, false},
};

constexpr Step PATTERN_SLEEP_SOFT[] = {
  {280.0f, 250.0f, 110, 0.08f,18, 40, false},
};

constexpr Step PATTERN_WAKEUP[] = {
  {740.0f, 980.0f,  46, 0.15f, 8, 10, false},
  {0.0f,   0.0f,    10, 0.0f,  0,  0, true },
  {1040.0f,1280.0f, 54, 0.18f, 8, 12, false},
  {0.0f,   0.0f,    12, 0.0f,  0,  0, true },
  {1360.0f,1640.0f, 74, 0.20f,10, 18, false},
};

constexpr Step PATTERN_ALARM_SOFT[] = {
  {540.0f, 620.0f, 110, 0.18f, 8, 22, false},
  {0.0f,   0.0f,    72, 0.0f,  0,  0, true },
  {700.0f, 820.0f, 120, 0.18f, 8, 24, false},
  {0.0f,   0.0f,   140, 0.0f,  0,  0, true },
};

constexpr Step PATTERN_ALARM_MEDIUM[] = {
  {640.0f, 760.0f, 100, 0.22f, 8, 18, false},
  {0.0f,   0.0f,    52, 0.0f,  0,  0, true },
  {860.0f, 980.0f, 112, 0.23f, 8, 20, false},
  {0.0f,   0.0f,    56, 0.0f,  0,  0, true },
  {1060.0f,1180.0f,124, 0.24f,10, 22, false},
};

constexpr Step PATTERN_ALARM_STRONG[] = {
  {760.0f, 920.0f,  88, 0.26f, 6, 16, false},
  {0.0f,   0.0f,    38, 0.0f,  0,  0, true },
  {980.0f, 1160.0f, 92, 0.27f, 6, 16, false},
  {0.0f,   0.0f,    38, 0.0f,  0,  0, true },
  {1240.0f,1440.0f,112, 0.28f, 8, 18, false},
};

constexpr Step PATTERN_LOW_BATTERY[] = {
  {460.0f, 420.0f, 130, 0.16f,10, 28, false},
  {0.0f,   0.0f,    60, 0.0f,  0,  0, true },
  {390.0f, 340.0f, 150, 0.17f,10, 34, false},
};

constexpr Step PATTERN_YAWN[] = {
  {560.0f, 460.0f, 120, 0.14f,14, 28, false},
  {0.0f,   0.0f,    22, 0.0f,  0,  0, true },
  {420.0f, 300.0f, 150, 0.13f,16, 40, false},
};

constexpr Step PATTERN_OFFENDED[] = {
  {420.0f, 360.0f, 150, 0.14f,10, 36, false},
};

constexpr Step PATTERN_REBOOTING[] = {
  {840.0f, 720.0f,  56, 0.16f, 8, 14, false},
  {0.0f,   0.0f,    14, 0.0f,  0,  0, true },
  {620.0f, 480.0f,  72, 0.16f, 8, 18, false},
};

struct PatternDef {
  const Step* steps;
  uint8_t length;
};

constexpr PatternDef getPattern(SoundEffect sound) {
  switch (sound) {
    case SoundEffect::BOOT:         return {PATTERN_BOOT,         (uint8_t)(sizeof(PATTERN_BOOT) / sizeof(PATTERN_BOOT[0]))};
    case SoundEffect::HAPPY:        return {PATTERN_HAPPY,        (uint8_t)(sizeof(PATTERN_HAPPY) / sizeof(PATTERN_HAPPY[0]))};
    case SoundEffect::SURPRISED:    return {PATTERN_SURPRISED,    (uint8_t)(sizeof(PATTERN_SURPRISED) / sizeof(PATTERN_SURPRISED[0]))};
    case SoundEffect::LOVE_INTRO:   return {PATTERN_LOVE_INTRO,   (uint8_t)(sizeof(PATTERN_LOVE_INTRO) / sizeof(PATTERN_LOVE_INTRO[0]))};
    case SoundEffect::LOVE_SOFT:    return {PATTERN_LOVE_SOFT,    (uint8_t)(sizeof(PATTERN_LOVE_SOFT) / sizeof(PATTERN_LOVE_SOFT[0]))};
    case SoundEffect::ANGRY_1:      return {PATTERN_ANGRY_1,      (uint8_t)(sizeof(PATTERN_ANGRY_1) / sizeof(PATTERN_ANGRY_1[0]))};
    case SoundEffect::ANGRY_2:      return {PATTERN_ANGRY_2,      (uint8_t)(sizeof(PATTERN_ANGRY_2) / sizeof(PATTERN_ANGRY_2[0]))};
    case SoundEffect::ANGRY_3:      return {PATTERN_ANGRY_3,      (uint8_t)(sizeof(PATTERN_ANGRY_3) / sizeof(PATTERN_ANGRY_3[0]))};
    case SoundEffect::HYPER:        return {PATTERN_HYPER,        (uint8_t)(sizeof(PATTERN_HYPER) / sizeof(PATTERN_HYPER[0]))};
    case SoundEffect::SLEEPY:       return {PATTERN_SLEEPY,       (uint8_t)(sizeof(PATTERN_SLEEPY) / sizeof(PATTERN_SLEEPY[0]))};
    case SoundEffect::SLEEP_SOFT:   return {PATTERN_SLEEP_SOFT,   (uint8_t)(sizeof(PATTERN_SLEEP_SOFT) / sizeof(PATTERN_SLEEP_SOFT[0]))};
    case SoundEffect::WAKEUP:       return {PATTERN_WAKEUP,       (uint8_t)(sizeof(PATTERN_WAKEUP) / sizeof(PATTERN_WAKEUP[0]))};
    case SoundEffect::ALARM_SOFT:   return {PATTERN_ALARM_SOFT,   (uint8_t)(sizeof(PATTERN_ALARM_SOFT) / sizeof(PATTERN_ALARM_SOFT[0]))};
    case SoundEffect::ALARM_MEDIUM: return {PATTERN_ALARM_MEDIUM, (uint8_t)(sizeof(PATTERN_ALARM_MEDIUM) / sizeof(PATTERN_ALARM_MEDIUM[0]))};
    case SoundEffect::ALARM_STRONG: return {PATTERN_ALARM_STRONG, (uint8_t)(sizeof(PATTERN_ALARM_STRONG) / sizeof(PATTERN_ALARM_STRONG[0]))};
    case SoundEffect::LOW_BATTERY:  return {PATTERN_LOW_BATTERY,  (uint8_t)(sizeof(PATTERN_LOW_BATTERY) / sizeof(PATTERN_LOW_BATTERY[0]))};
    case SoundEffect::YAWN:         return {PATTERN_YAWN,         (uint8_t)(sizeof(PATTERN_YAWN) / sizeof(PATTERN_YAWN[0]))};
    case SoundEffect::OFFENDED:     return {PATTERN_OFFENDED,     (uint8_t)(sizeof(PATTERN_OFFENDED) / sizeof(PATTERN_OFFENDED[0]))};
    case SoundEffect::REBOOTING:    return {PATTERN_REBOOTING,    (uint8_t)(sizeof(PATTERN_REBOOTING) / sizeof(PATTERN_REBOOTING[0]))};
    default: return {nullptr, 0};
  }
}
} // namespace

const char* soundName(SoundEffect s) {
  switch (s) {
    case SoundEffect::BOOT: return "BOOT";
    case SoundEffect::HAPPY: return "HAPPY";
    case SoundEffect::SURPRISED: return "SURPRISED";
    case SoundEffect::LOVE_INTRO: return "LOVE_INTRO";
    case SoundEffect::LOVE_LOOP: return "LOVE_LOOP";
    case SoundEffect::LOVE_SOFT: return "LOVE_SOFT";
    case SoundEffect::ANGRY_1: return "ANGRY_1";
    case SoundEffect::ANGRY_2: return "ANGRY_2";
    case SoundEffect::ANGRY_3: return "ANGRY_3";
    case SoundEffect::HYPER: return "HYPER";
    case SoundEffect::SLEEPY: return "SLEEPY";
    case SoundEffect::SLEEP: return "SLEEP";
    case SoundEffect::SLEEP_SOFT: return "SLEEP_SOFT";
    case SoundEffect::WAKEUP: return "WAKEUP";
    case SoundEffect::ALARM_SOFT: return "ALARM_SOFT";
    case SoundEffect::ALARM_MEDIUM: return "ALARM_MEDIUM";
    case SoundEffect::ALARM_STRONG: return "ALARM_STRONG";
    case SoundEffect::LOW_BATTERY: return "LOW_BATTERY";
    case SoundEffect::YAWN: return "YAWN";
    case SoundEffect::OFFENDED: return "OFFENDED";
    case SoundEffect::REBOOTING: return "REBOOTING";
    default: return "NONE";
  }
}

void AudioManager::begin() {
  i2s_config_t config = {};
  config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
  config.sample_rate = SAMPLE_RATE;
  config.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
  config.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
  config.communication_format = I2S_COMM_FORMAT_STAND_I2S;
  config.intr_alloc_flags = 0;
  config.dma_buf_count = 8;
  config.dma_buf_len = 256;
  config.use_apll = false;
  config.tx_desc_auto_clear = true;
  config.fixed_mclk = 0;

  i2s_pin_config_t pins = {};
  pins.bck_io_num = Pins::I2S_BCLK;
  pins.ws_io_num = Pins::I2S_LRCK;
  pins.data_out_num = Pins::I2S_DOUT;
  pins.data_in_num = I2S_PIN_NO_CHANGE;

  i2s_driver_install(AUDIO_I2S_PORT, &config, 0, nullptr);
  i2s_set_pin(AUDIO_I2S_PORT, &pins);
  i2s_set_clk(AUDIO_I2S_PORT, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);

  initialized = true;
  muted = false;
  playing = false;
  looped = false;
  phase = 0.0f;
  activePattern = nullptr;
  activePatternLength = 0;
  stepRemainingMs = 0;
  stepElapsedMs = 0;
  masterVolume = 0.34f;

  Serial.println("[AUDIO] MAX98357A I2S init ok");
}

void AudioManager::setMasterVolume(float value) {
  if (value < 0.0f) value = 0.0f;
  if (value > 1.0f) value = 1.0f;
  masterVolume = value;
}

void AudioManager::setMuted(bool value) {
  muted = value;
  Serial.print("[AUDIO] muted = ");
  Serial.println(muted ? "true" : "false");

  if (muted) {
    stop();
  }
}

SoundEffect AudioManager::stateToSound(RobotState state) const {
  switch (state) {
    case RobotState::BOOT: return SoundEffect::BOOT;
    case RobotState::HAPPY: return SoundEffect::HAPPY;
    case RobotState::SURPRISED: return SoundEffect::SURPRISED;
    case RobotState::LOVE_TRANSITION: return SoundEffect::LOVE_INTRO;
    case RobotState::LOVE: return SoundEffect::NONE;
    case RobotState::ANGRY_1: return SoundEffect::ANGRY_1;
    case RobotState::ANGRY_2: return SoundEffect::ANGRY_2;
    case RobotState::ANGRY_3: return SoundEffect::ANGRY_3;
    case RobotState::HYPER: return SoundEffect::HYPER;
    case RobotState::SLEEPY: return SoundEffect::SLEEPY;
    case RobotState::SLEEP: return SoundEffect::NONE;
    case RobotState::LOW_BATTERY: return SoundEffect::LOW_BATTERY;
    case RobotState::YAWN: return SoundEffect::YAWN;
    case RobotState::OFFENDED: return SoundEffect::OFFENDED;
    case RobotState::REBOOTING: return SoundEffect::REBOOTING;
    default: return SoundEffect::NONE;
  }
}

void AudioManager::playForState(RobotState state) {
  play(stateToSound(state));
}

void AudioManager::playWakeup() {
  play(SoundEffect::WAKEUP);
}

void AudioManager::play(SoundEffect sound) {
  if (!initialized || muted || sound == SoundEffect::NONE) return;

  const uint32_t now = millis();
  if (sound == lastPlayed && (now - soundStartedAt) < 180) {
    return;
  }

  stop();

  lastPlayed = sound;
  currentSound = sound;
  playing = true;
  looped = false;
  soundStartedAt = now;
  patternStepAt = now;
  patternStep = -1;
  phase = 0.0f;

  Serial.print("[AUDIO] play -> ");
  Serial.println(soundName(sound));

  startPattern(sound);
}

void AudioManager::stop() {
  if (!initialized) return;

  const bool hadPlayback = playing || currentSound != SoundEffect::NONE;

  playing = false;
  looped = false;
  currentSound = SoundEffect::NONE;
  activePattern = nullptr;
  activePatternLength = 0;
  patternStep = 0;
  stepRemainingMs = 0;
  stepElapsedMs = 0;
  phase = 0.0f;
  writeSilenceChunk(4);

  if (hadPlayback) {
    Serial.println("[AUDIO] stop");
  }
}

void AudioManager::startPattern(SoundEffect sound) {
  PatternDef def = getPattern(sound);
  activePattern = def.steps;
  activePatternLength = def.length;

  if (!activePattern || activePatternLength == 0) {
    playing = false;
    currentSound = SoundEffect::NONE;
    return;
  }

  advanceToNextStep();
}

void AudioManager::advanceToNextStep() {
  if (!activePattern || activePatternLength == 0) {
    stop();
    return;
  }

  ++patternStep;
  patternStepAt = millis();

  if (patternStep >= activePatternLength) {
    playing = false;
    currentSound = SoundEffect::NONE;
    activePattern = nullptr;
    activePatternLength = 0;
    stepRemainingMs = 0;
    stepElapsedMs = 0;
    return;
  }

  stepRemainingMs = activePattern[patternStep].durationMs;
  stepElapsedMs = 0;
}

void AudioManager::writeToneChunk(const PatternStep& step, uint16_t durationMs) {
  if (!initialized || durationMs == 0) return;

  const int totalSamples = (SAMPLE_RATE * durationMs) / 1000;
  const int totalStepSamples = max(1, (int)((SAMPLE_RATE * step.durationMs) / 1000));
  const int stepOffsetSamples = (SAMPLE_RATE * stepElapsedMs) / 1000;
  const int attackSamples = max(1, (int)((SAMPLE_RATE * step.attackMs) / 1000));
  const int releaseSamples = max(1, (int)((SAMPLE_RATE * step.releaseMs) / 1000));

  static int16_t buffer[256];
  int writtenSamples = 0;

  while (writtenSamples < totalSamples) {
    int chunk = totalSamples - writtenSamples;
    if (chunk > 256) chunk = 256;

    for (int i = 0; i < chunk; i++) {
      const int sampleIndex = stepOffsetSamples + writtenSamples + i;
      float t = (float)sampleIndex / (float)totalStepSamples;
      if (t < 0.0f) t = 0.0f;
      if (t > 1.0f) t = 1.0f;

      const float freq = step.freqStartHz + (step.freqEndHz - step.freqStartHz) * t;

      float env = 1.0f;
      if (sampleIndex < attackSamples) {
        env = (float)sampleIndex / (float)attackSamples;
      } else if (sampleIndex > (totalStepSamples - releaseSamples)) {
        const int relPos = totalStepSamples - sampleIndex;
        env = (float)relPos / (float)releaseSamples;
      }

      if (env < 0.0f) env = 0.0f;
      if (env > 1.0f) env = 1.0f;

      const float amp = step.amplitude * masterVolume * env;
      const float sample = sinf(phase) * amp;
      buffer[i] = (int16_t)(sample * 32767.0f);

      phase += 2.0f * PI_F * freq / (float)SAMPLE_RATE;
      if (phase >= 2.0f * PI_F) {
        phase -= 2.0f * PI_F;
      }
    }

    size_t bytesWritten = 0;
    i2s_write(AUDIO_I2S_PORT, buffer, chunk * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
    writtenSamples += chunk;
  }
}

void AudioManager::writeSilenceChunk(uint16_t durationMs) {
  if (!initialized || durationMs == 0) return;

  const int totalSamples = (SAMPLE_RATE * durationMs) / 1000;
  static int16_t buffer[256] = {0};

  int writtenSamples = 0;
  while (writtenSamples < totalSamples) {
    int chunk = totalSamples - writtenSamples;
    if (chunk > 256) chunk = 256;

    size_t bytesWritten = 0;
    i2s_write(AUDIO_I2S_PORT, buffer, chunk * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
    writtenSamples += chunk;
  }
}

void AudioManager::update(uint32_t /*now*/) {
  if (!initialized || muted || !playing || !activePattern || activePatternLength == 0) {
    return;
  }

  if (patternStep < 0 || patternStep >= activePatternLength) {
    stop();
    return;
  }

  const PatternStep& step = activePattern[patternStep];
  if (stepRemainingMs == 0) {
    advanceToNextStep();
    return;
  }

  const uint16_t sliceMs = (stepRemainingMs > AUDIO_SLICE_MS) ? AUDIO_SLICE_MS : stepRemainingMs;

  if (step.silence) {
    writeSilenceChunk(sliceMs);
  } else {
    writeToneChunk(step, sliceMs);
  }

  stepRemainingMs -= sliceMs;
  stepElapsedMs += sliceMs;

  if (stepRemainingMs == 0) {
    advanceToNextStep();
  }
}