// MAIN
// 1. Parse CLI.
// 2. Set up device.
// 3. Allocate USM memory.
// 4. Allocate Buffer memory.
// 5. Run sequential ? Use USM else use Buffer.
// 6. Run parallel ? use USM else use Buffer.
//
//
// For buffer execution:
// 1. Allocate and initialize graph vector on host.
// 2. Shove graph vector into sycl buffer C.
// 3. Copy sycl buffer C to sycl buffer A (Warshall P1).
// 4. Do computation on sycl buffer A (Warshall P1).
// 5. Access sycl buffer A on host.
// 6. Complete.
//
//
// For USM execution:
// 1. Allocate and initialize graph pointer on host.
// 2. Copy contents of graph into USM C.
// 3. Copy USM C to USM A (Warshall P1).
// 4. Do computation on USM A (Warshall P2).
// 5. Access USM on host.
// 6. Complete.
//
// TODO: look moore into malloc_shared

#include <sycl/sycl.hpp>
#include <dpc_common.hpp>

#include <CLI/CLI.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "timestamps.h"
#include "plf_nanotimer.h"
#include "graph.h"

int verify_results(sycl::buffer<int, 2> &a_parallel, std::vector<int> A_sequential, int vertices);
int verify_results(int *A_parallel, int *A_sequential, int vertices);
void warshall_parallel_p1(sycl::queue &q, sycl::buffer<int, 2> &a, std::vector<int> &c, int vertices);
void warshall_parallel_p2(sycl::queue &q, sycl::buffer<int, 2> &a, int vertices);
void warshall_sequential_p1(std::vector<int> &A, std::vector<int> &C, int vertices);
void warshall_sequential_p2(std::vector<int> &A, int vertices);
void warshall_parallel_p1(sycl::queue &q, int *A, int *C, int vertices);
void warshall_parallel_p2(sycl::queue &q, int *A, int vertices);
void warshall_sequential_p1(int *A, int *C, int vertices);
void warshall_sequential_p2(int *A, int vertices);


