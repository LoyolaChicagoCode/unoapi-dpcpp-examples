#include <algorithm>
#include <chrono>
#include <iostream>
#include <unordered_map>

#include <CLI/CLI.hpp>
#include <fmt/format.h>
//#include <scn/scn.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// dpc_common.hpp can be found in the dev-utilities include folder.
// e.g., $ONEAPI_ROOT/dev-utilities/<version>/include/dpc_common.hpp
#include <sycl/sycl.hpp>
#include <dpc_common.hpp>


// idea:
// read all words
// group into uniformly sized buckets (except last one)
// show number of resulting buckets
// allow word cloud queries based on single buckets or contiguous range of buckets


void tally_frequencies(const std::vector<std::string>& source, const size_t left, const size_t right, std::unordered_map<std::string, size_t>& freq_map) {
    fmt::print("l = {} r = {}\n", left, right);
    for (auto index = left; index < right; index ++) {
        const auto word = source.at(index);
        freq_map[word] ++;
    }
}

void print_frequencies(const std::unordered_map<std::string, size_t>& freq_map) {
    for (auto kv = freq_map.begin(); kv != freq_map.end(); kv ++) {
        fmt::print("{}: {} ", kv->first, kv->second);
    }
    fmt::print("\n");
}

int main(const int argc, const char *const argv[]) {
    constexpr size_t DEFAULT_BUCKET_SIZE{3};
    constexpr size_t DEFAULT_TOP_N_WORDS{3};
    size_t bucket_size{DEFAULT_BUCKET_SIZE};
    size_t top_n_words{DEFAULT_TOP_N_WORDS};
    bool run_sequentially{true};

    CLI::App app{"Moving word cloud"};
    app.option_defaults()->always_capture_default(true);
    app.add_option("-b,--bucket-size", bucket_size, "bucket size")->check(CLI::PositiveNumber.description(" >= 1"));
    app.add_option("-n,--top-n-words", top_n_words, "top n words")->check(CLI::PositiveNumber.description(" >= 1"));
    app.add_flag("-s,--sequential", run_sequentially);
    CLI11_PARSE(app, argc, argv);

    // https://stackoverflow.com/questions/69797846/reading-words-from-a-file-into-dynamically-allocated-array

    std::vector all(std::istream_iterator<std::string>(std::cin), {});

    const auto size = all.size();
    for (auto i = 0UL; i < size; i += bucket_size) {
        const auto b = std::min(i + bucket_size, size);
        fmt::print("{}..{}\n", i, b);
        std::unordered_map<std::string, size_t> freq_table;
        tally_frequencies(all, i, i + b, freq_table);
//        print_frequencies(freq_table);
    }

    fmt::print("read {} words\n", all.size());
    for (auto i = all.begin(); i != all.end(); i ++) {
        fmt::print("{}: {} ", *i, i->length());
    }

    return 0;
}
