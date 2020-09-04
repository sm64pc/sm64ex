#pragma once

#include "i_position.hpp"

class PositionWrapper : public IPosition {
  public:
    explicit PositionWrapper(f32* x_ptr, f32* y_ptr, f32* z_ptr)
      : x_ptr_(x_ptr),
        y_ptr_(y_ptr),
        z_ptr_(z_ptr) {}
    explicit PositionWrapper(const PositionWrapper& other) = delete;

    void set_x(const f32 x) override { *x_ptr_ = x; }
    f32 get_x() const override { return *x_ptr_; }

    void set_y(const f32 y) override { *y_ptr_ = y; }
    f32 get_y() const override { return *y_ptr_; }

    void set_z(const f32 z) override { *z_ptr_ = z; }
    f32 get_z() const override { return *z_ptr_; }

  private:
    f32* x_ptr_;
    f32* y_ptr_;
    f32* z_ptr_;
};