int main(const int argc, const char *const argv[])
{
    // Main declarations.
    std::vector<std::tuple<std::string, double>> timestamps;
    double time_result;
    
    // Default values.
    int edges{380};
    int vertices{200};
    bool run_sequential{false};
    bool run_parallel{false};
    bool run_cpuonly{false};
    bool use_usm{false};
    bool print{false};
    
    // CLI setup and parse.
    CLI::App app{"Transitive Closure"};
    app.option_defaults()->always_capture_default(true);
    app.add_option("-e, --edges", edges);
    app.add_option("-v, --vertices", vertices);
    app.add_flag("-s, --sequential", run_sequential);
    app.add_flag("-p, --parallel", run_parallel);
    app.add_flag("-c, --cpu-only", run_cpuonly);
    app.add_flag("-u, --use-usm", use_usm);
    CLI11_PARSE(app, argc, argv);
    
    plf::nanotimer time_total;
    time_total.start();

    // Sycl queue creation
    spdlog::info("setting up queue");
    plf::nanotimer time_queue_creation;
    time_queue_creation.start();
    sycl::device device{run_cpuonly ? sycl::cpu_selector_v : sycl::default_selector_v};
    sycl::queue q{device, dpc_common::exception_handler};
    std::string device_name = q.get_device().get_info<sycl::info::device::name>();
    uint device_max_compute_units = q.get_device().get_info<sycl::info::device::max_compute_units>();
    spdlog::info("device name: {}, device max compute units: {}", device_name, device_max_compute_units);
    time_result = time_queue_creation.get_elapsed_ns();
    mark_time(timestamps, time_result, "Sycl queue creation");



    // Allocate memory using USM and pointers
    plf::nanotimer time_memory_alloc_usm;
    time_memory_alloc_usm.start();
    int *graph_usm = sycl::malloc_shared<int>(vertices * vertices, q);
    int *sequential_usm = sycl::malloc_shared<int>(vertices * vertices, q);
    int *parallel_usm = sycl::malloc_shared<int>(vertices * vertices, q);

    if ((graph_usm == nullptr) || (sequential_usm == nullptr) ||
        (parallel_usm == nullptr)) {
      if (graph_usm != nullptr) free(graph_usm, q);
      if (sequential_usm != nullptr) free(sequential_usm, q);
      if (parallel_usm != nullptr) free(parallel_usm, q);

      spdlog::error("USM memory allocation failure");
      return -1;
    }
    time_result = time_memory_alloc_usm.get_elapsed_ns();
    mark_time(timestamps, time_result, "USM memory allocation");

    plf::nanotimer time_init_buffer;
    time_init_buffer.start();

    if (!generate_simple_directed_linear(graph_usm, edges, vertices)) {
        spdlog::info("(USM) Failed to generate simple directed graph with edges: {} and vertices: {}", edges, vertices);
        if (graph_usm != nullptr) free(graph_usm, q);
        if (sequential_usm != nullptr) free(sequential_usm, q);
        if (parallel_usm != nullptr) free(parallel_usm, q);
        return -1;
    }
    time_result = time_init_buffer.get_elapsed_ns();
    mark_time(timestamps, time_result, "USM graph initialization");



    // Allocate memory for parallel and sequential execution.
    plf::nanotimer time_memory_alloc;
    time_memory_alloc.start();
    spdlog::info("Allocating memory for graph data.");
    std::vector<int> graph_vec(vertices * vertices);
    spdlog::info("Allocating memory for sequential result.");
    std::vector<int> sequential_vec(vertices * vertices);
    spdlog::info("Allocating memory for parallel result.");
    sycl::buffer<int, 2> parallel_buf(sycl::range(vertices, vertices));
    time_result = time_memory_alloc.get_elapsed_ns();
    mark_time(timestamps, time_result, "Buffer memory allocation");

    // Initialize adjacency matrix.
    spdlog::info("Generating simple directed graph with edges: {} and vertices: {}", edges, vertices);
    if (!generate_simple_directed_linear(graph_vec, edges, vertices)) {
        spdlog::info("(Buffer) Failed to generate simple directed graph with edges: {} and vertices: {}", edges, vertices);
        return 1;
    }

    if (run_sequential) {
        plf::nanotimer time_sequential;
        if (use_usm) {
            // TODO implement overload function for 
            spdlog::info("Warshall sequential with USM begin");
            time_sequential.start();
            warshall_sequential_p1(sequential_usm, graph_usm, vertices);
            warshall_sequential_p2(sequential_usm, vertices);
            time_result = time_sequential.get_elapsed_ns();
            mark_time(timestamps, time_result, "Warshall USM sequential");
            spdlog::info("Warshall USM sequential end");
        }
        else { // Use buffers.
            spdlog::info("Warshall sequential with buffers begin");
            time_sequential.start();
            warshall_sequential_p1(sequential_vec, graph_vec, vertices);
            warshall_sequential_p2(sequential_vec, vertices);
            time_result = time_sequential.get_elapsed_ns();
            mark_time(timestamps, time_result, "Warshall sequential");
            spdlog::info("Warshall sequential end");
        }
    }

    if (run_parallel) {
        plf::nanotimer time_parallel;
        if (use_usm) {
            //TODO - implement overloaded function for warshall parallel w/ pointers.
            spdlog::info("Warshall parallel with USM begin");
            time_parallel.start();
            warshall_parallel_p1(q, parallel_usm, graph_usm, vertices);
            warshall_parallel_p2(q, parallel_usm, vertices);
            time_result = time_parallel.get_elapsed_ns();
            mark_time(timestamps, time_result, "Warshall USM parallel");
            spdlog::info("Warshall USM parallel end");
        }
        else { // Use buffers.
            spdlog::info("Warshall parallel with Buffers begin");
            time_parallel.start();
            warshall_parallel_p1(q, parallel_buf, graph_vec, vertices);
            warshall_parallel_p2(q, parallel_buf, vertices);
            const sycl::host_accessor trigger_work{parallel_buf};
            time_result = time_parallel.get_elapsed_ns();
            mark_time(timestamps, time_result, "Warshall Buffers parallel");
            spdlog::info("Warshall Buffers parallel end");
        }
    }

    if (run_sequential && run_parallel) {
        spdlog::info("Verify results begin");
        plf::nanotimer time_verify;
        time_verify.start();
        if (use_usm) {
            if (!verify_results(parallel_usm, sequential_usm, vertices)) {
                spdlog::warn("Results do not match!");
            }
            else {
                spdlog::info("Results match!");
            }
        }
        else {
            if (!verify_results(parallel_buf, sequential_vec, vertices)) {
                spdlog::warn("Results do not match!");
            }
            else {
                spdlog::info("Results match!");
            }
        }
        time_result = time_verify.get_elapsed_ns();
        mark_time(timestamps, time_result, "Verify results");
        spdlog::info("Verify results end");
    }

    if (graph_usm != nullptr) { free(graph_usm, q); }
    if (sequential_usm != nullptr) { free(sequential_usm, q); }
    if (parallel_usm != nullptr) { free(parallel_usm, q); }

    time_result = time_total.get_elapsed_ns();
    mark_time(timestamps, time_result, "Total time");
    print_timestamps(timestamps);
    spdlog::info("All done");
    return 0;
}

