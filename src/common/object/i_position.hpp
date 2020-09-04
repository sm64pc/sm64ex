#pragma once

#include "engine/math_util.h"

class IPosition {
 public:
  virtual void set_x(const f32 x) = 0;
  virtual f32 get_x() const = 0;

  virtual void set_y(const f32 y) = 0;
  virtual f32 get_y() const = 0;

  virtual void set_z(const f32 z) = 0;
  virtual f32 get_z() const = 0;

  void set(const IPosition& other) {
    set_x(other.get_x());
    set_y(other.get_y());
    set_z(other.get_z());
  }

  /**
   * Calculates (Euclidean distance)^2 from a given position to another. This
   * can be preferable over the exact distance, since square root operations
   * can be expensive.
   */
  f32 calc_sqr_distance_to(const IPosition& other) const {
    const auto dist_x = other.get_x() - get_x();
    const auto dist_y = other.get_y() - get_y();
    const auto dist_z = other.get_z() - get_z();

    return dist_x * dist_x + dist_y * dist_y + dist_z * dist_z;
  }

  void calc_angle_to(const IPosition& other, s16& h_angle, s16& v_angle) const {
    const auto dist_x = other.get_x() - get_x();
    const auto dist_y = other.get_y() - get_y();
    const auto dist_z = other.get_z() - get_z();

    h_angle = atan2s(dist_z, dist_x);
    v_angle = atan2s(sqrtf(dist_x * dist_x + dist_z * dist_z), -dist_y);
  }
};