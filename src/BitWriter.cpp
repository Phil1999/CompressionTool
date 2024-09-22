#include "BitWriter.h"
#include <fstream>

BitWriter::BitWriter(std::ofstream* output)
	: output_(output), current_byte_(0), bits_filled_(0) {

	buffer_.reserve(BUFFER_SIZE);
}

void BitWriter::WriteBit(bool bit) {
	// Left shift and add new bit
	current_byte_ = (current_byte_ << 1) | static_cast<std::uint8_t>(bit);
	++bits_filled_;

	// Add the new byte to the buffer when we've filled a whole byte.
	if (bits_filled_ == 8) {
		buffer_.push_back(current_byte_);
		current_byte_ = 0;
		bits_filled_ = 0;

		// If the buffer is full, write it to the file.
		if (buffer_.size() >= BUFFER_SIZE) {
			FlushBuffer();
		}
	}
}

void BitWriter::WriteBits(const std::string& bits) {
	// Write multiple bits at once for every char in the input string
	for (char bit_char : bits) {
		WriteBit(bit_char == '1');
	}
}

void BitWriter::Flush() {
	// Check if any remaining bits in current_byte not written to buffer yet
	if (bits_filled_ > 0) {
		// Move remaining bits to most significant positions
		current_byte_ <<= (8 - bits_filled_);
		buffer_.push_back(current_byte_);
		current_byte_ = 0;
		bits_filled_ = 0;
	}

	FlushBuffer();
}

void BitWriter::FlushBuffer() {
	if (!buffer_.empty()) {
		output_->write(reinterpret_cast<const char*>(buffer_.data()), buffer_.size());
		buffer_.clear();
	}
}