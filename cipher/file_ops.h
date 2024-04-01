#ifndef FILEOPS_H
#define FILEOPS_H

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <fstream>
#include <filesystem>

int read_from_binary(
    std::vector<std::byte> & buffer,
    std::string path_to_file
);

// If the file cannot be opened, then the function will notify the caller
// by returning 1.
template <class Writable>
int write_to_binary(Writable & buffer, std::string path_to_file)
{
    std::ofstream output(path_to_file, std::ios::binary);
    if (!output.is_open()) {
        spdlog::error("Error opening file to read from: {}", path_to_file);
        return 1;
    }
    // TODO: See if it is possible to write binary data to file without casting.
    output.write(reinterpret_cast<char*>(&buffer[0]), buffer.size());
    output.close();
    return 0;
}

#endif