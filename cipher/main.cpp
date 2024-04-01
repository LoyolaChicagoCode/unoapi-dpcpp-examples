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
    // main declarations begin
    std::string perf_output;
    ts_vector timestamps;
    std::string device_name;
    std::string input_file_path;
    std::string output_file_path;
    std::string key_file_path;
    // main declarations end
    
    // main inits begin
    // note: We must maintain the byte order of the key, therefore we cannot use a set container.
    std::vector<std::byte> input_message{std::byte{72}, std::byte{101}, std::byte{108}, std::byte{108}, std::byte{111}, std::byte{33}};
    std::vector<std::byte> key{std::byte{67}, std::byte{121}, std::byte{110}, std::byte{100}, std::byte{121}, std::byte{33}};
    bool run_sequentially{false};
    bool encode{false};
    bool decode{false};
    bool print_to_console{false};
    //main inits end

    // cli setup and parse begin
    CLI::App app{"Substitution cipher"};
    app.option_defaults()->always_capture_default(true);
    app.add_option("-k, --key", key, "key value");
    app.add_option("-f, --keyfile", key_file_path)
        ->check(CLI::ExistingFile);
    app.add_option("-m, --msg", input_message, "message");
    app.add_option("-i, --inputfile", input_file_path)
        ->check(CLI::ExistingFile);
    app.add_option("-o, --outputfile", output_file_path)
        ->check(CLI::ExistingFile);
    app.add_flag("-s, --sequential", run_sequentially);
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
    
    // start timer
    mark_time(timestamps, "start");
    
    // reading key from file begin - key file argument takes precedence over cmd line argument
    if (key_file_path.size()) {
        spdlog::info("reading key from key file: {}", key_file_path);
        if (read_from_binary(key, key_file_path) == 1) { return 1; }
        mark_time(timestamps, "initializing key vector");
    }
    // reading key from file end
    
    // reading input_message from file begin - input file argument takes precendence over cmd line argument
    if (input_file_path.size()) {
        spdlog::info("reading input message from input file: {}", input_file_path);
        if (read_from_binary(input_message, input_file_path) == 1) { return 1; }
        mark_time(timestamps, "initializing input_message vector");
    }
    // reading input_message from file end

    // main domain setup begin
    const auto input_message_size = input_message.size();
    const auto output_message_size = input_message_size;
    constexpr auto decimal_begin{0};
    constexpr auto decimal_end{256};
    std::vector<std::byte> byte_map{decimal_end - decimal_begin};
    // main domain setup end
    
    // initializing alphabet begin
    spdlog::info("preparing byte map");
    if (encode) {
        byte_map = scramble(key, decimal_begin, decimal_end);
    }
    else {
        byte_map = unscramble(key, decimal_begin, decimal_end);
    }
    mark_time(timestamps, "initializing byte map vector");
    // initializing alphabet end
    
    // run sequential begin
    if (run_sequentially) {
        device_name = "sequential";
        std::vector<std::byte> output_message(output_message_size);
        mark_time(timestamps, "output_message vector memmory allocation");
        
        spdlog::info("starting sequential byte substitution");
        for (int i = 0; i < input_message_size; i++) {
            output_message[i] = substitute<std::vector<std::byte>>(byte_map, input_message, decimal_begin, i);
        }
        mark_time(timestamps, "performing byte substitution on input_message sequentially");
        
        if (output_file_path.size()) {
            spdlog::info("preparing to write new msg to: {}", output_file_path);
            if (write_to_binary<std::vector<std::byte>>(output_message, output_file_path) == 1) { return 1; }
            mark_time(timestamps, "writing to output file");
        }
        
        if (print_to_console) {
            spdlog::info("preparing to print output_message to console");
            print_output_message<std::vector<std::byte>>(output_message);
            mark_time(timestamps, "printing to console");
        }
        
        spdlog::info("output message should be available now");
    }
    // run sequential end
    
    // run parallel begin
    else {
        // main parallel buffers
        sycl::buffer<std::byte> byte_map_buf{byte_map.data(), sycl::range<1>{decimal_end - decimal_begin}};
        sycl::buffer<std::byte> input_message_buf{input_message.data(), sycl::range<1>{input_message_size}};
        sycl::buffer<std::byte> output_message_buf{sycl::range<1>{output_message_size}};
        mark_time(timestamps, "sycl buffer memmory allocation");
        
        // main parallel queue
        sycl::device device {sycl::default_selector_v};
        sycl::queue Q{device};
        mark_time(timestamps, "queue creation");
        device_name = Q.get_device().get_info<sycl::info::device::name>();
        spdlog::info("Device: {}", device_name);
        
        // populate output message buffer with new msg
        spdlog::info("preparing for parallel byte substitutions");
        Q.submit([&](auto & h) {
            const sycl::accessor byte_map{byte_map_buf, h};
            const sycl::accessor input_message{input_message_buf, h};
            const sycl::accessor output_message{output_message_buf, h};
            
            h.parallel_for(input_message_size, [=](const auto & i) {
                output_message[i] = substitute<decltype(byte_map)>(byte_map, input_message, decimal_begin, i);
            });
        });
        spdlog::info("done submitting to queue...waiting for results");
        mark_time(timestamps, "performing byte substitutions on input_message in parallel");
    
        // host accessor to synchronize memory
        spdlog::info("preparing host accessor");
        const sycl::host_accessor output_message{output_message_buf};
        mark_time(timestamps, "host data access");
        
        if (output_file_path.size()) {
            spdlog::info("preparing to write new msg to: {}", output_file_path);
            if (write_to_binary<decltype(output_message)>(output_message, output_file_path) == 1) { return 1; }
            mark_time(timestamps, "writing to output file");
        }
        
        if (print_to_console) {
            spdlog::info("preparing to write output_message to console");
            print_output_message<decltype(output_message)>(output_message);
        }
    }
    // run parallel end
    
    mark_time(timestamps, "DONE");
    spdlog::info("all done for now");
    print_timestamps(timestamps, perf_output, device_name);
    return 0;
}