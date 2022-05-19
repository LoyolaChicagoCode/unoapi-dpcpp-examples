add_executable(integration main.cpp)
target_link_libraries(integration fmt::fmt spdlog::spdlog CLI11::CLI11)
