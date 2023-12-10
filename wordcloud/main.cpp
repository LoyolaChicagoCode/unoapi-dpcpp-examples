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
#include <sycl/sycl.hpp>
#include <dpc_common.hpp>

// idea:
// read all words
// group into uniformly sized buckets (except last one)
// show number of resulting buckets
// allow word cloud queries based on single buckets or contiguous range of buckets

// TODO better size_t or unsigned long or just long?

typedef std::vector<std::string> word_list;
typedef std::unordered_map<std::string, size_t> frequency_map;

// TODO better indices or iterators for subranges of vectors?

// tally frequencies within index range into the map
void tally_frequencies(const word_list& source, const size_t left, const size_t right, frequency_map& freq_map) {
//    spdlog::info("{}..{}", left, right);
    for (auto index = left; index < right; index ++) {
        const auto & word = source[index];
        freq_map[word] ++;
    }
}

// comparison for sorted sets of word-frequency pairs
struct descending_by_value {
    template <typename T> bool operator()(const T& l, const T& r) const {
        if (l.second != r.second) {
            return l.second > r.second;
        }
        return l.first < r.first;
    }
};

// print the most frequent words in a map
void print_frequencies(const frequency_map& freq_map, const size_t how_many) {
    // load the frequencies into a sorted set of word-frequency pairs
    std::set<std::pair<std::string, size_t>, descending_by_value> freq_set(freq_map.begin(), freq_map.end());
    auto count = 0UL;
    // print only the most frequent words and their frequencies
    for (const auto & kv : freq_set) {
        fmt::print("{}: {} ", kv.first, kv.second);
        if (++ count >= how_many) break;
    }
    fmt::print("\n");
}

int main(const int argc, const char *const argv[]) {
    constexpr size_t DEFAULT_BUCKET_SIZE{5};
    constexpr size_t DEFAULT_TOP_N_WORDS{3};
    constexpr size_t DEFAULT_MIN_WORD_LENGTH{6};

    size_t bucket_size{DEFAULT_BUCKET_SIZE};
    size_t top_n_words{DEFAULT_TOP_N_WORDS};
    size_t min_word_length{DEFAULT_MIN_WORD_LENGTH};
    bool run_sequentially{true};
    std::vector<std::pair<size_t, size_t>> bucket_ranges;

    CLI::App app{"Moving word cloud"};
    app.option_defaults()->always_capture_default(true);
    app.add_option("-b,--bucket-size", bucket_size, "bucket size")->check(CLI::PositiveNumber.description(" >= 1"));
    app.add_option("-n,--top-n-words", top_n_words, "top n words")->check(CLI::PositiveNumber.description(" >= 1"));
    app.add_option("-m,--min-word-length", min_word_length, "min word length")->check(CLI::PositiveNumber.description(" >= 1"));
    app.add_flag("-s,--sequential", run_sequentially, "run sequentially (without accelerator)");
    app.add_option("-r,--bucket-ranges", bucket_ranges, "bucket ranges");
    CLI11_PARSE(app, argc, argv);

    // TODO option for words to exclude, e.g., Google, Digitized
    // TODO strip punctuation

    // https://stackoverflow.com/questions/69797846/reading-words-from-a-file-into-dynamically-allocated-array

    // TODO compare performance with loop not based on istream/cin

    std::vector<std::string> words;
    // filter short words
    std::remove_copy_if(
            std::istream_iterator<std::string>(std::cin), {},
            std::back_inserter(words),
            [=](const auto & word) { return word.length() < min_word_length; }
            );

    const auto size = words.size();
    const auto num_of_buckets = ceil(1.0 * size / bucket_size);
    std::vector buckets(num_of_buckets, frequency_map{});

    spdlog::info("{} buckets", num_of_buckets);

    // TODO replace with CLI11 validator for bucket ranges above
    // log bucket ranges provided as an option
    for (const auto r : bucket_ranges) {
        if (r.first <= r.second && r.second < num_of_buckets) {
            spdlog::info("bucket range {}-{}", r.first, r.second);
        } else {
            spdlog::warn("ignoring invalid bucket range {}-{}", r.first, r.second);
            // TODO remove invalid range from container
        }
    }

    if (run_sequentially) {
        // tally word frequencies for each bucket
        for (auto idx = buckets.begin(); idx < buckets.end(); idx++) {
            const auto bucket_start = (idx - buckets.begin()) * bucket_size;
            const auto bucket_end = std::min(bucket_start + bucket_size, size);
            tally_frequencies(words, bucket_start, bucket_end, *idx);
        }

        if (bucket_ranges.empty()) {
            for (auto idx = buckets.begin(); idx < buckets.end(); idx++) {
                fmt::print("bucket {}: ", idx - buckets.begin());
                print_frequencies(*idx, top_n_words);
            }
        } else {
            // if valid bucket ranges are listed, combine them and print the result
            for (const auto r: bucket_ranges) {
                if (r.first <= r.second && r.second < num_of_buckets) {
                    fmt::print("bucket range {}-{}: ", r.first, r.second);
                    frequency_map combined(buckets[r.first]);
                    for (auto idx = r.first + 1; idx <= r.second; idx++) {
                        for (const auto &kv: buckets[idx]) {
                            combined[kv.first] += kv.second;
                        }
                    }
                    print_frequencies(combined, top_n_words);
                }
            }
        }
    } else {
        sycl::queue q{sycl::default_selector_v, dpc_common::exception_handler};
        spdlog::info("Device: {}", q.get_device().get_info<sycl::info::device::name>());

        sycl::buffer<std::string_view> w_buf{nullptr, sycl::range<1>(words.size())};//{words.data(), sycl::range<1>(words.size())};

    }
// how to print a container
//    std::ostream_iterator<std::string> out(std::cout, " ");
//    std::copy(words.begin(), words.end(), out);
//    std::cout << std::endl;

    return 0;
}
