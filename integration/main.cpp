#include <CLI/CLI.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <CL/sycl.hpp>
#include <dpc_common.hpp>

#include "f.h"
#include "trapezoid.h"

// {{UnoAPI:main-print-function-values:begin}}
// function template to avoid code repetition (DRY)
// works as long as the first argument has a size() method and an indexing operator[]
template <class Indexable> void print_function_values(const Indexable & values, const double x_min, const double dx, const uint x_precision, const uint y_precision) {
    for (auto i{0UL}; i < values.size(); i++) {
        fmt::print("{}: f({:.{}f}) = {:.{}f}\n", i, x_min + i * dx, x_precision, values[i], y_precision);
    }
}
// {{UnoAPI:main-print-function-values:end}}

int main(const int argc, const char * const argv[]) {
    // {{UnoAPI:main-declarations:begin}}
    constexpr size_t DEFAULT_NUMBER_OF_TRAPEZOIDS{10};
    size_t number_of_trapezoids{DEFAULT_NUMBER_OF_TRAPEZOIDS};
    double x_min{0.0};
    double x_max{1.0};
    bool show_function_values{false};
    bool run_sequentially{false};
    bool run_cpuonly{false};
    uint x_precision{1};
    uint y_precision{1};
    // {{UnoAPI:main-declarations:end}}

    // {{UnoAPI:main-cli-setup-and-parse:begin}}
    CLI::App app{"Trapezoidal integration"};
    app.option_defaults()->always_capture_default(true);
    app.add_option("-n,--trapezoids", number_of_trapezoids, "number of trapezoids")->check(CLI::PositiveNumber.description(" >= 1"));
    app.add_option("-l,--lower,--xmin", x_min, "x min value");
    app.add_option("-u,--upper,--xmax", x_max, "x max value");
    app.add_flag("-v,--show-function-values", show_function_values);
    app.add_flag("-s,--sequential", run_sequentially);
    app.add_flag("-c,--cpu-only", run_cpuonly);
    app.add_option("-x,--x-format-precision", x_precision, "decimal precision for x values")->check(CLI::PositiveNumber.description(" >= 1"));
    app.add_option("-y,--y-format-precision", y_precision, "decimal precision for y (function) values")->check(CLI::PositiveNumber.description(" >= 1"));
 
    CLI11_PARSE(app, argc, argv);
    // {{UnoAPI:main-cli-setup-and-parse:end}}

    if (x_min > x_max) {
        spdlog::error("invalid range: [{}, {}]", x_min, x_max);
        return 1;
    }


    // {{UnoAPI:main-domain-setup:begin}}
    const auto size{number_of_trapezoids + 1};
    const auto dx{(x_max - x_min) / number_of_trapezoids};
    const auto half_dx{0.5 * dx}; // precomputed for area calculation
    // {{UnoAPI:main-domain-setup:end}}

    spdlog::info("integrating function from {} to {} using {} trapezoid(s), dx = {}", x_min, x_max, number_of_trapezoids, dx);

    // {{UnoAPI:main-sequential-option:begin}}
    if (run_sequentially) {
        std::vector values(size, 0.0);
        double result{0.0};

        spdlog::info("starting sequential integration");

        // populate vector with function values and add trapezoid area to result
        values[0] = f(x_min);
        for (auto i{0UL}; i < number_of_trapezoids; i++) {
            values[i + 1] = f(x_min + i * dx);
            result += trapezoid(values[i], values[i + 1], half_dx);
        }

        spdlog::info("result should be available now");
        fmt::print("result = {}\n", result);

        if (show_function_values) {
            spdlog::info("showing function values");
            print_function_values(values, x_min, dx, x_precision, y_precision);
        }
    }
    // {{UnoAPI:main-sequential-option:end}}
    
    else {
        // important: buffer NOT explicitly backed by host-allocated vector
        // this allows the data to live on the device until accessed on the host (if desired)
        // {{UnoAPI:main-parallel-buffers:begin}}
        sycl::buffer<double> v_buf{sycl::range<1>{size}};
        sycl::buffer<double> r_buf{sycl::range<1>{1}};
        // {{UnoAPI:main-parallel-buffers:end}}

        spdlog::info("preparing for vectorized integration");

        // {{UnoAPI:main-parallel-devices:begin}}
        sycl::device_selector * device_selector = nullptr;

        if (run_cpuonly) {
            device_selector = new sycl::cpu_selector{};
        } else {
            device_selector = new sycl::default_selector{};
        }
        // {{UnoAPI:main-parallel-devices:end}}

        // we use an in-order queue for this simple, sequential computation
        // {{UnoAPI:main-parallel-inorder-q:begin}}
        sycl::queue q{*device_selector, dpc_common::exception_handler, sycl::property::queue::in_order()};
        spdlog::info("Device: {}", q.get_device().get_info<sycl::info::device::name>());
        // {{UnoAPI:main-parallel-inorder-q:end}}

        // populate buffer with function values
        // {{UnoAPI:main-parallel-submit-parallel-for:begin}}
        q.submit([&](auto & h) {
            const sycl::accessor v{v_buf, h};
            h.parallel_for(size, [=](const auto & index) {
                v[index] = f(x_min + index * dx);
            });
        }); // end of command group
        // {{UnoAPI:main-parallel-submit-parallel-for:end}}

        // perform reduction into result
        // {{UnoAPI:main-parallel-submit-reduce:begin}}
        q.submit([&](auto & h) {
            const sycl::accessor v{v_buf, h};
            const auto sum_reduction{sycl::reduction(r_buf, h, sycl::plus<>())};
            h.parallel_for(sycl::range<1>{number_of_trapezoids}, sum_reduction, [=](const auto & index, auto & sum) {
                sum.combine(trapezoid(v[index], v[index + 1], half_dx));
            });
        }); // end of command group
        // {{UnoAPI:main-parallel-submit-reduce:end}}

        spdlog::info("done submitting to queue...waiting for results");

        // {{UnoAPI:main-parallel-gather-on-host:begin}}
        const sycl::host_accessor result{r_buf};
        spdlog::info("result should be available now");
        fmt::print("result = {}\n", result[0]);
        // {{UnoAPI:main-parallel-gather-on-host:end}}

        // {{UnoAPI:main-parallel-show-results-log:begin}}
        if (show_function_values) {
            spdlog::info("preparing function values");
            const sycl::host_accessor values{v_buf};
            spdlog::info("showing function values");
            print_function_values(values, x_min, dx, x_precision, y_precision);
        }
        // {{UnoAPI:main-parallel-show-results-log:end}}

        // {{UnoAPI:main-parallel-cleanup:begin}}
        delete device_selector;
        // {{UnoAPI:main-parallel-cleanup:end}}
    }
    // end of scope waits for the queued work to complete

    spdlog::info("all done for now");
    return 0;
}