// Function compares sequential and parallel result.
int verify_results(sycl::buffer<int, 2> &a_parallel, std::vector<int> A_sequential, int vertices)
{
    const sycl::host_accessor A_parallel{a_parallel};
    for (int i = 0; i < vertices; i++) {
        for (int j = 0; j < vertices; j++) {
            if (A_parallel[i][j] != A_sequential[i * vertices + j]) {
                return -1;
            }
        }
    }
    return 1;
}

int verify_results(int *A_parallel, int *A_sequential, int vertices)
{
    for (int i = 0; i < vertices; i++) {
        for (int j = 0; j < vertices; j++) {
            if (A_parallel[i * vertices + j] != A_sequential[i * vertices + j]) {
                return -1;
            }
        }
    }
    return 1;
}

// Warshall procedure
// Step 1: copy adjacency matrix C into matrix A
// for i := 1 to n do 
//   for j := 1 to n do
//     A[i, j] := C[i, j]
//
//
// Parallel part 1 w/ Buffers.
void warshall_parallel_p1(sycl::queue &q, sycl::buffer<int, 2> &a, std::vector<int> &c, int vertices)
{
    sycl::buffer<int> c_buf(c.data(), sycl::range<1>{c.size()});
    q.submit([&](auto &h) {
        const sycl::accessor C(c_buf, h, sycl::read_only);
        const sycl::accessor A(a, h, sycl::write_only);

        h.parallel_for(sycl::range(vertices, vertices), [=](auto index) {
            int i = index[0];
            int j = index[1];
            A[i][j] = C[i * vertices + j];
        });
    });
}

// Parallel part 1 w/ USM.
//TODO
void warshall_parallel_p1(sycl::queue &q, int *A, int *C, int vertices)
{
    q.submit([&](auto &h) {
        h.parallel_for(sycl::range(vertices * vertices), [=](auto index) {
            A[index] = C[index];
        });
    });
}

// Parallel part 2 w/ USM.
//TODO
void warshall_parallel_p2(sycl::queue &q, int *A, int vertices)
{
    for (int k = 0; k < vertices; k++) {
        q.submit([&](auto &h) {
            h.parallel_for(sycl::range(vertices * vertices), [=](auto index) {
                int i = index / vertices;
                int j = index % vertices;
                A[i * vertices + j] = A[i * vertices + j] || A[i * vertices + k] && A[k * vertices + j];
            });
        });
    }
}

// Warshall procedure
// Step 2: compute the transitive closure of C as A
//
// for k := 1 to n do
//   for i := 1 to n do
//     for j := 1 to n do
//       if A[i, j] = false then
//         A[i, j] := A[i, k] and A[k, j]
//
// note: A[index] = A[i][j]
//
//
// Parallel part 2 w/ Buffers.
void warshall_parallel_p2(sycl::queue &q, sycl::buffer<int, 2> &a, int vertices)
{
    for (int k = 0; k < vertices; k++) {
        q.submit([&](auto &h) {
            const sycl::accessor A(a, h, sycl::read_write);

            h.parallel_for(sycl::range(vertices, vertices), [=](auto index) {
                int i = index[0];
                int j = index[1];
                A[i][j] = A[i][j] || A[i][k] && A[k][j];
            });
        });
    }
}

// Sequential part 1 w/ Buffers.
void warshall_sequential_p1(std::vector<int> &A, std::vector<int> &C, int vertices)
{
    for (int i = 0; i < vertices; i++) {
        for (int j = 0; j < vertices; j++) {
            A[i * vertices + j] = C[i * vertices + j];
        }
    }
}

// Sequential part 1 w/ USM.
void warshall_sequential_p1(int *A, int *C, int vertices)
{
    for (int i = 0; i < vertices; i++) {
        for (int j = 0; j < vertices; j++) {
            A[i * vertices + j] = C[i * vertices + j];
        }
    }
}

// Sequential part 2 w/ Buffers.
void warshall_sequential_p2(std::vector<int> &A, int vertices)
{
    for (int k = 0; k < vertices; k++) {
        for (int i = 0; i < vertices; i++) {
            for (int j = 0; j < vertices; j++) {
                A[i * vertices + j] = A[i * vertices + j] || A[i * vertices + k] && A[k * vertices + j];
            }
        }
    }
}

// Sequential part 2 w/ USM.
void warshall_sequential_p2(int *A, int vertices)
{
    for (int k = 0; k < vertices; k++) {
        for (int i = 0; i < vertices; i++) {
            for (int j = 0; j < vertices; j++) {
                A[i * vertices + j] = A[i * vertices + j] || A[i * vertices + k] && A[k * vertices + j];
            }
        }
    }  
}