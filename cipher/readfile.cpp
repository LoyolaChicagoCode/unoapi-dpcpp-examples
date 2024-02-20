#include <iostream>
#include <fstream>
#include "readfile.h"

// remove white spaces
std::string file_to_string(std::string text_file)
{
    // Create and open a text file
    std::ifstream file(text_file);
    std::string buffer(std::istreambuf_iterator<char>{file}, {});

    while (!buffer.empty() && std::isspace(buffer.back())) {
        buffer.pop_back();
    }
    return buffer;
}
