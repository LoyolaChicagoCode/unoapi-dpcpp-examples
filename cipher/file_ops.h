#ifndef FILEOPS_H
#define FILEOPS_H

void read_from_binary(
    std::string & buffer,
    std::string path_to_file
);

void write_to_binary(
    const sycl::host_accessor<char, 1> & buffer,
    std::string path_to_file
);

void write_to_binary(
    std::vector<char> & buffer,
    std::string path_to_file
);

#endif