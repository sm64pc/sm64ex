#include "pch.h"

#include "common/object/i_object_wrapper.cpp"
#include "common/object/i_physical_object_wrapper.cpp"

class PhysicalObjectWrapper : public IPhysicalObjectWrapper {
  public:
    explicit PhysicalObjectWrapper(struct Object* wrapped_object)
      : IPhysicalObjectWrapper(wrapped_object) {}

    PositionWrapper& get_position() { return position_; }
    PositionWrapper& get_home() { return home_; }
};

TEST(IPhysicalObjectWrapperTest, CanSetPosition) {
  struct Object wrapped_object;
  PhysicalObjectWrapper wrapper(&wrapped_object);

  auto &position = wrapper.get_position();
  position.set_x(1);
  position.set_y(2);
  position.set_z(3);

  EXPECT_EQ(wrapped_object.oPosX, 1);
  EXPECT_EQ(wrapped_object.oPosY, 2);
  EXPECT_EQ(wrapped_object.oPosZ, 3);
}

TEST(IPhysicalObjectWrapperTest, CanGetPosition) {
  struct Object wrapped_object;
  PhysicalObjectWrapper wrapper(&wrapped_object);

  wrapped_object.oPosX = 1;
  wrapped_object.oPosY = 2;
  wrapped_object.oPosZ = 3;

  auto& position = wrapper.get_position();
  EXPECT_EQ(position.get_x(), 1);
  EXPECT_EQ(position.get_y(), 2);
  EXPECT_EQ(position.get_z(), 3);
}

TEST(IPhysicalObjectWrapperTest, CanSetHome) {
  struct Object wrapped_object;
  PhysicalObjectWrapper wrapper(&wrapped_object);

  auto& home = wrapper.get_home();
  home.set_x(1);
  home.set_y(2);
  home.set_z(3);

  EXPECT_EQ(wrapped_object.oHomeX, 1);
  EXPECT_EQ(wrapped_object.oHomeY, 2);
  EXPECT_EQ(wrapped_object.oHomeZ, 3);
}

TEST(IPhysicalObjectWrapperTest, CanGetHome) {
  struct Object wrapped_object;
  PhysicalObjectWrapper wrapper(&wrapped_object);

  wrapped_object.oHomeX = 1;
  wrapped_object.oHomeY = 2;
  wrapped_object.oHomeZ = 3;

  auto& home = wrapper.get_home();
  EXPECT_EQ(home.get_x(), 1);
  EXPECT_EQ(home.get_y(), 2);
  EXPECT_EQ(home.get_z(), 3);
}
