#include "scramble.h"

// function scramble
// -----------------
// inputs: A key used to scramble the byte_map, 
//         the beginning decimal used to represent the "smallest" byte,
//         and the last decimal used to represent the "largest" byte. 
//
// output: Returns a vector "byte map" that may be used to transform plaintext to ciphertext.
//
// IE:
// key = {std::byte{67}, std::byte{121}, std::byte{110}, std::byte{64}, std::byte{33}}
//
// std::vector<std::byte> byte_map =
// [0  1   2   3  4  5 6 7 8 9...  ...255] indices :: static_cast<std::byte>(index)
// [67 121 110 64 33 0 1 2...      ...255] elements :: <std::byte{element}>
//
// The bytes represented by the indices correspond to plaintext.
// The bytes represented by the elements correspond to ciphertext.
//
std::vector<std::byte> scramble(std::vector<std::byte> key, const int decimal_begin, const int decimal_end)
{
    // Function call to remove duplicate bytes in the key.
    remove_duplicates(key);
    
    // Declarations
    std::list<std::byte> byte_list;
    std::vector<std::byte> byte_map;
    std::byte byte;
    
    // Initializing the key_size for iterating over the key object.
    const auto key_size = key.size();
    
    // Step 1: Initializing the byte_list with bytes ranging from decimal_begin to decimal_end.
    for (int i = decimal_begin; i < (decimal_end + decimal_begin); i++) {
        byte_list.push_back(static_cast<std::byte>(i));
    }
    
    // Step 2: Writing the bytes contained in object key to the byte_map and removing key bytes from the byte_list.
    for (int i = 0; i < key_size; i++) {
        byte_map.push_back(static_cast<std::byte>(key[i]));
        byte_list.remove(static_cast<std::byte>(key[i]));
    }
    
    // Step 3: Writing the remaining bytes from the byte_list to the byte_map.
    for (int i = decimal_begin; i < (decimal_begin + decimal_end - key_size); i++) {
        byte = byte_list.front();
        byte_map.push_back(byte);
        byte_list.pop_front();
    }
    // All done.
    return byte_map;
}

// function unscramble
// -------------------
// inputs: A key used to unscramble the byte_map,
//         the beginning decimal used to represnt the "smallest" byte,
//         and the last decimal used to represent the "largest" byte.
//
// output: Returns a vector "byte map" that may be used to transform ciphertext to plaintext.
//
// IE:
// key = {std::byte{67}, std::byte{121}, std::byte{110}, std::byte{64}, std::byte{33}}
//
// std::vector<std::byte> byte_map =
// 0 1 2.. ..33.. ..255 indices :: static_cast<std::byte>(index)
// 5 6 7.. ..4..  ..255 elements :: <std::byte>
//
// The bytes represented by the indices correspond to ciphertext.
// The bytes represented by the elements correspond to plaintext.
//
std::vector<std::byte> unscramble(std::vector<std::byte> key, const int decimal_begin, const int decimal_end)
{
    // Function call to remove duplicate bytes in the key.
    remove_duplicates(key);
    
    // Declarations
    std::vector<std::byte> byte_map;
    std::byte index_byte;
    std::byte byte;
    
    // Initializing counter_not to the size of the object key; this is the starting decimal value for bytes that are NOT mapped to by the key.
    auto counter_not = key.size();
    
    // Populating byte_map
    for (int i = decimal_begin; i < (decimal_end + decimal_begin); i++) {
        index_byte = static_cast<std::byte>(i);
        // Step 1: Find if the current_byte is contained in the object key.
        auto found = std::find_if(key.begin(), key.end(), [&index_byte](std::byte b) {
            return b == index_byte;
        });
        // Step 2: If the index_byte is not contained in the object key, then map to the byte value of decimal counter_not and increment counter_not.
        if (found == key.end()) {
            byte = static_cast<std::byte>(counter_not++);
            byte_map.push_back(byte);
        }
        // Step 3: Else the index_byte is contained in the object key, then map to the byte value of the index containing the element index_byte.
        else {
            byte = static_cast<std::byte>(std::distance(key.begin(), found)); // finding the index of the iterator found
            byte_map.push_back(byte);
        }
    }
    // All done.
    return byte_map;
}

// function remove_duplicates
// --------------------------
// input: a reference to the object key
// output: void
// description: This function removes all duplicate bytes from the provided argument.
void remove_duplicates(std::vector<std::byte> & key)
{
    std::set<std::byte> unique_bytes;
    key.erase(
        std::remove_if(
            key.begin(), key.end(), [&unique_bytes] (std::byte b) {
                if (unique_bytes.count(b) > 0) { return true; }
                unique_bytes.insert(b);
                return false;
            }
        ),
        key.end()
    );
}