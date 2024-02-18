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


int main(const int argc, const char *const argv[])
{
    // {{UnoAPI:main-declarations:begin}}
    std::string msg{"abcdefg"};
    std::string key{"dogs"};
    bool run_sequentially{false};
    std::string text_file;
    // {{UnoAPI:main-declarations:end}}

    // {{UnoAPI:main-cli-setup-and-parse:begin}}
    CLI::App app{"Substitution cipher"};
    app.option_defaults()->always_capture_default(true);
    app.add_option("-k, --key", key, "key value");
    app.add_option("-m, --msg", msg, "message");
    app.add_flag("-s, --sequential", run_sequentially);

    CLI11_PARSE(app, argc, argv);
    //{{UnoAPI:main-cli-setup-and-parse:end}}

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

    if (run_sequentially) {
        std::cout << "run sequentially not yet implemented" << std::endl;
        return 0;
    }

    else {
        std::vector<char> v1 = scramble(key, key_size, msg, msg_size, ascii_a, range);
        sycl::queue Q;
        sycl::buffer<char> msg_buf{msg.data(), sycl::range<1>{msg_size}};
        sycl::buffer<char> cipher_alphabet_buf{v1.data(), sycl::range<1>{range}};
        sycl::buffer<char> result_buf{sycl::range<1>{msg_size}};

        /* creating ciphertext */
        Q.submit([&](auto & h) {
            const sycl::accessor cipher_alphabet{cipher_alphabet_buf, h};
            const sycl::accessor msg{msg_buf, h};
            const sycl::accessor result{result_buf, h};
            h.parallel_for(msg_size, [=](const auto & i) {
                result[i] = cipher_alphabet[msg[i] - ascii_a];
            });
        });
    
        const sycl::host_accessor result{result_buf};
        const sycl::host_accessor cipher_alphabet{cipher_alphabet_buf};

        /* printing plaintext */
        std::cout << "plaintext = " << msg;
        std::cout << std::endl;
    
        /* printing keys */
        std::cout << "key = " << key;
        std::cout << std::endl;

        /* printing ciphertext */
        std::cout << "ciphertext = ";
        for (size_t i = 0; i < result.size(); i++) {
            std::cout << result[i];
        }
        std::cout << std::endl;
        return 0;
    }

}
