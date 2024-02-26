#include <algorithm>
#include <list>
#include <vector>
#include <set>

#include "scramble.h"

// method returns a vector used to transform plaintext -> ciphertext
std::vector<char> scramble(std::string keyword, const int character_base, const int character_range)
{
    // setting up keyword domain
    remove_duplicates(keyword);
    const auto keyword_size = keyword.size();

    // declaring plaintext and ciphertext alphabets
    std::list<char> plaintext_alphabet;
    std::vector<char> ciphertext_alphabet;
    
    // initializing plaintext_alphabet
    for (int i = character_base; i < (character_range + character_base); i++) {
        char character = char(i);
        plaintext_alphabet.push_back(character);
    }
    
    // writing keyword characters to ciphertext_alphabet and removing keyword characters from plaintext_alphabet
    for (int i = 0; i < keyword_size; i++) {
        ciphertext_alphabet.push_back(keyword[i]);
        plaintext_alphabet.remove(keyword[i]);
    }
    
    // writing remaining characters from plaintext_alphabet to ciphertext_alphabet
    for (int i = 0; i < (character_range - keyword_size); i++) {
        char character = plaintext_alphabet.front();
        ciphertext_alphabet.push_back(character);
        plaintext_alphabet.pop_front();
    }     
    return ciphertext_alphabet;
}

// method returns a vector used to transform ciphertext -> plaintext
std::vector<char> unscramble(std::string keyword, const int character_base, const int character_range)
{
    remove_duplicates(keyword);
    const auto keyword_size = keyword.size();
    auto counter_key = 0;
    auto counter_not = keyword_size;
    char character;
    std::vector<char> plaintext_alphabet;

    for (int i = character_base; i < (character_range + character_base); i++) {
        char index_char = char(i);
        if (keyword.find(index_char) == -1) {
            character = char(counter_not++);
            plaintext_alphabet.push_back(character);
        }
        else {
            character = char(counter_key++);
            plaintext_alphabet.push_back(character);
        }
    }
    return plaintext_alphabet;
}

// helper method to remove duplicates from a string
void remove_duplicates(std::string& keyword)
{
    std::set<char> unique_characters;
    keyword.erase(
        std::remove_if(
            keyword.begin(),
            keyword.end(),
            [&unique_characters] (char i) {
                if (unique_characters.count(i)) { return true; }
                unique_characters.insert(i);
                return false;
            }
        ),
        keyword.end()
    );
}