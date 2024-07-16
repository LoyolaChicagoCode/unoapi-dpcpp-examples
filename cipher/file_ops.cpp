#include "file_ops.h"

// If the file cannot be opened, then the function will notify the caller
// by returning 1.
int read_from_binary(std::vector<std::byte> & buffer, std::string path_to_file)
{
    std::ifstream input(path_to_file, std::ios::binary);
    if (!input.is_open()) {
        spdlog::error("Error opening file to read from: {}", path_to_file);
        return 1;
    }
    std::uintmax_t size = std::filesystem::file_size(path_to_file);
    buffer.resize(size);
    // TODO: See if it is possible to read binary data from file without casting.
    input.read(reinterpret_cast<char*>(buffer.data()), size);
    input.close();
    return 0;
}