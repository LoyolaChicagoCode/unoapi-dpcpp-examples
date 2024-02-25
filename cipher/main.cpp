#include <CLI/CLI.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <sycl/sycl.hpp>
#include <dpc_common.hpp>

#include <iostream>
#include <vector>

#include "scramble.h"
#include "timestamps.h"
#include "file_ops.h"

int main(const int argc, const char *const argv[])
{
    // main declarations begin
    std::string msg{"abcdefghijklmnopqrstuvwxyz"};
    std::string key{"Cyndy"};
    std::vector<char> substitution_alphabet;
    std::string perf_output;
    bool run_sequentially{false};
    bool encode{false};
    bool decode{false};
    std::string input_file_path;
    std::string output_file_path;
    ts_vector timestamps;
    std::string device_name;
    // main declarations end

    // cli setup and parse begin
    CLI::App app{"Substitution cipher"};
    app.option_defaults()->always_capture_default(true);
    app.add_option("-k, --key", key, "key value");
    app.add_option("-m, --msg", msg, "message");
    app.add_option("-i, --inputfile", input_file_path)
        ->check(CLI::ExistingFile);
    app.add_option("-o, --outputfile", output_file_path)
        ->check(CLI::ExistingFile);
    app.add_flag("-s, --sequential", run_sequentially);
    app.add_flag("-e, --encode", encode);
    app.add_flag("-d, --decode", decode);
    CLI11_PARSE(app, argc, argv);
    // cli setup and parse end

    // must encode or decode
    if (encode == decode) {
        spdlog::error("must specify -e --encode || -d --decode");
        return 1;
    }
    
    // file takes precendence over inline
    if (input_file_path.size() > 0) {
        msg = read_from_binary(input_file_path);
    }

    // main domain setup begin
    const auto msg_size = msg.size();
    const auto character_base{0};
    const auto character_range{128};
    // main domain setup end
    
    
    mark_time(timestamps, "start");
    // initializing alphabet begin
    spdlog::info("preparing alphabet vector");
    if (encode) {
        substitution_alphabet = scramble(key, character_base, character_range);
    }
    else {
        substitution_alphabet = unscramble(key, character_base, character_range);
    }
    mark_time(timestamps, "initializing alphabet vector");
    // initializing alphabet end
    
    // run sequential begin
    if (run_sequentially) {
        device_name = "sequential";
	    std::vector<char> result(msg_size);
        mark_time(timestamps, "memmory allocation");

	    spdlog::info("starting sequential encoding");
	    for (int i = 0; i < msg_size; i++) {
	        result[i] = substitution_alphabet[msg[i] - character_base];
	    }
	    mark_time(timestamps,"encoding plaintext");
        if (output_file_path.size() > 0) {
            spdlog::info("starting to write result to output file");
            write_to_binary(result, output_file_path);
        }
        mark_time(timestamps,"writing to file");
	    spdlog::info("results should be available now");
    }
    // run sequential end

    else {
        sycl::buffer<char> msg_buf{msg.data(), sycl::range<1>{msg_size}};
        sycl::buffer<char> substitution_alphabet_buf{substitution_alphabet.data(), sycl::range<1>{character_range}};
        sycl::buffer<char> result_buf{sycl::range<1>{msg_size}};
        mark_time(timestamps, "memmory allocation");
        spdlog::info("starting parallel encoding");

        sycl::device device {sycl::default_selector_v};
        sycl::queue Q{device};
        mark_time(timestamps,"Queue creation");
        device_name = Q.get_device().get_info<sycl::info::device::name>();
        spdlog::info("Device: {}", device_name);

        // performing substitution
        Q.submit([&](auto & h) {
            const sycl::accessor substitution_alphabet{substitution_alphabet_buf, h};
            const sycl::accessor msg{msg_buf, h};
            const sycl::accessor result{result_buf, h};
            h.parallel_for(msg_size, [=](const auto & i) {
                result[i] = substitution_alphabet[msg[i] - character_base];
            });
        });
        spdlog::info("done submitting to queue...waiting for results");
        mark_time(timestamps,"encoding plaintext");
    
        // host accessor to synchronize memory
        const sycl::host_accessor result{result_buf};
        mark_time(timestamps,"Host data access");
        
        spdlog::info("printing plaintext, key, ciphertext");
        mark_time(timestamps,"Output");
        if (output_file_path.size() > 0) {
            write_to_binary(result, output_file_path);
        }
    }
    mark_time(timestamps,"DONE");

    spdlog::info("all done for now");
    print_timestamps(timestamps, perf_output, device_name);
    return 0;
}