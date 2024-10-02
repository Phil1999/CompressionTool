#include <gtest/gtest.h>
#include "../src/EncodingAlgorithms.h"
#include <fstream>
#include <string>
#include <filesystem>
#include <chrono>
#include <random>
#include <iostream>

/// Not checking for empty file because in our main application, empty files
/// are checked in CompressionTool and not within the encoding classes themselves.

class CompressionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a unique temporary directory for each test
        temp_dir_ = std::filesystem::temp_directory_path() / ("compression_test_" + std::to_string(std::random_device{}()));
        std::filesystem::create_directory(temp_dir_);
    }

    void TearDown() override {
        // Safely remove the temporary directory and its contents
        if (std::filesystem::exists(temp_dir_)) {
            std::filesystem::remove_all(temp_dir_);
        }
    }

    std::string createInputFile(const std::string& content) {
        std::string filename = (temp_dir_ / "input.txt").string();
        std::ofstream file(filename, std::ios::binary);
        file << content;
        return filename;
    }

    std::string readOutputFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }

    std::string generateRandomString(size_t length) {
        const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, charset.length() - 1);
        std::string result(length, 0);
        std::generate_n(result.begin(), length, [&]() { return charset[dis(gen)]; });
        return result;
    }

    std::filesystem::path temp_dir_;
};

// RLE Tests

TEST_F(CompressionTest, RLEBasicCompression) {
    std::string input = "AABBBCCCC";
    std::string input_file = createInputFile(input);
    std::string output_file = (temp_dir_ / "output.rle").string();
    std::string decompressed_file = (temp_dir_ / "decompressed.txt").string();

    std::ifstream input_stream(input_file, std::ios::binary);
    std::ofstream output_stream(output_file, std::ios::binary);
    EncodingAlgorithms::RLECoding::encode(input_stream, output_stream);
    output_stream.close();

    std::ifstream compressed_stream(output_file, std::ios::binary);
    std::ofstream decompressed_stream(decompressed_file, std::ios::binary);
    EncodingAlgorithms::RLECoding::decode(compressed_stream, decompressed_stream);
    decompressed_stream.close();

    EXPECT_EQ(input, readOutputFile(decompressed_file));
}

// Runtime test for RLE
TEST_F(CompressionTest, RLERuntimeTest) {
    std::string input = generateRandomString(1000000); // 1MB of random data
    std::string input_file = createInputFile(input);
    std::string output_file = (temp_dir_ / "output.rle").string();
    std::string decompressed_file = (temp_dir_ / "decompressed.txt").string();

    auto start = std::chrono::high_resolution_clock::now();

    std::ifstream input_stream(input_file, std::ios::binary);
    std::ofstream output_stream(output_file, std::ios::binary);
    EncodingAlgorithms::RLECoding::encode(input_stream, output_stream);
    output_stream.close();

    std::ifstream compressed_stream(output_file, std::ios::binary);
    std::ofstream decompressed_stream(decompressed_file, std::ios::binary);
    EncodingAlgorithms::RLECoding::decode(compressed_stream, decompressed_stream);
    decompressed_stream.close();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;

    std::cout << "RLE compression and decompression of 1MB took " << diff.count() << " seconds" << std::endl;

    EXPECT_EQ(input, readOutputFile(decompressed_file));
}

// Huffman Tests
TEST_F(CompressionTest, HuffmanBasicCompression) {
    std::string input = "this is an example for huffman encoding";
    std::string input_file = createInputFile(input);
    std::string output_file = (temp_dir_ / "output.huff").string();
    std::string decompressed_file = (temp_dir_ / "decompressed.txt").string();

    std::ifstream input_stream(input_file, std::ios::binary);
    std::ofstream output_stream(output_file, std::ios::binary);
    EncodingAlgorithms::HuffmanCoding::encode(input_stream, output_stream);
    output_stream.close();

    std::ifstream compressed_stream(output_file, std::ios::binary);
    std::ofstream decompressed_stream(decompressed_file, std::ios::binary);
    EncodingAlgorithms::HuffmanCoding::decode(compressed_stream, decompressed_stream);
    decompressed_stream.close();

    EXPECT_EQ(input, readOutputFile(decompressed_file));
}

// Runtime test for Huffman
TEST_F(CompressionTest, HuffmanRuntimeTest) {
    std::string input = generateRandomString(1000000); // 1MB of random data
    std::string input_file = createInputFile(input);
    std::string output_file = (temp_dir_ / "output.huff").string();
    std::string decompressed_file = (temp_dir_ / "decompressed.txt").string();

    auto start = std::chrono::high_resolution_clock::now();

    std::ifstream input_stream(input_file, std::ios::binary);
    std::ofstream output_stream(output_file, std::ios::binary);
    EncodingAlgorithms::HuffmanCoding::encode(input_stream, output_stream);
    output_stream.close();

    std::ifstream compressed_stream(output_file, std::ios::binary);
    std::ofstream decompressed_stream(decompressed_file, std::ios::binary);
    EncodingAlgorithms::HuffmanCoding::decode(compressed_stream, decompressed_stream);
    decompressed_stream.close();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;

    std::cout << "Huffman compression and decompression of 1MB took " << diff.count() << " seconds" << std::endl;

    EXPECT_EQ(input, readOutputFile(decompressed_file));
}