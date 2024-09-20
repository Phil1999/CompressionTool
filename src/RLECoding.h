#pragma once

#include "FileHeader.h"
#include <string>
#include <vector>

class RLECoding {
public:
		static void encode(std::ifstream& input_file, std::ofstream& output_file);
		static void decode(std::ifstream& input_file, std::ofstream& output_file);

private:
	static constexpr size_t BUFFER_SIZE = 8192; // 8 kB buffer
	static constexpr unsigned char ESCAPE = 255;

	static void writeRun(std::vector<unsigned char>& buffer, std::ofstream& output_file, unsigned char character, unsigned char count);
	static void flushBuffer(const std::vector<unsigned char>& buffer, std::ofstream& output_file);
};



