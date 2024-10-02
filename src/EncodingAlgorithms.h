// EncodingAlgorithms.h
//
// This header file defines classes for implementing compression algorithms such as: 
// Huffman Coding and Run-Length Encoding (RLE). These algorithms are used to 
// compress and decompress data in a lossless manner. Both classes are designed 
// to work with file streams for input and output.
//
// The file also defines a common buffer size and a progress callback type used 
// by both algorithms to report progress during compression or decompression.
//


#pragma once

#include "BitReader.h"
#include "BitWriter.h"
#include <fstream>
#include <memory>
#include <functional>
#include <optional>
#include <unordered_map>
#include <cstdint>
#include <string>
#include <vector>

namespace EncodingAlgorithms {

    // Common buffer size for all compression algorithms.
    constexpr size_t BUFFER_SIZE = 16 * 1024; // 16 kB buffer

    // Callback type for reporting progress during compression and decompression.
    using ProgressCallback = std::function<void(std::int64_t)>;


	/**
	* @class HuffmanCoding
	* @brief Implements Huffman Coding for file compression and decompression.
	*
	* This class provides methods to compress and decompress data using the Huffman Coding
	* algorithm, which assigns variable-length codes to symbols based on their frequencies.
	*/
	class HuffmanCoding {
	public:
		
		/**
		* @brief Compresses the input file using Huffman Coding and writes to the output file.
		*
		* @param input_file: The input file stream containing data to compress.
		* @param output_file: The output file stream to write the compressed data.
		* @param progress_callback: Optional callback to report progress during compression.
		*/
		static void encode(std::ifstream& input_file, std::ofstream& output_file,
			std::optional<ProgressCallback> progress_callback = std::nullopt);

		/**
		* @brief Decompresses the input file using Huffman Coding and writes to the output file.
		*
		* @param input_file: The input file stream containing the compressed data.
		* @param output_file: The output file stream to write the decompressed data.
		* @param progress_callback: Optional callback to report progress during decompression.
		*/
		static void decode(std::ifstream& input_file, std::ofstream& output_file,
			std::optional<ProgressCallback> progress_callback = std::nullopt);


	private:
		// Internal structure representing a node in the Huffman tree.
		struct Node {
			std::uint8_t data;					///< The byte value stored in the leaf node.
			int frequency;						///< Frequency of the byte value in the input data.
			std::shared_ptr<Node> left, right;

			// Constructor for leaf nodes. (actual data)
			Node(std::uint8_t data, int freq) : data(data), frequency(freq), left(nullptr), right(nullptr) {}

			// Constructor for internal nodes. (combinations of lower-freq nodes)
			Node(int freq, std::shared_ptr<Node> l, std::shared_ptr<Node> r)
				// We can take adavantage of move semantics here
				: data(0), frequency(freq), left(std::move(l)), right(std::move(r)) {}

		};

		// Comparator for priority queue to build a min-heap (smallest frequency at the top).
		struct CompareNode {
			bool operator()(const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b) const {
				return a->frequency > b->frequency;
			}
		};


		/**
		 * @brief Builds a frequency table from the input file stream.
		 *
		 * This method reads the input file byte by byte and counts the frequency of
		 * each byte, storing the result in an unordered map. The key is the byte, and
		 * the value is the frequency of that byte in the input data.
		 *
		 * @param input_file: The input file stream containing the data.
		 * @return: An unordered map where each key is a byte, and each value is the frequency of that byte.
		 */
		static std::unordered_map<std::uint8_t, int> BuildFrequencyTable(std::ifstream& input_file);

		/**
		 * @brief Builds a Huffman tree based on the frequency table.
		 *
		 * This method constructs the Huffman tree using a priority queue (min-heap)
		 * where the nodes with the lowest frequency have the highest priority. The
		 * resulting tree can be used to generate the Huffman codes.
		 *
		 * @param freq_table: The frequency table containing byte frequencies.
		 * @return: A shared pointer to the root of the constructed Huffman tree.
		 */
		static std::shared_ptr<Node> BuildHuffmanTree(const std::unordered_map<std::uint8_t, int>& freq_table);

