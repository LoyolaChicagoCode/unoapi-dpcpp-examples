#include <sycl/sycl.hpp>
#include <dpc_common.hpp>

#include <CLI/CLI.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "timestamps.h"
#include "plf_nanotimer.h"

int verify_matrix_squared(std::string & filename, int & n)
{
    int rows, columns;
    std::ifstream input(filename);
    
    if (!input.is_open()) {
        std::cout << "The file could not be opened.";
        return 1;
    }
    
    input >> rows >> columns;
    n = rows;
    
    if (rows != columns) { return 1; }
    
    input.close();
    return 0;
}

int populate_matrix(std::string & filename, sycl::buffer<int, 2> & C_buf)
{
    const sycl::host_accessor C(C_buf);
    std::ifstream input(filename);
    
    if (!input.is_open()) {
        std::cout << "The file could not be opened.";
        return 1;
    }
    
    int rows, columns;
    input >> rows >> columns;
    
    if (rows != columns) { return 1; }
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            input >> C[i][j];
        }
    }
    
    input.close();
    return 0;
    
}

void print_matrix(const sycl::host_accessor<int, 2> & A, int n)
{
    std::cout << "Transitive Closure A of C: " << std::endl;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            std::cout << A[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

int main(const int argc, const char *const argv[])
{
    // main declarations::begin
    std::string input; // file to adjacency matrix
    int n; // squared matrix dimension
    std::string device_name;
    std::vector<std::tuple<std::string, double>> timestamps;
    double time_result;
    // main declarations::end
    
    // main inits::begin
    bool run_sequentially{false};
    bool run_cpuonly{false};
    bool print{false};
    // main inits::end
    
    // cli setup and parse::begin
    CLI::App app{"Transitive Closure"};
    app.option_defaults()->always_capture_default(true);
    app.add_option("-i, --input", input);
    app.add_flag("-s, --sequential", run_sequentially);
    app.add_flag("-c, --cpu-only", run_cpuonly);
    app.add_flag("-p, --print", print);
    CLI11_PARSE(app, argc, argv);
    // cli setup and parse::end
    
    plf::nanotimer time_total;
    time_total.start();
    
    // get dimensions of adjacency matrix::begin
    spdlog::info("attempting to verify square-ness of adjacency matrix using input: {}", input);
    if (verify_matrix_squared(input, n) == 1) {
        spdlog::error("error when verifying square-ness of adjacency matrix using input: {}", input);
        return 1;
    }
    // get dimensions of adjacency matrix::end
    
    // declare sycl buffers::begin
    plf::nanotimer time_buffer_dec;
    time_buffer_dec.start();
    sycl::buffer<int, 2> C_buf(sycl::range(n, n));
    sycl::buffer<int, 2> A_buf(sycl::range(n, n));
    time_result = time_buffer_dec.get_elapsed_ns();
    mark_time(timestamps, time_result, "declare sycl buffers");
    // declare sycl buffers::end
    
    // init sycl buffers::begin
    plf::nanotimer time_buffer_init;
    time_buffer_init.start();
    spdlog::info("attempting to populate adjacency matrix using input: {}", input);
    if (populate_matrix(input, C_buf) == 1) {
        spdlog::error("error when populating adjacency matrix using input: {}", input);
        return 1;
    }
    time_result = time_buffer_init.get_elapsed_ns();
    mark_time(timestamps, time_result, "populate sycl buffer with adj matrix");
    // init sycl buffers::end
    
    // run sequential::begin
    if (run_sequentially) {
        spdlog::info("starting sequential execution block");
        plf::nanotimer time_sequential; // time sequential block::begin
        time_sequential.start();
        
        // init host accessors::begin
        spdlog::info("initializing host accessors");
        plf::nanotimer time_init_host;
        time_init_host.start();
        const sycl::host_accessor C(C_buf);
        const sycl::host_accessor A(A_buf);
        time_result = time_init_host.get_elapsed_ns();
        mark_time(timestamps, time_result, "host accessor creation");
        // init host accessors::end
        
        // Warshall procedure step 1::begin
        spdlog::info("starting Warshall procedure step 1");
        plf::nanotimer time_warshall_p1;
        time_warshall_p1.start();
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                A[i][j] = C[i][j];
            }
        }
        time_result = time_warshall_p1.get_elapsed_ns();
        mark_time(timestamps, time_result, "Warshall procedure step 1");
        // Warshall procedure step 1::end
        
        // Warshall procedure step 2::begin
        spdlog::info("starting Warshall procedure step 2");
        plf::nanotimer time_warshall_p2;
        time_warshall_p2.start();
        for (int k = 0; k < n; k++) {
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    A[i][j] = A[i][j] || A[i][k] * A[k][j];
                }
            }
        }
        time_result = time_warshall_p2.get_elapsed_ns();
        mark_time(timestamps, time_result, "Warshall procedure step 2");
        // Warshall procedure step 2::end
        
        time_result = time_sequential.get_elapsed_ns(); // time sequential block::end
        mark_time(timestamps, time_result, "sequential block");
        
        if (print) {
            print_matrix(A, n);
        }
    }
    // run sequential::end
    
    // run parallel::begin
    else {
        spdlog::info("starting parallel execution block");
        // flag buffer for timing parallel execution
        spdlog::info("initializing flag");
        plf::nanotimer time_flag_init;
        time_flag_init.start();
        sycl::buffer<int> flag_buf{sycl::range<1>{1}};
        time_result = time_flag_init.get_elapsed_ns();
        mark_time(timestamps, time_result, "init flag buf");
        
        // sycl queue creation::begin
        spdlog::info("setting up queue");
        plf::nanotimer time_device_init;
        time_device_init.start();
        sycl::device device{run_cpuonly ? sycl::cpu_selector_v : sycl::default_selector_v};
        sycl::queue q{device, dpc_common::exception_handler, sycl::property::queue::in_order()};
        device_name = q.get_device().get_info<sycl::info::device::name>();
        spdlog::info("device: {}", device_name);
        time_result = time_device_init.get_elapsed_ns();
        mark_time(timestamps, time_result, "queue creation");
        // sycl queue creation::end

        // Warshall procedure
        // step 1: copy adjacency matrix C into matrix A
        // for i := 1 to n do 
        //   for j := 1 to n do
        //     A[i, j] := C[i, j]
        //
        q.submit([&](auto &h) {
            const sycl::accessor C(C_buf, h, sycl::read_only);
            const sycl::accessor A(A_buf, h, sycl::write_only);

            h.parallel_for(sycl::range(n, n), [=](auto index) {
                A[index] = C[index];

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
        for (int k = 0; k < n; k++) {

            q.submit([&](auto &h) {
                const sycl::accessor A(A_buf, h, sycl::write_only);
                
                h.parallel_for(sycl::range(n, n), [=](auto index) {
                    A[index] = A[index] || A[index[0]][k] * A[k][index[1]];

                });
            });
        }
        // access flag buff to initiate work on device::begin
        spdlog::info("preparing flag access");
        plf::nanotimer time_parallel;
        time_parallel.start();
        const sycl::host_accessor flag{flag_buf};
        time_result = time_parallel.get_elapsed_ns();
        mark_time(timestamps, time_result, "Warshall procedure step 1 & 2");
        // access flag buff to initiate work on device::end
        
        if (print) {
            const sycl::host_accessor A{A_buf};
            print_matrix(A, n);
        }
    }
    // run parallel::end
    
    time_result = time_total.get_elapsed_ns();
    mark_time(timestamps, time_result, "total time");
    print_timestamps(timestamps);
    spdlog::info("all done");
    return 0;
}