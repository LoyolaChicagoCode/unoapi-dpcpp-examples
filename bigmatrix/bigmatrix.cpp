#include <iostream>
#include <limits>
#include <chrono>

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

constexpr int M = 20000;

int main() {

  fmt::print("what up - this is fmt\n");
  spdlog::info("what up - this is spdlog");

  steady_clock::time_point zero;

  fmt::print("problem size: {}^2 integer matrix\n", M);

  sycl::queue q{sycl::property::queue::in_order()};

  fmt::print("Device: {}\n", q.get_device().get_info<sycl::info::device::name>());

  sycl::buffer<int, 2> m_buf(sycl::range(M, M));
  sycl::buffer<long> r_buf{sycl::range<1>{1}};

  fmt::print("starting to submit kernels to queue\n");
  zero = steady_clock::now();

  q.submit([&](auto &h) {
    sycl::accessor m(m_buf, h, sycl::write_only);

    h.parallel_for(sycl::range(M, M), [=](auto index) {
      m[index[0]][index[1]] = 1;
    });
  });

  q.submit([&](auto &h) {
    sycl::accessor m(m_buf, h, sycl::read_only);
    const auto sum_reduction{sycl::reduction(r_buf, h, sycl::plus<>())};

    h.parallel_for(sycl::range(M, M), sum_reduction, [=](const auto & index, auto & sum) {
        sum.combine(m[index[0]][index[1]]);
    });
  });

  spdlog::info("done submitting to queue...waiting for results");

  const sycl::host_accessor result{r_buf};
  spdlog::info("result should be available now");
  fmt::print("result = {}\n", result[0]);
}
