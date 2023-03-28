//
// Created by George K. Thiruvathukal on 3/19/23.
//

#include "range1.h"

#include <iostream>
#include <vector>

// Write a loop in modern C++ to count the integers up to 100000.
// This is stateful: Initializes vector and the performs the sum over the range.

int main() {
    std::vector<int> range(100000);
    for (int i = 0; i < 100000; ++i) {
        range[i] = i + 1;
    }

    for (int i : range) {
        std::cout << i << '\n';
    }

    return 0;
}
