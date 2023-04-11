#include <iostream>
#include <string>
#include "CLI/CLI.hpp"
#include "fmt/format.h"

int main(int argc, char* argv[]) {
    CLI::App app{"Command-line interface example"};

    // Add arguments for each built-in data type
    int i = 42;
    app.add_option("-i,--int", i, "Integer argument");

    double d = 3.14;
    app.add_option("-d,--double", d, "Double argument");

    float f = 2.71;
    app.add_option("-f,--float", f, "Float argument");

    bool b = true;
    app.add_flag("-b,--bool", b, "Boolean argument");

    std::string s = "Hello, world!";
    app.add_option("-s,--string", s, "String argument");

    // Parse the command-line arguments
    CLI11_PARSE(app, argc, argv);

    // Print the parsed arguments
    fmt::print("Parsed arguments:\n");
    fmt::print("  int: {}\n", i);
    fmt::print("  double: {}\n", d);
    fmt::print("  float: {}\n", f);
    fmt::print("  bool: {}\n", b);
    fmt::print("  string: {}\n", s);
    return 0;
}


