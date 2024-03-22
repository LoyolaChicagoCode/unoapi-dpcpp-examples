#include <sycl/sycl.hpp>
#include <dpc_common.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <fstream>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <filesystem>
#include <cstdint>

#include "file_ops.h"

void read_from_binary(std::vector<std::byte> & buffer, std::string path_to_file)
{
    std::ifstream input(path_to_file, std::ios::binary);
    if (!input.is_open()) {
        spdlog::info("Error opening file to read from: {}", path_to_file);
        exit(1);
    }
    std::uintmax_t size = std::filesystem::file_size(path_to_file);
    buffer.resize(size);
    input.read(reinterpret_cast<char*>(buffer.data()), size);
    input.close();
}

void write_to_binary(const sycl::host_accessor<std::byte, 1> & buffer, std::string path_to_file)
{
    std::ofstream output(path_to_file, std::ios::binary);
    if (!output.is_open()) {
        spdlog::info("Error opening file to read from: {}", path_to_file);
        exit(1);
    }
    output.write(reinterpret_cast<char*>(&buffer[0]), buffer.size());
    output.close();
}

void write_to_binary(std::vector<std::byte> & buffer, std::string path_to_file)
{
    std::ofstream output(path_to_file, std::ios::binary);
    if (!output.is_open()) {
        spdlog::info("Error opening file to read from: {}", path_to_file);
        exit(1);
    }
    output.write(reinterpret_cast<char*>(&buffer[0]), buffer.size());
    output.close();
}