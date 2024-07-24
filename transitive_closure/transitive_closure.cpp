#include <sycl/sycl.hpp>
#include <dpc_common.hpp>

#include <CLI/CLI.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "timestamps.h"
#include "plf_nanotimer.h"
#include "graph.h"

// This function is meant to populate a two-dimensional vector with adjacency matrix values.
// I had the idea to use a host_accessor to perform the Warshall procedure sequentially
// on the CPU. I came across the article referenced below outlining why this might be a
// bad idea. This function is the result.
// https://www.intel.com/content/www/us/en/docs/oneapi/optimization-guide-gpu/2024-2/performance-impact-of-usm-and-buffers.html#:~:text=The%20same%20is,should%20be%20avoided

int main(const int argc, const char *const argv[])
{
    // Main declarations.
    std::vector<std::vector<int>> adj_matrix;
    std::vector<int> linear_adj_matrix;
    std::vector<std::vector<int>> sequential_result;
    std::string device_name;
    std::vector<std::tuple<std::string, double>> timestamps;
    double time_result;
    
    // Main inits.
    int num_edges{380}; // Default
    int num_vertices{200}; // Default
    bool run_sequential{false};
    bool run_parallel{false};
    bool run_cpuonly{false};
    bool print{false};
    
    // CLI setup and parse.
    CLI::App app{"Transitive Closure"};
    app.option_defaults()->always_capture_default(true);
    app.add_option("-e, --edges", num_edges);
    app.add_option("-v, --vertices", num_vertices);
    app.add_flag("-s, --sequential", run_sequential);
    app.add_flag("-p, --parallel", run_parallel);
    app.add_flag("-c, --cpu-only", run_cpuonly);
    //app.add_flag("-p, --print", print);
    CLI11_PARSE(app, argc, argv);
    
    plf::nanotimer time_total;
    time_total.start();
    // Init parallel and sequential result.
    spdlog::info("Initializing result structures");
    sycl::buffer<int, 2> parallel_result(sycl::range(num_vertices, num_vertices));
    sequential_result.resize(num_vertices, std::vector<int>(num_vertices, 0));

    // Initialize adjacency matrix.
    spdlog::info("Generating simple directed graph with edges: {} and vertices: {}", num_edges, num_vertices);
    if (generate_simple_directed_graph(adj_matrix, num_edges, num_vertices) == 1) {
        return 1;
    }

    // Initialize linear adjacency matrix.
    spdlog::info("Linearizing adjacency matrix");
    linearize(adj_matrix, linear_adj_matrix, num_vertices);
    
    // run sequential::begin
    if (run_sequential) {
        spdlog::info("Run sequential begin");
        plf::nanotimer time_sequential;
        time_sequential.start();

        // Warshall procedure step 1
        for (int i = 0; i < num_vertices; i++) {
            for (int j = 0; j < num_vertices; j++) {
                sequential_result[i][j] = adj_matrix[i][j];
            }
        }
        
        // Warshall procedure step 2
        for (int k = 0; k < num_vertices; k++) {
            for (int i = 0; i < num_vertices; i++) {
                for (int j = 0; j < num_vertices; j++) {
                    sequential_result[i][j] = sequential_result[i][j] || sequential_result[i][k] && sequential_result[k][j];
                }
            }
        }
        time_result = time_sequential.get_elapsed_ns();
        mark_time(timestamps, time_result, "Run sequential");
        spdlog::info("Run sequential end");
    }
    // run sequential::end
    
    // run parallel::begin
    if (run_parallel) {
        spdlog::info("Run parallel begin");
        plf::nanotimer time_parallel;
        time_parallel.start();
        
        // Loading linear adjacency matrix into buffer.
        sycl::buffer<int> C_buf(linear_adj_matrix.data(), sycl::range<1>{linear_adj_matrix.size()});
        
        // sycl queue creation:
        spdlog::info("setting up queue");
        sycl::device device{run_cpuonly ? sycl::cpu_selector_v : sycl::default_selector_v};
        sycl::queue q{device, dpc_common::exception_handler, sycl::property::queue::in_order()};
        device_name = q.get_device().get_info<sycl::info::device::name>();
        spdlog::info("device: {}", device_name);

        // Warshall procedure
        // step 1: copy adjacency matrix C into matrix A
        // for i := 1 to n do 
        //   for j := 1 to n do
        //     A[i, j] := C[i, j]
        //
        
        // Copying Linear C into 2-D A.
        q.submit([&](auto &h) {
            const sycl::accessor C(C_buf, h, sycl::read_only);
            const sycl::accessor A(parallel_result, h, sycl::write_only);

            h.parallel_for(sycl::range(num_vertices, num_vertices), [=](auto index) {
                A[index] = C[index[0] * num_vertices + index[1]];

            });
        });

        // Warshall procedure
        // step 2: compute the transitive closure of C as A
        //
        // for k := 1 to n do
        //   for i := 1 to n do
        //     for j := 1 to n do
        //       if A[i, j] = false then
        //         A[i, j] := A[i, k] and A[k, j]
        //
        // note: A[index] = A[i][j]
        //
        for (int k = 0; k < num_vertices; k++) {
            q.submit([&](auto &h) {
                const sycl::accessor A(parallel_result, h, sycl::write_only);
                
                h.parallel_for(sycl::range(num_vertices, num_vertices), [=](auto index) {
                    A[index] = A[index] || A[index[0]][k] && A[k][index[1]];

                });
            });
        }
        // Init work on device.
        spdlog::info("Preparing host access");
        const sycl::host_accessor host_access{parallel_result};
        time_result = time_parallel.get_elapsed_ns();
        mark_time(timestamps, time_result, "Run parallel");
        spdlog::info("Run parallel end");
    }
    // run parallel::end

    if (run_sequential && run_parallel) {
        // Compare outputs.
        spdlog::info("Comparing results begin");
        plf::nanotimer time_compare;
        time_compare.start();
        
        const sycl::host_accessor A_back{parallel_result};
        for (int i = 0; i < num_vertices; i++) {
            for (int j = 0; j < num_vertices; j++) {
                if (A_back[i][j] != sequential_result[i][j]) {
                    spdlog::info("Results do not match!");
                    return 1;
                }
            }
        }
        time_result = time_compare.get_elapsed_ns();
        mark_time(timestamps, time_result, "Comparing results");
        spdlog::info("Results match!");
        spdlog::info("Comparing results end");
    }
    time_result = time_total.get_elapsed_ns();
    mark_time(timestamps, time_result, "Time Total");
    print_timestamps(timestamps);
    spdlog::info("all done");
    return 0;
}