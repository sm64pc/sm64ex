#pragma once

#include "include/types.h"

class ObjectBuilderParams {
public:
  ObjectBuilderParams& set_pos(s16 x, s16 y, s16 z) {
    this->x = x;
    this->y = y;
    this->z = z;

    return *this;
  }

  ObjectBuilderParams& set_angle(s16 x_angle, s16 y_angle, s16 z_angle) {
    this->x_angle = x_angle;
    this->y_angle = y_angle;
    this->z_angle = z_angle;

    return *this;
  }

  ObjectBuilderParams& set_beh_param(u16 beh_param) {
    this->beh_param = beh_param;

    return *this;
  }

  s16 x;
  s16 y;
  s16 z;

  s16 x_angle = 0;
  s16 y_angle = 0;
  s16 z_angle = 0;

  u16 beh_param = 0x00000000;
};