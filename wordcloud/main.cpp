#include <algorithm>
#include <chrono>
#include <iostream>
#include <set>
#include <unordered_map>
#include <utility>
#include <cmath>

#include <CLI/CLI.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// dpc_common.hpp can be found in the dev-utilities include folder.
// e.g., $ONEAPI_ROOT/dev-utilities/<version>/include/dpc_common.hpp
#include <CL/sycl.hpp>
#include <dpc_common.hpp>

// idea:
// read all words
// group into uniformly sized buckets (except last one)
// show number of resulting buckets
// allow word cloud queries based on single buckets or contiguous range of buckets

// TODO better size_t or unsigned long or just long?

typedef std::vector<std::string> word_list;
typedef std::unordered_map<std::string, size_t> frequency_map;
typedef std::vector<std::string>::const_iterator word_iterator;

// TODO better indices or iterators for subranges of vectors?

void tally_frequencies(const word_list& source, const size_t left, const size_t right, frequency_map& freq_map) {
    spdlog::info("{}..{}", left, right);
    for (auto index = left; index < right; index ++) {
        const auto word = source[index];
        freq_map[word] ++;
    }
}

struct descending_by_value {
    template <typename T> bool operator()(const T& l, const T& r) const {
        if (l.second != r.second) {
            return l.second > r.second;
        }
        return l.first < r.first;
    }
};

void print_frequencies(const frequency_map& freq_map, const size_t how_many) {
    std::set<std::pair<std::string, size_t>, descending_by_value> freq_set(freq_map.begin(), freq_map.end());
    auto count = 0UL;
    for (auto kv = freq_set.begin(); kv != freq_set.end(); kv ++) {
        fmt::print("{}: {} ", kv->first, kv->second);
        if (++ count >= how_many) break;
    }
    fmt::print("\n");
}

int main(const int argc, const char *const argv[]) {
    constexpr size_t DEFAULT_BUCKET_SIZE{5};
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

    // TODO compare performance with loop not based on istream/cin

    word_list words(std::istream_iterator<std::string>(std::cin), {});
    const auto size = words.size();
    const auto num_of_buckets = ceil(1.0 * size / bucket_size);
    std::vector buckets(num_of_buckets, frequency_map{});

    spdlog::info("{} buckets", num_of_buckets);

    for (auto idx = buckets.begin(); idx < buckets.end(); idx ++) {
        const auto bucket_start = (idx - buckets.begin()) * bucket_size;
        const auto bucket_end = std::min(bucket_start + bucket_size, size);
        tally_frequencies(words, bucket_start, bucket_end, *idx);
    }

    for (auto idx = buckets.begin(); idx < buckets.end(); idx ++) {
        fmt::print("bucket {}: ", idx - buckets.begin());
        print_frequencies(*idx, top_n_words);
    }

//    std::ostream_iterator<std::string> out(std::cout, " ");
//    std::copy(words.begin(), words.end(), out);

    return 0;
}
