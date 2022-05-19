#include <chrono>

#include <CLI/CLI.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// dpc_common.hpp can be found in the dev-utilities include folder.
// e.g., $ONEAPI_ROOT/dev-utilities/<version>/include/dpc_common.hpp
#include <CL/sycl.hpp>
#include <oneapi/dpl/random>
#include <dpc_common.hpp>
//#include <sycl/ext/intel/ac_types/ac_int.hpp>

int main(const int argc, const char *const argv[]) {
    constexpr size_t DEFAULT_NUMBER_OF_PLAYERS{4};
    constexpr uint64_t DEFAULT_NUMBER_OF_DARTS{1000000};
    size_t number_of_players{DEFAULT_NUMBER_OF_PLAYERS};
    uint64_t number_of_darts{DEFAULT_NUMBER_OF_DARTS};
    bool randomize{false};
    bool use_ranlux{false};

    CLI::App app{"Monte Carlo algorithm for estimating pi"};
    app.add_option("-p,--players", number_of_players, "number of players");
    app.add_option("-n,--darts", number_of_darts, "number of darts per player");
    app.add_flag("-r,--randomize", randomize, "randomize dart locations");
    app.add_flag("-l,--ranlux", use_ranlux, "use ranlux instead of LCG (minstd) for random number generation");
    CLI11_PARSE(app, argc, argv);

    spdlog::info("{} players are going to throw {} darts each", number_of_players, number_of_darts);
    spdlog::info("using {} engine with real distribution", use_ranlux ? "ranlux" : "minstd");
    spdlog::info("randomization is {}", randomize ? "on" : "off");

    const auto seed = randomize ? time(nullptr) : 0;
    std::vector<uint64_t> counts(number_of_players, 0);
    uint64_t sum{0};

    {
        sycl::queue q{sycl::default_selector{}, dpc_common::exception_handler};
        sycl::buffer<uint64_t> c_buf{counts.data(), sycl::range<1>(counts.size())};
        sycl::buffer<uint64_t> s_buf{&sum, 1};

        spdlog::info("Device: {}", q.get_device().get_info<sycl::info::device::name>());
        spdlog::info("Max workgroup size: {}", q.get_device().get_info<sycl::info::device::max_work_group_size>());

        q.submit([&](auto &h) {
            const auto c = c_buf.get_access<sycl::access_mode::write>(h);

            h.parallel_for(number_of_players, [=](const auto index) {
                const auto offset = 37 * index.get_linear_id() + 13;
                oneapi::dpl::minstd_rand minstd(seed, offset);
                oneapi::dpl::ranlux48 ranlux(seed, offset);

                //constexpr double R{1.0};
                //oneapi::dpl::uniform_real_distribution<double> distr(0.0, R);
                constexpr uint64_t R{3037000493UL}; // largest prime <= sqrt(ULONG_MAX / 2)
                oneapi::dpl::uniform_int_distribution<uint64_t> distr(0, R);
                const auto r_square{R * R};

                auto darts_within_circle{0UL};
                for (auto i{0UL}; i < number_of_darts; i++) {
                    const auto x{use_ranlux ? distr(ranlux) : distr(minstd)};
                    const auto y{use_ranlux ? distr(ranlux) : distr(minstd)};
                    const auto d_square{x * x + y * y};
                    if (d_square <= r_square)
                        darts_within_circle++;
                }
                c[index] = darts_within_circle;
            });
        });

        q.submit([&](auto &h) {
            const auto c{c_buf.get_access<sycl::access_mode::read>(h)};
            const auto sum_reduction{sycl::reduction(s_buf, h, sycl::plus<>())};

            h.parallel_for(sycl::range<1>{number_of_players}, sum_reduction, [=](const auto index, auto &sum) {
                sum.combine(c[index]);
            });
        });

        spdlog::info("done submitting to queue...waiting for results");
    }

    for (auto i{0UL}; i < number_of_players; i++) {
        spdlog::info("result[{}] = {}", i, counts[i]);
    }
    spdlog::info("sum = {}", sum);

    const double pi{4.0 * sum / (number_of_players * number_of_darts)};
    fmt::print("pi = {}\n", pi);

    return 0;
}
