
# UnoAPI:CMakeLists-targetlibraries:begin

add_executable(cipher main.cpp substitute.h scramble.cpp file_ops.cpp timestamps.cpp)
target_link_libraries(cipher spdlog::spdlog CLI11::CLI11)

enable_testing()
add_executable(cipher_tests test.cpp scramble.cpp file_ops.cpp)
target_link_libraries(cipher_tests gtest_main fmt::fmt spdlog::spdlog CLI11::CLI11)
include(GoogleTest)
gtest_discover_tests(cipher_tests)

# UnoAPI: CMakeLists-targetlibraries:end