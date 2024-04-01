#ifndef SCRAMBLE_H
#define SCRAMBLE_H

#include <algorithm>
#include <list>
#include <vector>
#include <set>

// note: Argument key is passed by value this way the key object in main is unaffected
// by function calls to remove_duplicates from within scramble and unscramble.
std::vector<std::byte> scramble(
    std::vector<std::byte> key,
    const int decimal_begin,
    const int decimal_end
);

std::vector<std::byte> unscramble(
    std::vector<std::byte> key,
    const int decimal_begin,
    const int decimal_end
);

// note: Argument key is passed by reference here.
void remove_duplicates(
    std::vector<std::byte> & key
);

#endif