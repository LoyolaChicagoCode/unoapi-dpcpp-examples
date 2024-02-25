#include <fstream>
#include <iterator>
#include <algorithm>
#include <iostream>

#include <sycl/sycl.hpp>
#include <dpc_common.hpp>

#include "file_ops.h"

std::string read_from_binary(std::string path_to_file)
{
    std::ifstream input(path_to_file, std::ios::binary);
    if (!input.is_open()) {
        std::cerr << "Error opening file to read from: " << path_to_file << std::endl;
        exit(1);
    }
    std::string buffer(std::istreambuf_iterator<char>(input), {});
    input.close();
    return buffer;
}

void write_to_binary(const sycl::host_accessor<char, 1> & ciphertext, std::string path_to_file)
{
    std::ofstream output(path_to_file, std::ios::binary);
    if (!output.is_open()) {
        std::cerr << "Error opening file to write to: " << path_to_file << std::endl;
        exit(1);
    }
    std::copy(
        ciphertext.begin(),
        ciphertext.end(),
        std::ostreambuf_iterator<char>(output)
    );
    output.close();
}

void write_to_binary(std::vector<char> & ciphertext, std::string path_to_file)
{
    std::ofstream output(path_to_file, std::ios::binary);
    if (!output.is_open()) {
        std::cerr << "Error opening file to write to: " << path_to_file << std::endl;
        exit(1);
    }
    std::copy(
        ciphertext.begin(),
        ciphertext.end(),
        std::ostreambuf_iterator<char>(output)
    );
    output.close();
}