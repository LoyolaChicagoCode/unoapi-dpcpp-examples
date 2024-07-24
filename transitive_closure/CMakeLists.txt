add_executable(transitive_closure transitive_closure.cpp timestamps.cpp graph.cpp)
target_link_libraries(transitive_closure fmt::fmt spdlog::spdlog CLI11::CLI11)