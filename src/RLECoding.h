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
	static constexpr std::byte ESCAPE{255};

	static void writeRun(std::vector<std::byte>& buffer, std::ofstream& output_file, std::byte character, std::byte count);
	static void flushBuffer(const std::vector<std::byte>& buffer, std::ofstream& output_file);
};



