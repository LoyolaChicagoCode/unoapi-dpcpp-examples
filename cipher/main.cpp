// the encoding function was moved to a header file

// including for main
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
#include "readfile.h"


int main(const int argc, const char *const argv[])
{
    // {{UnoAPI:main-declarations:begin}}
    std::string msg{"abcdefg"};
    std::string key{"dogs"};
    std::string perf_output;
    bool run_sequentially{false};
    std::string text_file;
    ts_vector timestamps;
    std::string device_name;
    // {{UnoAPI:main-declarations:end}}

    // {{UnoAPI:main-cli-setup-and-parse:begin}}
    CLI::App app{"Substitution cipher"};
    app.option_defaults()->always_capture_default(true);
    app.add_option("-k, --key", key, "key value");
    app.add_option("-m, --msg", msg, "message");
    app.add_option("-f, --file", text_file)
        ->check(CLI::ExistingFile);
    app.add_flag("-s, --sequential", run_sequentially);

    CLI11_PARSE(app, argc, argv);
    //{{UnoAPI:main-cli-setup-and-parse:end}}

    // if file option, then load contents of file to string::msg
    if (text_file.size() > 0) {
        msg = file_to_string(text_file);
    }


    // {{UnoAPI:main-domain-setup:begin}}
    const auto key_size = key.size();
    const auto msg_size = msg.size();
    const auto ascii_a{97};
    const auto range{26};
    // {{UnoAPI:main-domain-setup:end}}

    // bound-checks:begin
    if (msg_size <= 0) {
        spdlog::error("invalid msg size");
    }

    if (key_size <= 0) {
        spdlog::error("invalid key size");
    }
    // bound-checks:end
    mark_time(timestamps, "Start");


    if (run_sequentially) {
	device_name = "sequential";
	std::vector<char> v1 = scramble(key, key_size, msg, msg_size, ascii_a, range);
	std::vector<char> result(msg_size);
	mark_time(timestamps,"Scramble alphabet");
	spdlog::info("starting sequential encoding");
	for (int i = 0; i < msg_size; i++) {
	    result[i] = v1[msg[i] - ascii_a];
	}
	mark_time(timestamps,"encoding plaintext");
	spdlog::info("results should be available now");
	//fmt::print("ciphertext = {}\n", result);
    }
    else {

        std::vector<char> v1 = scramble(key, key_size, msg, msg_size, ascii_a, range);
        mark_time(timestamps, "Scramble alphabet");
        spdlog::info("prepared ciphertext alphabet");


        sycl::buffer<char> msg_buf{msg.data(), sycl::range<1>{msg_size}};
        sycl::buffer<char> cipher_alphabet_buf{v1.data(), sycl::range<1>{range}};
        sycl::buffer<char> result_buf{sycl::range<1>{msg_size}};
        mark_time(timestamps, "Memmory allocation");
        spdlog::info("starting parallel encoding");


        sycl::device device {sycl::default_selector_v};
        sycl::queue Q{device};
        mark_time(timestamps,"Queue creation");
        device_name = Q.get_device().get_info<sycl::info::device::name>();
        spdlog::info("Device: {}", device_name);


        /* creating ciphertext */
        Q.submit([&](auto & h) {
            const sycl::accessor cipher_alphabet{cipher_alphabet_buf, h};
            const sycl::accessor msg{msg_buf, h};
            const sycl::accessor result{result_buf, h};
            h.parallel_for(msg_size, [=](const auto & i) {
                result[i] = cipher_alphabet[msg[i] - ascii_a];
            });
        });
        spdlog::info("done submitting to queue...waiting for results");
        mark_time(timestamps,"encoding plaintext");

    
        const sycl::host_accessor result{result_buf};
        const sycl::host_accessor cipher_alphabet{cipher_alphabet_buf};
        mark_time(timestamps,"Host data access");


        spdlog::info("printing plaintext, key, ciphertext");
        /* printing plaintext */
        //std::cout << "plaintext = " << msg;
        //std::cout << std::endl;
        /* printing keys */
        //std::cout << "key = " << key;
        //std::cout << std::endl;
        /* printing ciphertext */
        //std::cout << "ciphertext = ";
        //for (size_t i = 0; i < result.size(); i++) {
        //    std::cout << result[i];
        //}
        //std::cout << std::endl;
        mark_time(timestamps,"Output");
    }
    mark_time(timestamps, "DONE");
    spdlog::info("all done for now");

    print_timestamps(timestamps, perf_output, device_name);
    return 0;
}
