add_executable(integration main.cpp f.cpp trapezoid.cpp timestamps.cpp)
target_link_libraries(integration fmt::fmt spdlog::spdlog CLI11::CLI11)

enable_testing()
add_executable(integration_tests test.cpp f.cpp trapezoid.cpp)
target_link_libraries(integration_tests gtest_main fmt::fmt spdlog::spdlog CLI11::CLI11)
include(GoogleTest)
gtest_discover_tests(integration_tests)
