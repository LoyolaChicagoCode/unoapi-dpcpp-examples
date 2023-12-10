#include <spdlog/spdlog.h>
#include <gtest/gtest.h>

#include "f.h"
#include "trapezoid.h"

// {{UnoAPI:integration-test-scaffolding:begin}}
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
// {{UnoAPI:integration-test-scaffolding:end}}

// {{UnoAPI:integration-test-simple1:begin}}
TEST_F(IntegrationTest, Simple1) {
    EXPECT_NEAR(single_trapezoid(1, 1, 0.5), 1, EPS);
}
// {{UnoAPI:integration-test-simple1:end}}

// {{UnoAPI:integration-test-simple2:begin}}
TEST_F(IntegrationTest, Simple2) {
    EXPECT_NEAR(single_trapezoid(0, 1, 0.5), 0.5, EPS);
}
// {{UnoAPI:integration-test-simple2:end}}

// {{UnoAPI:integration-test-simple3:begin}}
TEST_F(IntegrationTest, Simple3) {
    EXPECT_NEAR(single_trapezoid(-1, 1, 0.5), 0, EPS);
}
// {{UnoAPI:integration-test-simple3:end}}

// {{UnoAPI:integration-test-f1:begin}}
TEST_F(IntegrationTest, F1) {
    EXPECT_NEAR(f(0.5), 0.75, EPS);
}
// {{UnoAPI:integration-test-f1:end}}

// double outer_trapezoid(
//     const int grain_size,
//     const double x_pos,
//     const double dx_inner,
//     const double half_dx_inner
// )

// {{UnoAPI:integration-test-outer1:begin}}
TEST_F(IntegrationTest, Outer1) {
    EXPECT_NEAR(outer_trapezoid(1000, 0.0, 0.001, 0.0005), 1, EPS);
}
// {{UnoAPI:integration-test-outer1:end}}
