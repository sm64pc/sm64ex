#include "pch.h"
#include "common/scripts/dynamic_level_script_builder.cpp"

#include <initializer_list>

#include "common/scripts/i_level_script_builder.cpp"
#include "common/scripts/level_script_part.cpp"
#include "common/scripts/util.cpp"

// TODO: Move this to a common file.
typedef uintptr_t LevelScript;

void expect_scripts(const DynamicLevelScriptBuilder& builder,
                    const std::initializer_list<LevelScript> expected_scripts) {
  const auto* const entry_pointer = builder.get_entry_pointer();
  const auto size = builder.size();

  const std::vector<LevelScript> actual_scripts(
      entry_pointer,
      entry_pointer + size);

  EXPECT_THAT(actual_scripts, testing::ElementsAreArray(expected_scripts));
}

TEST(DynamicLevelScriptBuilderTest, ReturnsEmptyByDefault) {
  const DynamicLevelScriptBuilder builder;
  expect_scripts(builder, {});
}

TEST(DynamicLevelScriptBuilderTest, CanAddScript) {
  DynamicLevelScriptBuilder builder;

  builder.add_script(1)
         .add_script(2)
         .add_script(3)
         .add_script(4);

  expect_scripts(builder, {1, 2, 3, 4});
}

TEST(DynamicLevelScriptBuilderTest, CanAddScriptsWithInitializerList) {
  DynamicLevelScriptBuilder builder;

  builder.add_scripts({1, 2})
         .add_scripts({3, 4});

  expect_scripts(builder, {1, 2, 3, 4});
}

TEST(DynamicLevelScriptBuilderTest, CanAddScriptsWithPointer) {
  DynamicLevelScriptBuilder builder;

  LevelScript part1[] = {1, 2};
  LevelScript part2[] = {3, 4};
  builder.add_scripts(part1, 2)
         .add_scripts(part2, 2);

  expect_scripts(builder, {1, 2, 3, 4});
}

TEST(DynamicLevelScriptBuilderTest, CanAddBuilder) {
  DynamicLevelScriptBuilder builder;

  const auto child1 = std::make_shared<DynamicLevelScriptBuilder>();
  const auto child2 = std::make_shared<DynamicLevelScriptBuilder>();

  builder.add_builder(child1);
  (*child1).add_scripts({1, 2})
           .add_builder(child2);
  child2->add_scripts({3, 4});

  expect_scripts(builder, {1, 2, 3, 4});
}
