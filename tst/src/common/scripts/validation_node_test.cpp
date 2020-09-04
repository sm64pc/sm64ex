#include "pch.h"
#include "common/scripts/validation_node.cpp"

TEST(ValidationNodeTest, IsInvalidInitially) {
  const auto validation_node = std::make_shared<ValidationNode>();
  EXPECT_FALSE(validation_node->is_valid());
}

TEST(ValidationNodeTest, CanValidate) {
  auto validation_node = std::make_shared<ValidationNode>();
  validation_node->validate();
  EXPECT_TRUE(validation_node->is_valid());
}

TEST(ValidationNodeTest, CanInvalidate) {
  auto validation_node = std::make_shared<ValidationNode>();

  validation_node->validate();
  EXPECT_TRUE(validation_node->is_valid());

  validation_node->invalidate();
  EXPECT_FALSE(validation_node->is_valid());
}

TEST(ValidationNodeTest, DependingInvalidates) {
  auto parent_node = std::make_shared<ValidationNode>();
  auto child_node = std::make_shared<ValidationNode>();

  parent_node->validate();
  child_node->validate();
  EXPECT_TRUE(parent_node->is_valid());
  EXPECT_TRUE(child_node->is_valid());

  parent_node->depend_on(child_node);
  EXPECT_FALSE(parent_node->is_valid());
  EXPECT_TRUE(child_node->is_valid());
}
 
TEST(ValidationNodeTest, InvalidationAffectsParents) {
  auto parent_node = std::make_shared<ValidationNode>();
  auto child_node = std::make_shared<ValidationNode>();
  parent_node->depend_on(child_node);

  parent_node->validate();
  child_node->validate();
  EXPECT_TRUE(parent_node->is_valid());
  EXPECT_TRUE(child_node->is_valid());

  child_node->invalidate();
  EXPECT_FALSE(parent_node->is_valid());
  EXPECT_FALSE(child_node->is_valid());
}

TEST(ValidationNodeTest, InvalidationDoesntAffectParentsAfterClear) {
  auto parent_node = std::make_shared<ValidationNode>();
  auto child_node = std::make_shared<ValidationNode>();
  parent_node->depend_on(child_node);

  parent_node->validate();
  child_node->validate();
  EXPECT_TRUE(parent_node->is_valid());
  EXPECT_TRUE(child_node->is_valid());

  parent_node->clear();
  parent_node->validate();
  EXPECT_TRUE(parent_node->is_valid());
  EXPECT_TRUE(child_node->is_valid());

  child_node->invalidate();
  EXPECT_TRUE(parent_node->is_valid());
  EXPECT_FALSE(child_node->is_valid());
}
