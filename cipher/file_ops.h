#ifndef FILEOPS_H
#define FILEOPS_H


std::string read_from_binary
(
    std::string path_to_file
);

void write_to_binary
(
    const sycl::host_accessor<char, 1> & result,
    std::string path_to_file
);

void write_to_binary
(
    std::vector<char> & result,
    std::string path_to_file
);

#endif
