#pragma once

#include "robot_types.h"
#include "event_queue.h"
#include "audio_manager.h"
#include "display_manager.h"

struct RobotBrain {
  RobotState currentState = RobotState::BOOT;
  RobotState previousState = RobotState::BOOT;

  RobotMoodProfile moodProfile = RobotMoodProfile::CALM;

  uint32_t stateStartedAt = 0;
  uint32_t stateDuration = 0;
  bool stateSoundPlayed = false;

  uint32_t lastInteractionAt = 0;
  uint32_t lastIdleLookAt = 0;
  uint32_t lastAutoBlinkAt = 0;
  uint32_t lastIdleEventAt = 0;
  uint32_t lastLoveSoftAt = 0;
  uint32_t lastSleepSoftAt = 0;
  uint32_t lastStimAt = 0;

  uint8_t stimulationScore = 0;

  void begin(uint32_t now) {
    changeState(RobotState::BOOT, 1200, now);
    moodProfile = RobotMoodProfile::CALM;

    lastInteractionAt = now;
    lastIdleLookAt = now;
    lastAutoBlinkAt = now;
    lastIdleEventAt = now;
    lastLoveSoftAt = now;
    lastSleepSoftAt = now;
    lastStimAt = now;
    stimulationScore = 0;
  }

  void changeState(RobotState next, uint32_t durationMs, uint32_t now) {
    if (currentState == next && stateDuration == durationMs) {
      return;
    }

    previousState = currentState;
    currentState = next;
    stateStartedAt = now;
    stateDuration = durationMs;
    stateSoundPlayed = false;

    Serial.print("[STATE] ");
    Serial.print(stateName(previousState));
    Serial.print(" -> ");
    Serial.println(stateName(currentState));
  }

  void emit(EventQueue& queue, RobotEvent event) {
    if (event == RobotEvent::NONE) return;

    if (!queue.push(event)) {
      Serial.println("[BRAIN] event queue overflow");
      return;
    }

    Serial.print("[BRAIN EVENT] ");
    Serial.println(eventName(event));
  }

  uint32_t idleEventInterval() const {
    return (moodProfile == RobotMoodProfile::CALM) ? 8000 : 5000;
  }

  void registerStimulus(EventQueue& queue, RobotEvent event, uint32_t now) {
    if (event != RobotEvent::TAP_SINGLE &&
        event != RobotEvent::TAP_DOUBLE &&
        event != RobotEvent::TAP_TRIPLE &&
        event != RobotEvent::TAP_RAPID) {
      return;
    }

    if (now - lastStimAt > 4000) {
      stimulationScore = 0;
    }

    lastStimAt = now;

    if (event == RobotEvent::TAP_RAPID) stimulationScore += 3;
    else stimulationScore += 1;

    if (stimulationScore >= 7) {
      emit(queue, RobotEvent::OVERSTIMULATED);
      stimulationScore = 0;
    }
  }

