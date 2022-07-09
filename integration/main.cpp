#include <chrono>

#include <CLI/CLI.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// dpc_common.hpp can be found in the dev-utilities include folder.
// e.g., $ONEAPI_ROOT/dev-utilities/<version>/include/dpc_common.hpp
#include <CL/sycl.hpp>
#include <dpc_common.hpp>

#include "trapezoid.h"

double f(double x);

int main(const int argc, const char *const argv[]) {
    constexpr size_t DEFAULT_NUMBER_OF_TRAPEZOIDS{10};
    size_t number_of_trapezoids{DEFAULT_NUMBER_OF_TRAPEZOIDS};
    double x_min{0.0};
    double x_max{1.0};
    bool show_function_values{false};
    bool run_sequentially{false};

    CLI::App app{"Trapezoidal integration"};
    app.option_defaults()->always_capture_default(true);
    app.add_option("-n,--trapezoids", number_of_trapezoids, "number of trapezoids")->check(CLI::PositiveNumber.description(" >= 1"));
    app.add_option("-l,--lower,--xmin", x_min, "x min value");
    app.add_option("-u,--upper,--xmax", x_max, "x max value");
    app.add_flag("-v,--show_function_values", show_function_values);
    app.add_flag("-s,--sequential", run_sequentially);
    CLI11_PARSE(app, argc, argv);

    if (x_min > x_max) {
        spdlog::error("invalid range: [{}, {}]", x_min, x_max);
        return 1;
    }

    const auto size{number_of_trapezoids + 1};
    const auto dx = (x_max - x_min) / number_of_trapezoids;
    const auto half_dx = 0.5 * dx; // precomputed for area calculation

    spdlog::info("integrating function from {} to {} using {} trapezoid(s), dx = {}", x_min, x_max, number_of_trapezoids, dx);

    std::vector values(size, 0.0);
    double result{0};

    if (run_sequentially) {
        spdlog::info("starting sequential integration");

        values[0] = f(x_min);
        for (auto index = 1L; index <= number_of_trapezoids; index++) {
            // x = x_min + index * dx
            //   = x_min + index * (x_max - x_min) / number_of_trapezoids
            //   = (x_min * number_of_trapezoids + index * (x_max - x_min)) / number_of_trapezoids
            const auto x = (x_min * (number_of_trapezoids - index) + x_max * index) / number_of_trapezoids;
            values[index] = f(x);
            result += trapezoid(values[index - 1], values[index], half_dx);
        }
    } else {
        spdlog::info("preparing for vectorized integration");

        // we use an in-order queue for this simple, sequential computation
        sycl::queue q{sycl::default_selector{}, dpc_common::exception_handler, sycl::property::queue::in_order()};
        spdlog::info("Device: {}", q.get_device().get_info<sycl::info::device::name>());

        // important: buffer NOT backed by host-allocated vector
        // this allows the data to live on the device until accessed on the host (if desired)
        sycl::buffer<double> v_buf{sycl::range<1>{size}};
        sycl::buffer<double> r_buf{&result, 1};

        q.submit([&](auto &h) {
            const auto v{v_buf.get_access<sycl::access_mode::write>(h)};
            h.parallel_for(size, [=](const auto index) {
                const double x = (x_min * (number_of_trapezoids - index) + x_max * index) / number_of_trapezoids;
                v[index] = f(x);
            });
        });

        q.submit([&](auto &h) {
            const auto v{v_buf.get_access<sycl::access_mode::read>(h)};
            const auto sum_reduction{sycl::reduction(r_buf, h, sycl::plus<>())};
            h.parallel_for(sycl::range<1>{number_of_trapezoids}, sum_reduction, [=](const auto index, auto &sum) {
                sum.combine(trapezoid(v[index], v[index + 1], half_dx));
            });
        });

        spdlog::info("done submitting to queue...waiting for results");
    }
    spdlog::info("results should be available now");

    if (show_function_values) {

        // TODO explicitly copy from device (using host_accessor?)

        for (auto i{0UL}; i < size; i++) {
            fmt::print("{}: f({}) = {}\n", i, x_min + i * dx, values[i]);
        }
        fmt::print("\n");
    }
    fmt::print("result = {}\n", result);

    return 0;
}

double f(const double x) {
    return cos(x) * cos(x) + sin(x) * sin(x);
}
