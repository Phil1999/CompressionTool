// FileHeader.h
//
// FileHeader is responsible for handling file metadata, such as the magic number,
// file version, and original file extension. This class allows writing and reading
// metadata to/from compressed files to ensure the correct algorithm and file format
// are used for decompression. It also includes validation checks to detect
// invalid or corrupted file headers.


#pragma once

#include <array>
#include <string>
#include <fstream>


/**
* @class FileHeader
* @brief Handles metadata for compressed files.
*
* The FileHeader class is responsible for writing and reading file metadata,
* including the magic number (to identify the compression algorithm),
* file version, and the original file extension. This information is used during
* compression and decompression to ensure that the correct format is applied and
* we are able to decode into the original extension.
*/
class FileHeader {
public:
    // Constants defining the size of different parts of the file header.
    static constexpr size_t MAGIC_NUMBER_SIZE = 3;        ///< Size of the magic number field (3 bytes).
    static constexpr size_t VERSION_SIZE = 1;             ///< Size of the version field (1 byte).
    static constexpr size_t EXTENSION_LENGTH_SIZE = 1;    ///< Size of the extension length field (1 byte).
    static constexpr size_t VERSION_NUMBER = 1;           ///< Current version number of the file format.

    /**
    * @brief Default constructor for FileHeader.
    *
    * Initializes an empty FileHeader object.
    */
    FileHeader() = default;

    /**
    * @brief Constructs a FileHeader with the given magic number and file extension.
    *
    * @param magic: A 3-byte magic number used to identify the compression algorithm.
    * @param extension: The original file extension before compression.
    */
    FileHeader(const std::array<char, MAGIC_NUMBER_SIZE>& magic, const std::string& extension);

    /**
    * @brief Writes the file header to the output stream.
    *
    * This method writes the magic number, version, and original file extension
    * to the output file. It is used during compression to store the file's metadata.
    *
    * @param output_file: The output stream where the header will be written.
    */
    void write(std::ofstream& output_file) const;

    /**
    * @brief Reads the file header from the input stream.
    *
    * This method reads the magic number, version, and original file extension
    * from the input file. It validates the correctness of the file header and
    * throws exceptions if any part of the header is invalid or corrupted.
    *
    * @param input_file: The input stream from which the header will be read.
    * @return: A FileHeader object containing the read metadata.
    * @throws: InvalidHeaderException if the header is invalid or corrupted.
    */
    static FileHeader read(std::ifstream& input_file);

    /**
    * @brief Validates the magic number against an expected value.
    *
    * This method checks if the magic number stored in the header matches
    * the expected magic number, which identifies the compression algorithm used.
    *
    * @param expected: The expected magic number for the file.
    * @return: true if the magic number matches the expected value, false otherwise.
    */
    bool is_valid_magic_number(const std::array<char, MAGIC_NUMBER_SIZE>& expected) const;

    // Public member variables containing the file metadata.
    std::array<char, MAGIC_NUMBER_SIZE> magic_number_;      ///< Magic number identifying the compression algorithm.
    uint8_t version_ = VERSION_NUMBER;                      ///< File format version.
    std::string original_extension_;                        ///< The original file extension before compression.

};