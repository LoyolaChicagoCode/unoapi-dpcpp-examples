#include <cstdio>
#include <fmt/format.h>
#include <sycl/sycl.hpp>
#include <dpc_common.hpp>

#include "timestamps.h"

// new mark_time -> using plf_nanotimer
void mark_time(std::vector<std::tuple<std::string, double>> & timestamps, double & timestamp, const std::string_view label) {
    timestamps.push_back(std::pair(label.data(), timestamp));
}

// new print -> using plf_nanotimer
void print_timestamps(std::vector<std::tuple<std::string, double>> & timestamps) {
    std::for_each(timestamps.begin(), timestamps.end(), [](const auto & tuple) {
        fmt::print("{},{}", std::get<0>(tuple), std::get<1>(tuple));
        fmt::print("\n");
    });
}