  void processEvent(
    RobotEvent event,
    EventQueue& queue,
    DisplayManager& display,
    AudioManager& audio,
    uint32_t now
  ) {
    if (event == RobotEvent::NONE) return;

    registerStimulus(queue, event, now);

    switch (event) {
      case RobotEvent::BOOT_DONE:
        changeState(RobotState::IDLE, 0, now);
        display.chooseLook();
        break;

      case RobotEvent::TAP_SINGLE:
        lastInteractionAt = now;
        if (currentState == RobotState::TIRED || currentState == RobotState::SLEEPY || currentState == RobotState::SLEEP || currentState == RobotState::CLOCK) {
          emit(queue, RobotEvent::WAKEUP);
        } else {
          display.setLookTarget(-2, 0);
          changeState(RobotState::HAPPY, 1200, now);
          display.requestBlink(now);
        }
        break;

      case RobotEvent::TAP_DOUBLE:
        lastInteractionAt = now;
        if (currentState == RobotState::TIRED || currentState == RobotState::SLEEPY || currentState == RobotState::SLEEP || currentState == RobotState::CLOCK) {
          emit(queue, RobotEvent::WAKEUP);
        } else {
          display.setLookTarget(2, -2);
          changeState(RobotState::SURPRISED, 1400, now);
          display.requestBlink(now);
        }
        break;

      case RobotEvent::TAP_TRIPLE:
        lastInteractionAt = now;
        if (currentState == RobotState::TIRED || currentState == RobotState::SLEEPY || currentState == RobotState::SLEEP || currentState == RobotState::CLOCK) {
          emit(queue, RobotEvent::WAKEUP);
        } else {
          display.setLookTarget(-1, 4);
          changeState(RobotState::LOVE_TRANSITION, 700, now);
        }
        break;

      case RobotEvent::TAP_RAPID:
        lastInteractionAt = now;
        moodProfile = RobotMoodProfile::ACTIVE;
        if (currentState == RobotState::TIRED || currentState == RobotState::SLEEPY || currentState == RobotState::SLEEP || currentState == RobotState::CLOCK) {
          emit(queue, RobotEvent::WAKEUP);
        } else {
          display.setLookTarget(0, 0);
          changeState(RobotState::HYPER, 1800, now);
        }
        break;

      case RobotEvent::HOLD_START:
        lastInteractionAt = now;
        if (currentState == RobotState::TIRED || currentState == RobotState::SLEEPY || currentState == RobotState::SLEEP || currentState == RobotState::CLOCK) {
          emit(queue, RobotEvent::WAKEUP);
        }
        break;

      case RobotEvent::HOLD_LEVEL_1:
        lastInteractionAt = now;
        display.setLookTarget(0, 2);
        changeState(RobotState::ANGRY_1, 0, now);
        display.angryLevel = 1;
        break;

      case RobotEvent::HOLD_LEVEL_2:
        lastInteractionAt = now;
        display.setLookTarget(0, 2);
        changeState(RobotState::ANGRY_2, 0, now);
        display.angryLevel = 2;
        break;

      case RobotEvent::HOLD_LEVEL_3:
        lastInteractionAt = now;
        display.setLookTarget(0, 2);
        changeState(RobotState::ANGRY_3, 0, now);
        display.angryLevel = 3;
        break;

      case RobotEvent::HOLD_VERY_LONG:
        lastInteractionAt = now;
        display.setLookTarget(-2, 3);
        changeState(RobotState::OFFENDED, 2600, now);
        display.angryLevel = 0;
        break;

      case RobotEvent::HOLD_RELEASE:
        lastInteractionAt = now;
        if (currentState == RobotState::ANGRY_1 ||
            currentState == RobotState::ANGRY_2 ||
            currentState == RobotState::ANGRY_3) {
          changeState(RobotState::IDLE, 0, now);
          display.angryLevel = 0;
          display.chooseLook();
        }
        break;

      case RobotEvent::IDLE_TIMEOUT:
        if (currentState == RobotState::IDLE) {
          display.setLookTarget(0, 6);
          changeState(RobotState::TIRED, Config::TIRED_TO_SLEEPY_MS, now);
        }
        break;

      case RobotEvent::SLEEP_TIMEOUT:
        if (currentState == RobotState::TIRED) {
          display.setLookTarget(0, 6);
          changeState(RobotState::SLEEPY, Config::SLEEPY_TO_SLEEP_MS, now);
        } else if (currentState == RobotState::SLEEPY) {
          display.setLookTarget(0, 0);
          changeState(RobotState::SLEEP, 0, now);
        }
        break;

      case RobotEvent::CLOCK_TIMEOUT:
        if (currentState == RobotState::SLEEP) {
          display.setLookTarget(0, 0);
          changeState(RobotState::CLOCK, 0, now);
        }
        break;

      case RobotEvent::WAKEUP:
        lastInteractionAt = now;
        audio.playWakeup();
        display.requestBlink(now);
        if (currentState == RobotState::TIRED || currentState == RobotState::SLEEPY || currentState == RobotState::SLEEP || currentState == RobotState::CLOCK) {
          display.setLookTarget(0, 0);
          changeState(RobotState::WAKE_SCAN, Config::WAKE_SCAN_MS, now);
        } else {
          changeState(RobotState::HAPPY, 1000, now);
        }
        stateSoundPlayed = true;
        break;

      case RobotEvent::LOW_BATTERY:
        display.setLookTarget(0, 5);
        changeState(RobotState::LOW_BATTERY, 0, now);
        break;

      case RobotEvent::BATTERY_OK:
        if (currentState == RobotState::LOW_BATTERY) {
          changeState(RobotState::IDLE, 0, now);
          display.chooseLook();
        }
        break;

      case RobotEvent::IDLE_LOOK_UP:
        if (currentState == RobotState::IDLE) {
          display.setLookTarget(0, -6);
        }
        break;

      case RobotEvent::IDLE_DOUBLE_BLINK:
        if (currentState == RobotState::IDLE) {
          display.requestBlink(now);
        }
        break;

      case RobotEvent::IDLE_YAWN:
        if (currentState == RobotState::IDLE) {
          display.setLookTarget(0, 5);
          changeState(RobotState::YAWN, 1800, now);
        }
        break;

      case RobotEvent::OVERSTIMULATED:
        if (currentState == RobotState::IDLE ||
            currentState == RobotState::HAPPY ||
            currentState == RobotState::HYPER ||
            currentState == RobotState::SURPRISED) {
          display.setLookTarget(0, 6);
          changeState(RobotState::TIRED, 1400, now);
        }
        break;

      case RobotEvent::REBOOT_REQUEST:
        changeState(RobotState::REBOOTING, 2200, now);
        break;

      default:
        break;
    }
  }

