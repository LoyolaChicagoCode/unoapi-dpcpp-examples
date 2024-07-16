#include <sycl/sycl.hpp>
#include <dpc_common.hpp>

#include <CLI/CLI.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "scramble.h"
#include "timestamps.h"
#include "file_ops.h"
#include "substitute.h"
#include "plf_nanotimer.h"

//
// re-run integration with parameters we are using for cipher
// add power function to parallel_for and sequential
//
//

// function print_output_message
// -----------------------------
// description: This fucntion prints the bytes
// contained in the argument output_message as characters.
//
template <class Iterable>
void print_output_message(Iterable & output_message)
{
    // ensuring argument output_message contains type byte
    static_assert(std::is_same<typename Iterable::value_type, std::byte>::value,
                  "Iterable must contain std::byte elements");
    fmt::print("output_message: ");
    std::for_each(output_message.begin(), output_message.end(), [](std::byte b) {
        fmt::print("{}", static_cast<char>(b));
    });
    fmt::print("\n");
}

int main(const int argc, const char *const argv[])
{
    // main declarations :: begin
    std::string perf_output;
    std::string device_name;
    std::string input_file_path;
    std::string output_file_path;
    std::string key_file_path;
    std::vector<std::tuple<std::string, double>> timestamps;
    double time_result;
    // main declarations :: end
    
    // main inits :: begin
    // note: We must maintain the byte order of the key, therefore we cannot use a set container.
    std::vector<std::byte> input_message{std::byte{72}, std::byte{101}, std::byte{108}, std::byte{108}, std::byte{111}, std::byte{33}};
    std::vector<std::byte> key{std::byte{67}, std::byte{121}, std::byte{110}, std::byte{100}, std::byte{121}, std::byte{33}};
    uint grain_size{100};
    const double f_double{10};
    bool run_sequentially{false};
    bool run_cpuonly{false};
    bool encode{false};
    bool decode{false};
    bool print_to_console{false};
    //main inits :: end

    // cli setup and parse begin
    CLI::App app{"Substitution cipher"};
    app.option_defaults()->always_capture_default(true);
    app.add_option("-g, --grainsize", grain_size)
        ->check(CLI::PositiveNumber.description(" >= 1"));
    app.add_option("-k, --key", key, "key value");
    app.add_option("-f, --keyfile", key_file_path)
        ->check(CLI::ExistingFile);
    app.add_option("-m, --msg", input_message, "message");
    app.add_option("-i, --inputfile", input_file_path)
        ->check(CLI::ExistingFile);
    app.add_option("-o, --outputfile", output_file_path)
        ->check(CLI::ExistingFile);
    app.add_flag("-s, --sequential", run_sequentially);
    app.add_flag("-c, --cpu-only", run_cpuonly);
    app.add_flag("-e, --encode", encode);
    app.add_flag("-d, --decode", decode);
    app.add_flag("-p, --print", print_to_console);
    CLI11_PARSE(app, argc, argv);
    // cli setup and parse end

    // must encode or decode
    if (encode == decode) {
        spdlog::error("must specify -e --encode || -d --decode");
        return 1;
    }
    
    plf::nanotimer time_total;
    time_total.start();
    
    // read key from file :: begin
    if (key_file_path.size()) {
        spdlog::info("reading key from key file: {}", key_file_path);
        plf::nanotimer time_key_vector;
        time_key_vector.start();
        if (read_from_binary(key, key_file_path) == 1) { return 1; }
        time_result = time_key_vector.get_elapsed_ns();
        mark_time(timestamps, time_result, "initializing key vector");
    }
    // read key from file end :: end
    
    // read input message from file :: begin
    if (input_file_path.size()) {
        spdlog::info("reading input message from input file: {}", input_file_path);
        plf::nanotimer time_input_vector;
        time_input_vector.start();
        if (read_from_binary(input_message, input_file_path) == 1) { return 1; }
        time_result = time_input_vector.get_elapsed_ns();
        mark_time(timestamps, time_result, "initializing input_message vector");
    }
    // read input message from file :: end

    // main domain setup :: begin
    plf::nanotimer time_domain;
    time_domain.start();
    const auto input_message_size = input_message.size();
    const auto output_message_size = input_message_size;
    const auto work_load{input_message_size / grain_size};
    constexpr auto decimal_begin{0};
    constexpr auto decimal_end{256};
    std::vector<std::byte> byte_map{decimal_end - decimal_begin};
    time_result = time_domain.get_elapsed_ns();
    mark_time(timestamps, time_result, "domain setup");
    // main domain setup :: end
    
    if (input_message_size % grain_size != 0) {
        spdlog::warn("input_message_size {} is not a multiple of grain_size {}", input_message_size, grain_size);
    }
    
    // initializing byte map :: begin
    spdlog::info("preparing byte map");
    plf::nanotimer time_byte_map_vector;
    time_byte_map_vector.start();
    if (encode) {
        byte_map = scramble(key, decimal_begin, decimal_end);
    }
    else {
        byte_map = unscramble(key, decimal_begin, decimal_end);
    }
    time_result = time_byte_map_vector.get_elapsed_ns();
    mark_time(timestamps, time_result, "initializing byte map vector");
    // initializing byte map :: end
    
    // run sequential begin
    if (run_sequentially) {
        plf::nanotimer time_sequential;
        time_sequential.start();
        device_name = "sequential";
        
        // memory allocation for output message :: begin
        spdlog::info("allocating memory for output message sequential");
        plf::nanotimer time_output_alloc;
        time_output_alloc.start();
        std::vector<std::byte> output_message(output_message_size);
        time_result = time_output_alloc.get_elapsed_ns();
        mark_time(timestamps, time_result, "output message vector memmory alloc");
        // memory allocation for output message :: end
        
        // sequential byte substitution :: begin
        spdlog::info("starting sequential byte substitution");
        plf::nanotimer time_seq_byte_sub;
        time_seq_byte_sub.start();
        for (int i = 0; i < work_load + 1; i++) {
            substitute<decltype(byte_map)>(byte_map, input_message, output_message, input_message_size, grain_size, i);
        }
        time_result = time_seq_byte_sub.get_elapsed_ns();
        mark_time(timestamps, time_result, "sequential byte substitution");
        // sequential byte substitution :: end
        
        // write output message to output file :: begin
        if (output_file_path.size()) {
            spdlog::info("preparing to write new msg to: {}", output_file_path);
            plf::nanotimer time_seq_output;
            time_seq_output.start();
            if (write_to_binary<std::vector<std::byte>>(output_message, output_file_path) == 1) { return 1; }
            time_result = time_seq_output.get_elapsed_ns();
            mark_time(timestamps, time_result, "write to output file");
        }
        // write output message to output file :: end
        
        // print output message to console :: begin
        if (print_to_console) {
            spdlog::info("preparing to print output_message to console");
            plf::nanotimer time_seq_print;
            time_seq_print.start();
            print_output_message<std::vector<std::byte>>(output_message);
            time_result = time_seq_print.get_elapsed_ns();
            mark_time(timestamps, time_result, "print to console");
        }
        // print output message to console :: end
        
        spdlog::info("output message should be available now");
        time_result = time_sequential.get_elapsed_ns();
        mark_time(timestamps, time_result, "sequential block");
    }
    // run sequential end
    
    // run parallel begin
    else {
        plf::nanotimer time_parallel;
        time_parallel.start();
        
        // allocate sycl buffer memory :: begin
        spdlog::info("allocating memory for sycl buffers");
        plf::nanotimer time_sycl_buf_alloc;
        time_sycl_buf_alloc.start();
        sycl::buffer<std::byte> byte_map_buf{byte_map.data(), sycl::range<1>{decimal_end - decimal_begin}};
        sycl::buffer<std::byte> input_message_buf{input_message.data(), sycl::range<1>{input_message_size}};
        sycl::buffer<std::byte> output_message_buf{sycl::range<1>{output_message_size}};
        sycl::buffer<std::byte> flag_buf{sycl::range<1>{1}};
        time_result = time_sycl_buf_alloc.get_elapsed_ns();
        mark_time(timestamps, time_result, "sycl buffer memmory alloc");
        // allocate sycl buffer memory :: end
        
        // sycl Q creation :: begin
        plf::nanotimer time_sycl_queue_create;
        time_sycl_queue_create.start();
        sycl::device device { run_cpuonly ? sycl::cpu_selector_v : sycl::default_selector_v };
        sycl::queue Q{device, dpc_common::exception_handler};
        time_result = time_sycl_queue_create.get_elapsed_ns();
        mark_time(timestamps, time_result, "queue creation");
        // sycl Q creation :: end
        
        device_name = Q.get_device().get_info<sycl::info::device::name>();
        spdlog::info("Device: {}", device_name);
        
        // populate output message buffer with new msg
        spdlog::info("preparing for parallel byte substitutions");
        Q.submit([&](auto & h) {
            // data transfer ocurring here
            const sycl::accessor byte_map_acc{byte_map_buf, h};
            const sycl::accessor input_message_acc{input_message_buf, h};
            const sycl::accessor output_message_acc{output_message_buf, h};
            const sycl::accessor flag_acc{flag_buf, h};
            
            // kernel code
            h.parallel_for(work_load + 1, [=](const auto & i) {
                substitute<decltype(byte_map_acc)>(byte_map_acc, input_message_acc, output_message_acc, input_message_size, grain_size, i);
            });
        });
        spdlog::info("done submitting to queue...waiting for results");
        
        // access flag buff to initiate work on target device :: begin
        spdlog::info("preparing flag access");
        plf::nanotimer time_parallel_byte_sub;
        time_parallel_byte_sub.start();
        const sycl::host_accessor flag{flag_buf};
        time_result = time_parallel_byte_sub.get_elapsed_ns();
        mark_time(timestamps, time_result, "parallel byte substitution");
        // access flag buff to initiate work on target device :: end
    
        // host accessor to synchronize memory :: begin
        spdlog::info("preparing output_message access");
        plf::nanotimer time_host_access;
        time_host_access.start();
        const sycl::host_accessor output_message{output_message_buf};
        time_result = time_host_access.get_elapsed_ns();
        mark_time(timestamps, time_result, "host data access");
        // host accessor to synchronize memory :: end
        
        // write output message to output file :: begin
        if (output_file_path.size()) {
            spdlog::info("preparing to write new msg to: {}", output_file_path);
            plf::nanotimer time_output;
            time_output.start();
            
            if (write_to_binary<decltype(output_message)>(output_message, output_file_path) == 1) { return 1; }
            
            time_result = time_output.get_elapsed_ns();
            mark_time(timestamps, time_result, "write to output file");
        }
        // write output message to output file :: end
        
        // print output message to console :: begin
        if (print_to_console) {
            spdlog::info("preparing to write output_message to console");
            plf::nanotimer time_print;
            time_print.start();
            
            print_output_message<decltype(output_message)>(output_message);
            
            time_result = time_print.get_elapsed_ns();
            mark_time(timestamps, time_result, "print to console");
        }
        // print output message to console :: end
        
        time_result = time_parallel.get_elapsed_ns();
        mark_time(timestamps, time_result, "parallel block");
    }
    // run parallel end
    
    time_result = time_total.get_elapsed_ns();
    mark_time(timestamps, time_result, "total time");
    
    spdlog::info("all done for now");
    print_timestamps(timestamps);
    return 0;
}