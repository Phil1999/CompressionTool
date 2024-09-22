#include "BitReader.h"

#include <fstream>

BitReader::BitReader(std::ifstream* input)
	: input_(input), buffer_pos_(0), buffer_size_(0), bits_remaining_(0), eof_(false) {

	buffer_.resize(BUFFER_SIZE);
	FillBuffer();
}

bool BitReader::FillBuffer() {
	if (eof_) return false;

	input_->read(reinterpret_cast<char*>(buffer_.data()), BUFFER_SIZE);
	buffer_size_ = input_->gcount();
	buffer_pos_ = 0;

	if (buffer_size_ == 0) {
		eof_ = true;
		return false;
	}

	return true;
}

bool BitReader::ReadBit(bool& bit) {
	// Check if all bits are used in the current_byte
	if (bits_remaining_ == 0) {
		if (buffer_pos_ >= buffer_size_) {

			// If we're at end of buffer, try to fill with more data.
			if (!FillBuffer()) {
				return false; // EOF reached
			}
		}
		// Update with new value from buffer if we have data in the buffer
		current_byte_ = buffer_[buffer_pos_++];
		bits_remaining_ = 8;
	}

	// 0x80 = (1000 0000) binary it allows us to check for MSB using a mask.
	bit = (current_byte_ & 0x80) != 0;
	// Left shift MSB
	current_byte_ <<= 1;
	--bits_remaining_;
	return true;
}