  void update(
    EventQueue& queue,
    DisplayManager& display,
    AudioManager& audio,
    uint32_t now
  ) {
    if (!stateSoundPlayed) {
      switch (currentState) {
        case RobotState::BOOT:
        case RobotState::HAPPY:
        case RobotState::SURPRISED:
        case RobotState::LOVE_TRANSITION:
        case RobotState::ANGRY_1:
        case RobotState::ANGRY_2:
        case RobotState::ANGRY_3:
        case RobotState::HYPER:
        case RobotState::TIRED:
        case RobotState::SLEEPY:
        case RobotState::LOW_BATTERY:
        case RobotState::YAWN:
        case RobotState::OFFENDED:
        case RobotState::REBOOTING:
          audio.playForState(currentState);
          break;

        default:
          break;
      }

      stateSoundPlayed = true;
    }

    if (stateDuration > 0 && (now - stateStartedAt) >= stateDuration) {
      switch (currentState) {
        case RobotState::BOOT:
          emit(queue, RobotEvent::BOOT_DONE);
          break;

        case RobotState::HAPPY:
        case RobotState::SURPRISED:
        case RobotState::HYPER:
          changeState(RobotState::IDLE, 0, now);
          display.chooseLook();
          break;

        case RobotState::LOVE_TRANSITION:
          display.setLookTarget(0, 4);
          changeState(RobotState::LOVE, 3500, now);
          break;

        case RobotState::LOVE:
          changeState(RobotState::IDLE, 0, now);
          display.chooseLook();
          break;

        case RobotState::TIRED:
        case RobotState::SLEEPY:
          emit(queue, RobotEvent::SLEEP_TIMEOUT);
          break;

        case RobotState::WAKE_SCAN:
          changeState(RobotState::IDLE, 0, now);
          display.chooseLook();
          break;

        case RobotState::YAWN:
          changeState(RobotState::IDLE, 0, now);
          display.chooseLook();
          break;

        case RobotState::OFFENDED:
          changeState(RobotState::IDLE, 0, now);
          display.chooseLook();
          break;

        case RobotState::REBOOTING:
          changeState(RobotState::BOOT, 1200, now);
          break;

        default:
          break;
      }
    }

    if (currentState == RobotState::IDLE && (now - lastIdleLookAt) > 2000) {
      lastIdleLookAt = now;
      display.chooseLook();
    }

    if ((currentState == RobotState::IDLE || currentState == RobotState::LOVE) &&
        (now - lastAutoBlinkAt) > 3200) {
      lastAutoBlinkAt = now;
      display.requestBlink(now);
    }

    if (currentState == RobotState::IDLE &&
        (now - lastInteractionAt) >= Config::IDLE_TO_SLEEPY_MS) {
      emit(queue, RobotEvent::IDLE_TIMEOUT);
      lastInteractionAt = now;
    }

    if (currentState == RobotState::IDLE &&
        (now - lastIdleEventAt) > idleEventInterval()) {
      lastIdleEventAt = now;

      int roll = random(0, 100);

      if (moodProfile == RobotMoodProfile::CALM) {
        if (roll < 45) emit(queue, RobotEvent::IDLE_LOOK_UP);
        else if (roll < 75) emit(queue, RobotEvent::IDLE_DOUBLE_BLINK);
        else emit(queue, RobotEvent::IDLE_YAWN);
      } else {
        if (roll < 30) emit(queue, RobotEvent::IDLE_LOOK_UP);
        else if (roll < 60) emit(queue, RobotEvent::IDLE_DOUBLE_BLINK);
        else emit(queue, RobotEvent::IDLE_YAWN);
      }
    }

    if ((now - lastInteractionAt) > 30000) {
      moodProfile = RobotMoodProfile::CALM;
    }

    if (currentState == RobotState::LOVE && (now - lastLoveSoftAt) > 1400) {
      lastLoveSoftAt = now;
      audio.play(SoundEffect::LOVE_SOFT);
    }

    if (currentState == RobotState::SLEEP && (now - lastSleepSoftAt) > 2200) {
      lastSleepSoftAt = now;
      audio.play(SoundEffect::SLEEP_SOFT);
    }

    if (currentState == RobotState::SLEEP &&
        (now - stateStartedAt) >= Config::SLEEP_TO_CLOCK_MS) {
      emit(queue, RobotEvent::CLOCK_TIMEOUT);
    }

    if (currentState == RobotState::LOW_BATTERY) {
      display.setLookTarget(0, 5);
    }
  }
};