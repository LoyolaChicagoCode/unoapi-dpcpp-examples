//
// Created by George K. Thiruvathukal on 3/19/23.
//

#include <iostream>
#include <vector>

// This is kind of wasteful...

int main() {
    for (int i : std::vector<int>(100000)) {
        std::cout << i + 1 << '\n';
    }
    return 0;
}
