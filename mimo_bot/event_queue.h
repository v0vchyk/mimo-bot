// ============================================================
// FILE: event_queue.h
// ============================================================
#pragma once

#include "robot_types.h"

struct EventQueue {
  static constexpr uint8_t CAPACITY = 16;

  RobotEvent items[CAPACITY]{};
  uint8_t head = 0;
  uint8_t tail = 0;
  uint8_t count = 0;

  bool push(RobotEvent event) {
    if (count >= CAPACITY) {
      return false;
    }

    items[tail] = event;
    tail = (tail + 1) % CAPACITY;
    count++;
    return true;
  }

  RobotEvent pop() {
    if (count == 0) {
      return RobotEvent::NONE;
    }

    RobotEvent event = items[head];
    head = (head + 1) % CAPACITY;
    count--;
    return event;
  }

  bool isEmpty() const {
    return count == 0;
  }

  void clear() {
    head = 0;
    tail = 0;
    count = 0;
  }
};