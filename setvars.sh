source /opt/intel/oneapi/setvars.sh

export CC=icx
export CXX=dpcpp

# required for compiling GoogleTest
export CFLAGS="-fp-model=precise"
export CXXFLAGS="-fp-model=precise"

PATH=/opt/intel/oneapi/compiler/latest/linux/bin:$PATH
