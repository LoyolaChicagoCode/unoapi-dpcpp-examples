source /opt/intel/oneapi/setvars.sh --force --include-intel-llvm

export CC=clang
export CXX=clang++

# required for compiling GoogleTest
export CFLAGS="-fp-model=precise"
export CXXFLAGS="-fsycl -fp-model=precise"

PATH=/opt/intel/oneapi/compiler/latest/linux/bin-llvm:$PATH

echo ":: set project-specific environment variables"
