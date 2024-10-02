// BitReader.h
//
// A utility class for reading individual bits from a binary input stream.
// 
// The BitReader class reads bits sequentially from a binary file.
// This is particularly useful in scenarios such as
// parsing compressed data formats, implementing bit-level protocols, or any
// application where bit-wise data manipulation is required. In the context
// of this application, it is for the Huffman Encoding algorithim which operates
// at a bit level.

#pragma once


#include <vector>
#include <cstdint>
#include <iosfwd>

/**
 * @class BitReader
 * @brief Reads individual bits from a binary input stream.
 *
 * The BitReader class allows sequential reading of bits from a binary
 * input stream. It manages an internal buffer to minimize I/O operations
 * and provides methods to read bits one at a time. When the buffer is
 * empty, it refills from the input stream until the end of the file is reached.
 */
class BitReader {
public:
	/**
	* @brief Constructs a BitReader with the given input stream.
	*
	* Initializes the BitReader with an input stream for binary data. This constructor
	* also fills the internal buffer from the stream to prepare for bit reading.
	* 
	* @param input: Pointer to an open std::ifstream object for reading binary data.
	*/
	explicit BitReader(std::ifstream* input);

	/**
	* @brief Reads the next bit from the input stream.
	*
	* Retrieves the next bit from the buffered data. If the buffer is empty,
	* it attempts to fill the buffer from the input stream. If the end of the
	* file is reached, the eof_ flag is set.
	*
	* @param: bit Reference to a bool where the read bit will be stored.
	* @return: true if a bit was successfully read; false if the end of the file
	*         has been reached or an error occurred.
	*/
	bool ReadBit(bool& bit);

private:
	/**
	* @brief Fills the internal buffer with data from the input stream.
	*
	* Attempts to read up to BUFFER_SIZE bytes from the input stream into the
	* buffer. Updates buffer_size_ with the number of bytes read. Resets the
	* buffer position and bit counters. Sets the eof_ flag if no more data is
	* available from the input stream.
	*
	* @return: true if data was successfully read into the buffer; false if
	*         the end of the file was reached or an error occurred.
	*/
	bool FillBuffer();

	std::ifstream* input_;
	std::vector<std::uint8_t> buffer_;				  ///< Internal buffer for storing read bytes.
	size_t buffer_pos_;
	size_t buffer_size_;
	std::uint8_t current_byte_;
	int bits_remaining_;							  ///< Number of bits left in current_byte_.
	bool eof_;										  ///< Flag indicating if end of file has been reached.

	static constexpr size_t BUFFER_SIZE = 16 * 1024;  ///< Size of the internal buffer (16 kB).
};