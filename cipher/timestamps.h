#ifndef INTEGRATION_TIMESTAMPS_H
#define INTEGRATION_TIMESTAMPS_H

#include <vector>
#include <unordered_map>
#include <chrono>

// can use const pair with clang++ but not g++
typedef std::vector<std::pair<const std::string, const std::chrono::steady_clock::time_point> > ts_vector;

void mark_time(ts_vector& timestamps, std::string_view label);
void print_timestamps(const ts_vector & timestamps, std::string_view filename, std::string_view device_name);

#endif // INTEGRATION_TIMESTAMPS_H
