#include <chrono>
#include <cmath>

#include <CLI/CLI.hpp>
#include <fmt/format.h>
#include <scn/scn.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// dpc_common.hpp can be found in the dev-utilities include folder.
// e.g., $ONEAPI_ROOT/dev-utilities/<version>/include/dpc_common.hpp
#include <CL/sycl.hpp>
#include <dpc_common.hpp>


int main(const int argc, const char *const argv[]) {
    constexpr size_t DEFAULT_BUCKET_SIZE{100000};
    size_t bucket_size{DEFAULT_BUCKET_SIZE};
    bool run_sequentially{true};

    CLI::App app{"Moving word cloud"};
    app.option_defaults()->always_capture_default(true);
    app.add_option("-b,--bucket-size", bucket_size, "bucket size")->check(CLI::PositiveNumber.description(" >= 1"));
    app.add_flag("-s,--sequential", run_sequentially);
    CLI11_PARSE(app, argc, argv);

    fmt::print("hello!\n");

    return 0;
}
