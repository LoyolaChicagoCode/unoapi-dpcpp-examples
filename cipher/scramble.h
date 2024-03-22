#ifndef SCRAMBLE_H
#define SCRAMBLE_H

std::vector<std::byte> scramble(
    std::string keyword,
    const int character_base,
    const int character_range
);

std::vector<std::byte> unscramble(
    std::string keyword,
    const int character_base,
    const int character_range
);

void remove_duplicates(std::string& this_keyword);

#endif