		/**
		 * @brief Builds a decoding tree from the encoding table.
		 *
		 * This method builds a binary tree for decoding Huffman-encoded data. Each
		 * string in the encoding table corresponds to a path in the tree (0 for left,
		 * 1 for right), and the leaf nodes store the decoded byte.
		 *
		 * @param encoding_table: The encoding table where keys are bytes and values are Huffman codes.
		 * @return: A shared pointer to the root of the decoding tree.
		 */
		static std::shared_ptr<Node> BuildDecodingTree(const std::unordered_map<std::uint8_t, std::string>& encoding_table);


		/**
		 * @brief Builds the Huffman encoding table from the Huffman tree.
		 *
		 * This method generates the encoding table by traversing the Huffman tree
		 * recursively. The table maps each byte to its corresponding Huffman code.
		 *
		 * @param node: The current node in the Huffman tree.
		 * @param code: The current Huffman code being built (as a vector of characters).
		 * @param encoding_table: The table where keys are bytes, and values are their Huffman codes.
		 */
		static void BuildEncodingTable(const std::shared_ptr<Node>& node, std::vector<char>& code,
			std::unordered_map<std::uint8_t, std::string>& encoding_table);


		/**
		 * @brief Writes the encoding table to the output file.
		 *
		 * This method writes the size of the encoding table followed by each entry.
		 * For each entry, it writes the byte, the length of the Huffman code, and the
		 * Huffman code itself, using the BitWriter.
		 *
		 * @param encoding_table: The table mapping bytes to their Huffman codes.
		 * @param bit_writer: The BitWriter object used to write the data to the output file.
		 */
		static void WriteEncodingTable(const std::unordered_map<std::uint8_t, std::string>& encoding_table,
			BitWriter& bit_writer);

		/**
		 * @brief Reads the encoding table from the input file.
		 *
		 * This method reads the encoding table from the input file using a BitReader.
		 * It first reads the table size, then each byte and its corresponding Huffman
		 * code. We expect the header metadata to already have been processed.
		 *
		 * @param bit_reader The BitReader object used to read the data from the input file.
		 * @return An unordered map where each key is a byte, and each value is the corresponding Huffman code.
		 */
		static std::unordered_map<std::uint8_t, std::string> ReadEncodingTable(BitReader& bit_reader);


	};

    /**
     * @class RLECoding
     * @brief Implements Run-Length Encoding (RLE) for file compression and decompression.
     */
	class RLECoding {
	public:


		/**
		 * @brief Compresses the input file using Run-Length Encoding (RLE).
		 *
		 * This method reads the input file, compresses repeating bytes using RLE, and
		 * writes the compressed data to the output file.
		 *
		 * @param input_file: The input file stream containing data to compress.
		 * @param output_file: The output file stream to write the compressed data.
		 * @param progress_callback: Optional callback to report progress during compression.
		 */
		static void encode(std::ifstream& input_file, std::ofstream& output_file, std::optional<ProgressCallback> progress_callback = std::nullopt);

		/**
		 * @brief Decompresses the input file using Run-Length Encoding (RLE).
		 *
		 * This method reads the compressed input file, decompresses the data, and
		 * writes it to the output file.
		 *
		 * @param input_file: The input file stream containing the compressed data.
		 * @param output_file: The output file stream to write the decompressed data.
		 * @param progress_callback: Optional callback to report progress during decompression.
		 */
		static void decode(std::ifstream& input_file, std::ofstream& output_file, std::optional<ProgressCallback> progress_callback = std::nullopt);

	private:

		static constexpr std::byte ESCAPE{ 255 };			///< Escape character for 255 byte limit


		/**
		 * @brief Writes a run of repeated bytes to the output file.
		 *
		 * This helper function writes the byte and its repeat count to the output file
		 * using the RLE format. The ESCAPE byte is used to handle long runs (runs of more than 255).
		 *
		 * @param buffer The buffer to store the compressed data before writing.
		 * @param output_file The output file stream to write the compressed data.
		 * @param character The byte being repeated.
		 * @param count The number of times the byte is repeated.
		 */
		static void writeRun(std::vector<std::byte>& buffer, std::ofstream& output_file, std::byte character, std::byte count);

		/**
		 * @brief Flushes the buffer to the output file.
		 *
		 * This helper function writes the content of the buffer to the output file.
		 *
		 * @param buffer: The buffer containing compressed data to flush.
		 * @param output_file: The output file stream to write the data.
		 */
		static void flushBuffer(const std::vector<std::byte>& buffer, std::ofstream& output_file);
	};

}