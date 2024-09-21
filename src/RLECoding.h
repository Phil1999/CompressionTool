#pragma once

#include "FileHeader.h"
#include <string>
#include <vector>
#include <functional>
#include <cstdint>
#include <optional>

class RLECoding {
public:

	using ProgressCallback = std::function<void(std::int64_t)>;

	static void encode(std::ifstream& input_file, std::ofstream& output_file, std::optional<ProgressCallback> progress_callback = std::nullopt);
	static void decode(std::ifstream& input_file, std::ofstream& output_file, std::optional<ProgressCallback> progress_callback = std::nullopt);

private:
	static constexpr size_t BUFFER_SIZE = 16 * 1024; // 16 kB buffer
	static constexpr std::byte ESCAPE{255};

	static void writeRun(std::vector<std::byte>& buffer, std::ofstream& output_file, std::byte character, std::byte count);
	static void flushBuffer(const std::vector<std::byte>& buffer, std::ofstream& output_file);
};



