#pragma once

struct InputEvent {

  static InputEvent* instance;

  float x;
  float y;

  int px;
  int py;

  bool pressed;

  InputEvent();

  void update();

};
