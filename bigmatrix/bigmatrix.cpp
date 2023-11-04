#include <iostream>
#include <limits>
#include <chrono>

#include <CLI/CLI.hpp>
// TODO discuss why this is necessary
#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <sycl/sycl.hpp>
#include <dpc_common.hpp>

using std::chrono::steady_clock;
using std::chrono::milliseconds;
using std::chrono::duration_cast;

constexpr int DEFAULT_M = 20000;

int main(const int argc, const char * const argv[]) {
  uint M{DEFAULT_M};

  fmt::print("what up - this is fmt\n");
  spdlog::info("what up - this is spdlog");

  CLI::App app{"Square matrix simple map-reduce example"};
  app.option_defaults()->always_capture_default(true);
  app.add_option("-s,--size", M, "size")->check(CLI::PositiveNumber.description(" >= 1"));
  CLI11_PARSE(app, argc, argv);

  steady_clock::time_point zero;

  fmt::print("problem size: {}^2 integer matrix\n", M);

  sycl::queue q{sycl::property::queue::in_order()};

  fmt::print("Device: {}\n", q.get_device().get_info<sycl::info::device::name>());

  // oneAPI figures out where to allocate these buffers
  sycl::buffer<uint, 2> m_buf(sycl::range(M, M));
  sycl::buffer<ulong> r_buf{sycl::range<1>{1}};

  fmt::print("starting to submit kernels to queue\n");
  zero = steady_clock::now();

  // initialize matrix on accelerator
  q.submit([&](auto &h) {
    sycl::accessor m(m_buf, h, sycl::write_only);

    h.parallel_for(sycl::range(M, M), [=](auto index) {
      m[index[0]][index[1]] = 1;
    });
  });

  // map: transform matrix elements on accelerator
  q.submit([&](auto &h) {
    sycl::accessor m(m_buf, h);

    h.parallel_for(sycl::range(M, M), [=](auto index) {
      m[index[0]][index[1]] *= 2;
    });
  });

  // reduce: add matrix elements on accelerator
  q.submit([&](auto &h) {
    sycl::accessor m(m_buf, h, sycl::read_only);
    const auto sum_reduction{sycl::reduction(r_buf, h, sycl::plus<>())};

    h.parallel_for(sycl::range(M, M), sum_reduction, [=](const auto & index, auto & sum) {
        sum.combine(m[index[0]][index[1]]);
    });
  });

  spdlog::info("done submitting jobs to queue...waiting for results");

  const sycl::host_accessor result{r_buf};
  spdlog::info("result should be available now");
  fmt::print("result = {}\n", result[0]);
}
