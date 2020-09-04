#include "pch.h"

TEST(BuildTest, BuildsSuccessfully) {
  // TODO: Not portable via GitHub, find an approach that isn't hard-coded.
  const auto exit_code = system(
      "R:/msys64/msys2_shell.cmd -mingw64 -c \"cd "
      "R:/Documents/CppWorkspace/sm64-port; make VERSION=us -j4\"");

  EXPECT_EQ(exit_code, EXIT_SUCCESS);
}
