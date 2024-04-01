#include <spdlog/spdlog.h>
#include <gtest/gtest.h>

#include <sycl/sycl.hpp>
#include <dpc_common.hpp>

#include "scramble.h"
#include "file_ops.h"
#include "substitute.h"

class CipherTest : public testing::Test {
    public:
        void SetUp() override {
            spdlog::set_level(spdlog::level::off);
            spdlog::info("spdlog level set to {}", spdlog::get_level());
            read_from_binary(plaintext, plaintext_filepath);
            read_from_binary(ciphertext, ciphertext_filepath);
            read_from_binary(key, key_filepath);
        }
    
        void TearDown() override {
            plaintext.clear();
            ciphertext.clear();
            key.clear();
        }
    
    protected:
        const std::string plaintext_filepath = "<path to /test-data/plaintext.txt>";
        const std::string ciphertext_filepath = "<path to /test-data/ciphertext.txt>";
        const std::string key_filepath = "<path to /test-data/key.txt>";
    
        const int decimal_base{0};
        const int decimal_range{256};
    
        std::vector<std::byte> plaintext;
        std::vector<std::byte> ciphertext;
        std::vector<std::byte> key;
};

TEST_F(CipherTest, TestScramble) {
    EXPECT_EQ(plaintext.size(), ciphertext.size());
    std::vector<std::byte> alphabet = scramble(key, decimal_base, decimal_range);
    for (int i = 0; i < plaintext.size(); i++) {
        EXPECT_EQ(substitute(alphabet, plaintext, decimal_base, i), ciphertext[i]);
    }
}

TEST_F(CipherTest, TestUnscramble) {
    EXPECT_EQ(plaintext.size(), ciphertext.size());
    std::vector<std::byte> alphabet = unscramble(key, decimal_base, decimal_range);
    for (int i = 0; i < ciphertext.size(); i++) {
        EXPECT_EQ(substitute(alphabet, ciphertext, decimal_base, i), plaintext[i]);
    }
}