#include <gtest/gtest.h>

// Checks that GoogleTest is wired into the build and that ctest discovers it.
TEST(SmokeTest, FrameworkRuns) {
	EXPECT_TRUE(true);
}

// Checks that the SAMPLES_DIR compile definition reaches the tests.
TEST(SmokeTest, SamplesDirIsDefined) {
  EXPECT_STRNE(SAMPLES_DIR, "");
}
