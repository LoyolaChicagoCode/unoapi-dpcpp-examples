//
// Created by George K. Thiruvathukal on 3/19/23.
//


// Use modern C++ to randomly generate a million floating point values in the range 0 to 1 and bucket them in 0.1 increments?

#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>

int main() {
    std::vector<int> buckets(10, 0); // initialize all buckets to 0

    // set up random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 1);

    // generate a million random numbers and bucket them
    for (int i = 0; i < 1000000; ++i) {
        double value = dis(gen); // generate a random number between 0 and 1
        int bucket = std::floor(value * 10); // calculate the bucket index
        buckets[bucket]++; // increment the count in the corresponding bucket
    }

    // print the bucket counts
    for (int i = 0; i < 10; ++i) {
        std::cout << i << " - " << (i + 1) / 10.0 << ": " << buckets[i] << '\n';
    }

    return 0;
}
