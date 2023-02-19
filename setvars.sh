source /opt/intel/oneapi/setvars.sh --force

export CC=icx
export CXX=icpx

# required for compiling GoogleTest
export CFLAGS="-fp-model=precise"
export CXXFLAGS="-fsycl -fp-model=precise"

PATH=/opt/intel/oneapi/compiler/latest/linux/bin:$PATH

echo ":: set project-specific environment variables"
