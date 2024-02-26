#include <sycl/sycl.hpp>
#include <dpc_common.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <fstream>
#include <iterator>
#include <algorithm>
#include <iostream>

#include "file_ops.h"

void read_from_binary(std::string & buffer, std::string path_to_file)
{
    std::ifstream input(path_to_file, std::ios::binary);
    if (!input.is_open()) {
        spdlog::info("Error opening file to read from: {}", path_to_file);
        exit(1);
    }
    buffer.assign(std::istreambuf_iterator<char>(input), {});
    input.close();
}

void write_to_binary(const sycl::host_accessor<char, 1> & buffer, std::string path_to_file)
{
    std::ofstream output(path_to_file, std::ios::binary);
    if (!output.is_open()) {
        spdlog::info("Error opening file to read from: {}", path_to_file);
        exit(1);
    }
    std::copy(
        buffer.begin(),
        buffer.end(),
        std::ostreambuf_iterator<char>(output)
    );
    output.close();
}

void write_to_binary(std::vector<char> & buffer, std::string path_to_file)
{
    std::ofstream output(path_to_file, std::ios::binary);
    if (!output.is_open()) {
        spdlog::info("Error opening file to read from: {}", path_to_file);
        exit(1);
    }
    std::copy(
        buffer.begin(),
        buffer.end(),
        std::ostreambuf_iterator<char>(output)
    );
    output.close();
}