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
    
        const int decimal_begin{0};
        const int decimal_end{256};
        const int grain_size{1};
    
        std::vector<std::byte> plaintext;
        std::vector<std::byte> ciphertext;
        std::vector<std::byte> key;
};

TEST_F(CipherTest, TestScramble) {
    EXPECT_EQ(plaintext.size(), ciphertext.size());
    
    const auto plaintext_size = plaintext.size();
    const auto ciphertext_size = ciphertext.size();
    const auto work_load{plaintext_size / grain_size};
    
    std::vector<std::byte> byte_map = scramble(key, decimal_begin, decimal_end);
    std::vector<std::byte> temp_ciphertext{ciphertext_size};
    
    for (int i = 0; i < work_load + 1; i++) {
        substitute<std::vector<std::byte>>(byte_map, plaintext, temp_ciphertext, plaintext_size, grain_size, i);
    }
    
    for (int i = 0; i < ciphertext_size; i++) {
        ASSERT_EQ(temp_ciphertext[i], ciphertext[i]);
    }
    
}

TEST_F(CipherTest, TestUnscramble) {
    EXPECT_EQ(plaintext.size(), ciphertext.size());
    
    const auto ciphertext_size = ciphertext.size();
    const auto plaintext_size = plaintext.size();
    const auto work_load{ciphertext_size / grain_size};
    
    std::vector<std::byte> byte_map = unscramble(key, decimal_begin, decimal_end);
    std::vector<std::byte> temp_plaintext{plaintext_size};
    
    for (int i = 0; i < work_load + 1; i++) {
        substitute<std::vector<std::byte>>(byte_map, ciphertext, temp_plaintext, ciphertext_size, grain_size, i);
    }
    
    for (int i = 0; i < ciphertext.size(); i++) {
        ASSERT_EQ(temp_plaintext[i], plaintext[i]);
    }
    
}