// including for main
#include <sycl/sycl.hpp>
#include <iostream>
#include <dpc_common.hpp>

// including for scramble
#include <iostream>
#include <vector>
#include <list>


/*
 * TODO
 * + add function to generate a msg:string given an input length
 * + add functionality to take in user input for key and msg length from cmd line
 * + refactor variable naming convention
 * + add logging and time
 * + use name space
 * + read msg from file.txt or std input
 * + support command line options and arguments
 * + implement decrypt
 * + choice of running sequential or parall_for
 *
 * */




// consts
const std::string msg{"abcdefgh"};
const auto size = msg.size();

const std::string key{"dogs"};
const auto key_size = key.size();
const auto ascii_a{97};
const auto range{26};



std::vector<char> scramble(const std::string& key);

/*
 *
function scramble
-----------------
Assumptions- The set of characters used in the constant key string and plaintext alphabet are [a-z].

Input- A reference to the constant string key.

Output- A vector (v1) representing the ciphertext alphabet.

Cipher algorithm- The algorithm is based on mixed alphabet substitution. The set of the input key
determines the first sequence of characters in the ciphertext alphabet. The characters not included
in the key set and are in the alphabet set are then appended to the back of the ciphertext alphabet.

example
-------
key = "dogs"

index: 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25  (plaintext alphabet)
v1:   [d o g s a b c e f h  i  j  k  l  m  n  p  q  r  t  u  v  w  x  y  z] (ciphertext alphabet)

*/

std::vector<char> scramble(const std::string& key)
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
    // check time complexity of removing from front of list, O(1) I think?
    for (int i = 0; i < (range-key_size); i++) {
        char element = alphabet.front();
        v1.push_back(element);
        alphabet.pop_front();
    }     
    return v1;
}



int main()
{
    std::vector<char> v1 = scramble(key);
    
    sycl::queue Q;
    sycl::buffer<char> msg_buf{msg.data(), sycl::range<1>{size}};
    sycl::buffer<char> cipher_alphabet_buf{v1.data(), sycl::range<1>{range}};
    sycl::buffer<char> result_buf{sycl::range<1>{size}};
    
    /* creating ciphertext */
    Q.submit([&](auto & h) {
        const sycl::accessor cipher_alphabet{cipher_alphabet_buf, h};
        const sycl::accessor msg{msg_buf, h};
        const sycl::accessor result{result_buf, h};
        h.parallel_for(size, [=](const auto & i) {
            result[i] = cipher_alphabet[msg[i] - ascii_a];
        });
    });
    
    const sycl::host_accessor result{result_buf};
    const sycl::host_accessor cipher_alphabet{cipher_alphabet_buf};
    
    /* printing plaintext */
    std::cout << "plaintext = " << msg;
    std::cout << std::endl;
    
    /* printing keys */
    std::cout << "key = " << key;
    std::cout << std::endl;

    /* printing ciphertext */
    std::cout << "ciphertext = ";
    for (size_t i = 0; i < result.size(); i++) {
        std::cout << result[i];
    }
    std::cout << std::endl;

    return 0;
}
