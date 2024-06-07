#ifndef CIPHER_TIMESTAMPS_H
#define CIPHER_TIMESTAMPS_H

#include <vector>
#include <unordered_map>
#include <chrono>

// mark_time using nanotimer
void mark_time(std::vector<std::tuple<std::string, double>> & timestamps, double & timestamp, std::string_view label);

// print_timestamps using nanotimer
void print_timestamps(std::vector<std::tuple<std::string, double>> & timestamps);

#endif