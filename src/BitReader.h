#pragma once


#include <vector>
#include <cstdint>
#include <iosfwd>

class BitReader {
public:
	explicit BitReader(std::ifstream* input);
	bool ReadBit(bool& bit);

private:
	bool FillBuffer();

	std::ifstream* input_;
	std::vector<std::uint8_t> buffer_;
	size_t buffer_pos_;
	size_t buffer_size_;
	std::uint8_t current_byte_;
	int bits_remaining_;
	bool eof_;

	static constexpr size_t BUFFER_SIZE = 16 * 1024;  // 16 kB buffer.
};