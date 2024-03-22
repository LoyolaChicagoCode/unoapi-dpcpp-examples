#ifndef FILEOPS_H
#define FILEOPS_H

void read_from_binary(
    std::vector<std::byte> & buffer,
    std::string path_to_file
);

void write_to_binary(
    const sycl::host_accessor<std::byte, 1> & buffer,
    std::string path_to_file
);

void write_to_binary(
    std::vector<std::byte> & buffer,
    std::string path_to_file
);

#endif