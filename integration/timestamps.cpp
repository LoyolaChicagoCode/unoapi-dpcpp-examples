#include <cstdio>
#include <fmt/format.h>

#include "timestamps.h"

// {{UnoAPI:timestamps-mark-time:begin}}
void mark_time(ts_vector& timestamps, const std::string_view label) {
    timestamps.push_back(std::pair(label.data(), std::chrono::steady_clock::now()));
}
// {{UnoAPI:timestamps-mark-time:end}}

// {{UnoAPI:timestamps-print-timestamps:begin}}
void print_timestamps(const ts_vector& timestamps, const std::string_view filename) {
    using std::chrono::milliseconds;
    using std::chrono::duration_cast;
    auto outfile = filename.empty() ? stdout : std::fopen(filename.data(), "w");
    for (auto t = timestamps.begin() + 1; t != timestamps.end(); t++) {
        const auto dur = duration_cast<milliseconds>(t->second - (t - 1)->second).count();
        fmt::print(outfile, "{}: {} ms\n", t->first, dur);
    }
    const auto& start = timestamps.front().second;
    const auto& stop = timestamps.back().second;
    fmt::print(outfile, "TOTAL time: {} ms\n", duration_cast<milliseconds>(stop - start).count());
    if (! filename.empty())
        std::fclose(outfile);
}
// {{UnoAPI:timestamps-print-timestamps:end}}
