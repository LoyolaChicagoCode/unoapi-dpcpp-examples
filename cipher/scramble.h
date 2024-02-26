#ifndef SCRAMBLE_H
#define SCRAMBLE_H

std::vector<char> scramble(
    std::string keyword,
    const int character_base,
    const int character_range
);

std::vector<char> unscramble(
    std::string keyword,
    const int character_base,
    const int character_range
);

void remove_duplicates(std::string& this_keyword);

#endif