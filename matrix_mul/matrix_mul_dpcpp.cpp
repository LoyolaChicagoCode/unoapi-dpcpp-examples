//==============================================================
// Copyright © 2020 Intel Corporation
//
// SPDX-License-Identifier: MIT
// =============================================================

/**
 * Matrix_mul multiplies two large matrices both the CPU and the offload device,
 * then compares results. If the code executes on both CPU and the offload
 * device, the name of the offload device and a success message are displayed.
 *
 * For comprehensive instructions regarding DPC++ Programming, go to
 * https://software.intel.com/en-us/oneapi-programming-guide and search based on
 * relevant terms noted in the comments.
 */

#include <sycl/sycl.hpp>
#include <iostream>
#include <limits>
#include <chrono>

// TODO discuss why this is necessary
#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// dpc_common.hpp can be found in the dev-utilities include folder.
// e.g., $ONEAPI_ROOT/dev-utilities/<version>/include/dpc_common.hpp
#include "dpc_common.hpp"

using namespace std;
using namespace sycl;

using std::chrono::steady_clock;
using std::chrono::milliseconds;
using std::chrono::duration_cast;

/**
 * Each element of the product matrix c[i][j] is computed from a unique row and
 * column of the factor matrices, a[i][k] and b[k][j]
 */

// Matrix size constants.
constexpr int m_size = 150 * 8;  // Must be a multiple of 8.
constexpr int M = m_size / 8;
constexpr int N = m_size / 4;
constexpr int P = m_size / 2;

/**
 * Perform matrix multiplication on host to verify results from device.
 */
int VerifyResult(const host_accessor<float, 2> c_back, steady_clock::time_point zero);

int main() {
  int result;

  fmt::print("what up - this is fmt\n");
  spdlog::info("what up - this is spdlog");

  steady_clock::time_point zero;


  // Initialize the device queue with the default selector. The device queue is
  // used to enqueue kernels. It encapsulates all states needed for execution.
  try {
    sycl::queue q{sycl::default_selector_v, dpc_common::exception_handler};

    cout << "Device: " << q.get_device().get_info<info::device::name>() << "\n";

    // Create 2D buffers for matrices

    buffer<float, 2> a_buf(range(M, N));
    buffer<float, 2> b_buf(range(N, P));
    buffer<float, 2> b_buf_t(range(P, N));
    buffer<float, 2> c_buf(range(M, P));

    cout << "Problem size: c(" << M << "," << P << ") = a(" << M << "," << N
         << ") * b(" << N << "," << P << ")\n";

    fmt::print("starting to submit kernels to queue\n");
    zero = steady_clock::now();

    // Using three command groups to illustrate execution order. The use of
    // first two command groups for initializing matrices is not the most
    // efficient way. It just demonstrates the implicit multiple command group
    // execution ordering.

    // Submit command group to queue to initialize matrix a
    q.submit([&](auto &h) {
      // Get write only access to the buffer on a device.
      accessor a(a_buf, h, write_only);

      // Execute kernel.
      h.parallel_for(range(M, N), [=](auto index) {
        // Each element of matrix a is 1.
        a[index] = 1.0f;
      });
    });

    // Submit command group to queue to initialize matrix b
    q.submit([&](auto &h) {
      // Get write only access to the buffer on a device
      accessor b(b_buf, h, write_only);

      // Execute kernel.
      h.parallel_for(range(N, P), [=](auto index) {
        // Each column of b is the sequence 1,2,...,N
        b[index] = index[0] + 1.0f;
      });
    });
      
    //  b transpose-> b_t
    q.submit([&](auto &h) {
      accessor b(b_buf, h, read_only);
      accessor b_t(b_buf_t, h, write_only);
    
      h.parallel_for(range(M, N), [=](auto index) {
        int row = index[0];
        int col = index[1];
        b_t[row][col] = b[col][row];
      });
    });

    // Submit command group to queue to multiply matrices: c = a * b
    q.submit([&](auto &h) {
      // Read from a and b, write to c
      // Update using b_t: to multiply a[row] by b_t[row] to calculate c[index].
      accessor a(a_buf, h, read_only);
      accessor b_t(b_buf_t, h, read_only);
      accessor c(c_buf, h, write_only);

      int width_a = a_buf.get_range()[1];

      // Execute kernel.
      h.parallel_for(range(M, P), [=](auto index) {
        // Get global position in Y direction.
        int row = index[0];
        // Get global position in X direction. (using matrix b_t, no need for global pos of col)
        //int col = index[1];

        float sum = 0.0f;

        // Compute the result of one element of c
        for (int i = 0; i < width_a; i++) {
          sum += a[row][i] * b_t[row][i];
        }

        c[index] = sum;
      });
    });
      
    // verify result in scope of c_buf
    const host_accessor c_back{c_buf};
    result = VerifyResult(c_back, zero);
      
  } catch (sycl::exception const &e) {
    cout << "An exception is caught while multiplying matrices.\n";
    terminate();
  }

  cout << "Result of matrix multiplication using DPC++: ";

  return result;
}

bool ValueSame(float a, float b) {
  return fabs(a - b) < numeric_limits<float>::epsilon();
}

int VerifyResult(const host_accessor<float, 2> c_back, const steady_clock::time_point zero) {

  auto now = steady_clock::now();
  fmt::print("time to compute results in ms: {}\n", duration_cast<milliseconds>(now - zero).count());

  // Check that the results are correct by comparing with host computing.
  int i, j, k;

  // 2D arrays on host side.
  float(*a_host)[N] = new float[M][N];
  float(*b_host)[P] = new float[N][P];
  float(*c_host)[P] = new float[M][P];

  // Each element of matrix a is 1.
  for (i = 0; i < M; i++)
    for (j = 0; j < N; j++) a_host[i][j] = 1.0f;

  // Each column of b_host is the sequence 1,2,...,N
  for (i = 0; i < N; i++)
    for (j = 0; j < P; j++) b_host[i][j] = i + 1.0f;

  // c_host is initialized to zero.
  for (i = 0; i < M; i++)
    for (j = 0; j < P; j++) c_host[i][j] = 0.0f;

  for (i = 0; i < M; i++) {
    for (k = 0; k < N; k++) {
      // Each element of the product is just the sum 1+2+...+n
      for (j = 0; j < P; j++) {
        c_host[i][j] += a_host[i][k] * b_host[k][j];
      }
    }
  }

  bool mismatch_found = false;

  // Compare host side results with the result buffer from device side: print
  // mismatched data 5 times only.
  int print_count = 0;

  for (i = 0; i < M; i++) {
    for (j = 0; j < P; j++) {
      if (!ValueSame(c_back[i][j], c_host[i][j])) {
        cout << "Fail - The result is incorrect for element: [" << i << ", "
             << j << "], expected: " << c_host[i][j]
             << ", but found: " << c_back[i][j] << "\n";
        mismatch_found = true;
        print_count++;
        if (print_count == 5) break;
      }
    }

    if (print_count == 5) break;
  }

  auto now2 = steady_clock::now();
  fmt::print("time to verify results in ms: {}\n", duration_cast<milliseconds>(now2 - now).count());

  delete[] a_host;
  delete[] b_host;
  delete[] c_host;

  if (!mismatch_found) {
    cout << "Success - The results are correct!\n";
    return 0;
  } else {
    cout << "Fail - The results mismatch!\n";
    return -1;
  }
}