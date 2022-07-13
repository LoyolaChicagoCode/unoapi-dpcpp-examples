#include <CLI/CLI.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <CL/sycl.hpp>
#include <dpc_common.hpp>

#include "f.h"
#include "trapezoid.h"

template <class Indexable> void print_function_values(const Indexable & values, const double x_min, const double dx) {
    for (auto i{0UL}; i < values.size(); i++) {
        fmt::print("{}: f({}) = {}\n", i, x_min + i * dx, values[i]);
    }
}

int main(const int argc, const char * const argv[]) {
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
    const auto dx{(x_max - x_min) / number_of_trapezoids};
    const auto half_dx{0.5 * dx}; // precomputed for area calculation
    double result{0.0};

    spdlog::info("integrating function from {} to {} using {} trapezoid(s), dx = {}", x_min, x_max, number_of_trapezoids, dx);

    if (run_sequentially) {
        std::vector values(size, 0.0);

        spdlog::info("starting sequential integration");

        // populate vector with function values and add trapezoid area to result
        values[0] = f(x_min);
        for (auto i{1UL}; i <= number_of_trapezoids; i++) {
            // x = x_min + i * dx
            //   = x_min + i * (x_max - x_min) / number_of_trapezoids
            //   = (x_min * number_of_trapezoids + i * (x_max - x_min)) / number_of_trapezoids
            const auto x{(x_min * (number_of_trapezoids - i) + x_max * i) / number_of_trapezoids};
            values[i] = f(x);
            result += trapezoid(values[i - 1], values[i], half_dx);
        }

        if (show_function_values) {
            spdlog::info("showing function values");
            print_function_values(values, x_min, dx);
        }
    } else {
        // important: buffer NOT backed by host-allocated vector
        // this allows the data to live on the device until accessed on the host (if desired)
        sycl::buffer<double> v_buf{sycl::range<1>{size}};
        sycl::buffer<double> r_buf{&result, 1};

        spdlog::info("preparing for vectorized integration");

        // we use an in-order queue for this simple, sequential computation
        sycl::queue q{sycl::default_selector{}, dpc_common::exception_handler, sycl::property::queue::in_order()};
        spdlog::info("Device: {}", q.get_device().get_info<sycl::info::device::name>());

        // populate buffer with function values
        q.submit([&](auto & h) {
            const sycl::accessor v{v_buf, h};
            h.parallel_for(size, [=](const auto & index) {
                const double x{(x_min * (number_of_trapezoids - index) + x_max * index) / number_of_trapezoids};
                v[index] = f(x);
            });
        });

        // perform reduction into result
        q.submit([&](auto & h) {
            const sycl::accessor v{v_buf, h};
            const auto sum_reduction{sycl::reduction(r_buf, h, sycl::plus<>())};
            h.parallel_for(sycl::range<1>{number_of_trapezoids}, sum_reduction, [=](const auto & index, auto & sum) {
                sum.combine(trapezoid(v[index], v[index + 1], half_dx));
            });
        });

        spdlog::info("done submitting to queue...waiting for results");

        if (show_function_values) {
            const sycl::host_accessor values{v_buf};
            spdlog::info("showing function values");
            print_function_values(values, x_min, dx);
        }
    }

    spdlog::info("result should be available now");
    fmt::print("result = {}\n", result);

    return 0;
}
