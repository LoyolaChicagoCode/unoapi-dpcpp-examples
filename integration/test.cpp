#include <spdlog/spdlog.h>
#include <gtest/gtest.h>

#include "trapezoid.h"

class IntegrationTest : public testing::Test {
protected:
    static void SetUpTestSuite() {
        spdlog::set_level(spdlog::level::off);
        spdlog::info("spdlog level set to {}", spdlog::get_level()); // should be invisible!
    }

    static void TearDownTestSuite() {
    }
};

TEST_F(IntegrationTest, Simple1) {
    EXPECT_EQ(trapezoid(1, 1, 0.5), 1);
}

TEST_F(IntegrationTest, Simple2) {
    EXPECT_EQ(trapezoid(0, 1, 0.5), 0.5);
}

TEST_F(IntegrationTest, Simple3) {
    EXPECT_EQ(trapezoid(-1, 1, 0.5), 0);
}
