#include <spdlog/spdlog.h>
#include <gtest/gtest.h>

#include "f.h"
#include "trapezoid.h"

class IntegrationTest : public testing::Test {
public:
    static constexpr double EPS{1e-5};
protected:
    static void SetUpTestSuite() {
        spdlog::set_level(spdlog::level::off);
        spdlog::info("spdlog level set to {}", spdlog::get_level()); // should be invisible!
    }

    static void TearDownTestSuite() {
    }
};

TEST_F(IntegrationTest, Simple1) {
    EXPECT_NEAR(trapezoid(1, 1, 0.5), 1, EPS);
}

TEST_F(IntegrationTest, Simple2) {
    EXPECT_NEAR(trapezoid(0, 1, 0.5), 0.5, EPS);
}

TEST_F(IntegrationTest, Simple3) {
    EXPECT_NEAR(trapezoid(-1, 1, 0.5), 0, EPS);
}

TEST_F(IntegrationTest, F1) {
    EXPECT_NEAR(f(0.5), 1, EPS);
}
