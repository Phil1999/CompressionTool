#pragma once

#include <array>
#include <string>
#include <fstream>

class FileHeader {
public:
    static constexpr size_t MAGIC_NUMBER_SIZE = 3;
    static constexpr size_t VERSION_SIZE = 1;
    static constexpr size_t EXTENSION_LENGTH_SIZE = 1;
    static constexpr size_t VERSION_NUMBER = 1;

    FileHeader() = default;
    FileHeader(const std::array<char, MAGIC_NUMBER_SIZE>& magic, const std::string& extension);

    void write(std::ofstream& output_file) const;
    static FileHeader read(std::ifstream& input_file);

    bool is_valid_magic_number(const std::array<char, MAGIC_NUMBER_SIZE>& expected) const;

    std::array<char, MAGIC_NUMBER_SIZE> magic_number_;
    uint8_t version_ = VERSION_NUMBER;
    std::string original_extension_;

};