#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <iosfwd>

class BitWriter {
public:
	explicit BitWriter(std::ofstream* output);
	void WriteBit(bool bit);
	void WriteBits(const std::string& bits);
	void Flush();

private:
	void FlushBuffer();

	std::ofstream* output_;
	std::vector<std::uint8_t> buffer_;
	std::uint8_t current_byte_;
	int bits_filled_;

	static constexpr size_t BUFFER_SIZE = 16 * 1024;  // 16 kB buffer.
};