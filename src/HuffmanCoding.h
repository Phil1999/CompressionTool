#pragma once

#include "BitReader.h"
#include "BitWriter.h"
#include <memory>
#include <functional>
#include <iosfwd> // For forward declaration of ifstream and ofstream.
#include <optional>
#include <unordered_map>
#include <cstdint>
#include <string>
#include <vector>


class HuffmanCoding {
public:
	// Setup callback for reporting progress.
	using ProgressCallback = std::function<void(std::int64_t)>;

	static void encode(std::ifstream& input_file, std::ofstream& output_file,
		std::optional<ProgressCallback> progress_callback = std::nullopt);
	static void decode(std::ifstream& input_file, std::ofstream& output_file,
		std::optional<ProgressCallback> progress_callback = std::nullopt);


private:
	struct Node {
		std::uint8_t data;
		int frequency;
		std::shared_ptr<Node> left, right;

		// Constructor for leaf nodes. (actual data)
		Node(std::uint8_t data, int freq) : data(data), frequency(freq), left(nullptr), right(nullptr) {}
		// Constructor for internal nodes. (combinations of lower-freq nodes)
		Node(int freq, std::shared_ptr<Node> l, std::shared_ptr<Node> r) 
			// We can take adavantage of move semantics here
            : data(0), frequency(freq), left(std::move(l)), right(std::move(r)) {}

	};

	struct CompareNode {
		bool operator()(const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b) const {
			return a->frequency > b->frequency;
		}
	};



	static std::unordered_map<std::uint8_t, int> BuildFrequencyTable(std::ifstream& input_file);
	static std::shared_ptr<Node> BuildHuffmanTree(const std::unordered_map<std::uint8_t, int>& freq_table);
	static std::shared_ptr<Node> BuildDecodingTree(const std::unordered_map<std::uint8_t, std::string>& encoding_table);

	static void BuildEncodingTable(const std::shared_ptr<Node>& node, std::vector<char>& code,
		std::unordered_map<std::uint8_t, std::string>& encoding_table);

	static void WriteEncodingTable(const std::unordered_map<std::uint8_t, std::string>& encoding_table,
		BitWriter& bit_writer);

	static std::unordered_map<std::uint8_t, std::string> ReadEncodingTable(BitReader& bit_reader);

	static constexpr size_t BUFFER_SIZE = 16 * 1024; // 16 kB buffer.


};