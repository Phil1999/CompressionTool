#include "FileHeader.h"
#include "CompressionExceptions.h"


FileHeader::FileHeader(const std::array<char, MAGIC_NUMBER_SIZE>& magic, const std::string& extension)
    : magic_number_(magic), original_extension_(extension) {}

void FileHeader::write(std::ofstream& output_file) const {
    output_file.write(magic_number_.data(), MAGIC_NUMBER_SIZE);
    output_file.write(reinterpret_cast<const char*>(&version_), VERSION_SIZE);

    auto extension_length = static_cast<uint8_t>(original_extension_.length());
    output_file.write(reinterpret_cast<const char*>(&extension_length), EXTENSION_LENGTH_SIZE);
    output_file.write(original_extension_.data(), extension_length);
}

FileHeader FileHeader::read(std::ifstream& input_file) {
    FileHeader header;

    input_file.read(header.magic_number_.data(), MAGIC_NUMBER_SIZE);
    if (input_file.gcount() != MAGIC_NUMBER_SIZE) {
        throw InvalidHeaderException("Failed to read magic number");
    }

    input_file.read(reinterpret_cast<char*>(&header.version_), VERSION_SIZE);
    if (input_file.gcount() != VERSION_SIZE || header.version_ != VERSION_NUMBER) {
        throw InvalidHeaderException("Unsupported file version");
    }

    uint8_t extension_length;
    input_file.read(reinterpret_cast<char*>(&extension_length), EXTENSION_LENGTH_SIZE);
    if (input_file.gcount() != EXTENSION_LENGTH_SIZE || extension_length == 0) {
        throw InvalidHeaderException("Invalid extension length");
    }

    header.original_extension_.resize(extension_length);
    input_file.read(header.original_extension_.data(), extension_length);
    if (input_file.gcount() != extension_length) {
        throw InvalidHeaderException("Failed to read original file extension");
    }

    return header;

}

bool FileHeader::is_valid_magic_number(const std::string& expected) const {
    return std::string_view(magic_number_.data(), magic_number_.size()) == expected;
}

