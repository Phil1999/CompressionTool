// BitWriter.h
//
// A utility class for writing individual bits to a binary output stream.
//
// The BitWriter class writes bits sequentially to a binary file.
// It buffers data to improve write efficiency and handles
// partial bytes by padding them with 0s


#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <iosfwd>


/**
 * @class BitWriter
 * @brief Writes individual bits to a binary output stream.
 *
 * The BitWriter class allows sequential writing of bits to a binary output
 * stream. It buffers data in chunks to minimize I/O operations and handles
 * partial bytes, ensuring that any remaining bits are properly written when
 * flushed. The class supports writing individual bits as well as multiple bits
 * from a string representation.
 */
class BitWriter {
public:
	/**
	* @brief Constructs a BitWriter with the given output stream.
	*
	* Initializes the BitWriter with an output stream for binary data.
	* The constructor prepares the internal buffer for writing and sets the
	* internal state to handle bit-level operations.
	*
	* @param output: Pointer to an open std::ofstream object for writing binary data.
	*               The BitWriter does not take ownership of the stream.
	*/
	explicit BitWriter(std::ofstream* output);

	/**
	* @brief Writes a single bit to the output stream.
	*
	* Adds a bit to the current byte being assembled. Once a full byte has been
	* accumulated, it is stored in the buffer. If the buffer reaches capacity,
	* it is flushed to the output stream.
	*
	* @param bit: The bit to write (true for 1, false for 0).
	*/
	void WriteBit(bool bit);

	/**
	* @brief Writes multiple bits from a string to the output stream.
	*
	* Writes each character from the string as a bit. Characters '1' are
	* interpreted as 1, and characters '0' are interpreted as 0.
	*
	* @param bits: A string where each character represents a bit ('1' or '0').
	*/
	void WriteBits(const std::string& bits);

	/**
	* @brief Flushes any remaining bits and data to the output stream.
	*
	* Ensures that any partial byte is written to the buffer by padding the
	* remaining bits with zeroes. After this, it writes the entire buffer to
	* the output stream and clears the buffer.
	*/
	void Flush();

private:

	/**
	* @brief Writes the internal buffer to the output stream.
	*
	* If the buffer contains data, it is written to the output stream, and the
	* buffer is cleared. This function is used when the buffer reaches capacity
	* or when flushing remaining data.
	*/
	void FlushBuffer();

	std::ofstream* output_;
	std::vector<std::uint8_t> buffer_;				  ///< Internal buffer for storing bytes before writing.
	std::uint8_t current_byte_;
	int bits_filled_;								  ///< Number of bits filled in current_byte_.

	static constexpr size_t BUFFER_SIZE = 16 * 1024;  ///< Size of the internal buffer (16 kB).
};