# Build status

[![OneAPI CI with CMake](https://github.com/LoyolaChicagoCode/unoapi-dpcpp-examples/actions/workflows/oneapi-cmake.yml/badge.svg)](https://github.com/LoyolaChicagoCode/unoapi-dpcpp-examples/actions/workflows/oneapi-cmake.yml)

# Synopsis

A number of example programs for the [UnoAPI](https://unoapi.cs.luc.edu) curricular modules for high-performance computing using data-parallel C++ with Intel’s OneAPI under development at [Loyola University Chicago](https://luc.edu/cs).

All examples are intended to follow current practices, such as

- CMake
- modern C++ syntax (currently C++17 for best compatibility and stability)
- [GoogleTest](https://google.github.io/googletest)
- [CLI11](https://github.com/CLIUtils/CLI11) command-line option parser
- [spdlog](https://spdlog.docsforge.com) logging library
- [{fmt}](https://fmt.dev) output library
- [scnlib](https://scnlib.readthedocs.io) input library
- [magic_enum](https://github.com/Neargye/magic_enum) additional support for enums

Additional notes are being added to the [UnoAPI Wiki](https://github.com/LoyolaChicagoBooks/unoapi/wiki).

Please report any issues with these code examples [here](https://github.com/LoyolaChicagoCode/unoapi-dpcpp-examples/issues). 

# Debian-based Linux systems

## One-time setup

```shell
wget https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB
sudo apt-key add GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB
rm GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB
echo "deb https://apt.repos.intel.com/oneapi all main" | sudo tee /etc/apt/sources.list.d/oneAPI.list
sudo apt update
sudo apt install build-essential cmake intel-oneapi-compiler-dpcpp-cpp
git clone https://github.com/LoyolaChicagoCode/unoapi-dpcpp-examples
```

## Building the examples

```shell
cd unoapi-dpcpp-examples
source ./setvars.sh
cmake -DCMAKE_BUILD_TYPE=Debug -S . -B build
cmake --build build
```

## Running an example

After a successful build, you'll be able to run any of the examples in `unoapi-dpcpp-examples/build/bin`, e.g.: 

```shell
cd unoapi-dpcpp-examples
./build/bin/integration
```

# MacOS

The Intel DPC++ compiler is currently unavailable for MacOS.
We will add instructions if and when this changes. 

# Windows

The examples should run on Windows, and we are planning to add instructions in the future.
