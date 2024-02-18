#include "scramble.h"

#include <list>
#include <vector>

std::vector<char> scramble(const std::string& key, const int key_size, const std::string& msg, const int msg_size, const int ascii_a, const int range)

{
    // TODO: I need to check for duplicate characters and remove them from key.
   
    // declarations
    std::list<char> alphabet;
    std::vector<char> v1;
    
    // initializing alphabet
    auto ascii_value = ascii_a;
    for (int i = 0; i < range; i++) {
        char element = char(ascii_value);
        alphabet.push_back(element);
        ascii_value++;
    }
    
    // writing out keyword
    for (int i = 0; i < key_size; i++) {
        v1.push_back(key[i]);
        alphabet.remove(key[i]);
    }
    
    // writing out remaining characters
    for (int i = 0; i < (range-key_size); i++) {
        char element = alphabet.front();
        v1.push_back(element);
        alphabet.pop_front();
    }     
    return v1;
}
