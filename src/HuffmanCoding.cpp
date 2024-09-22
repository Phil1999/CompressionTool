#include "HuffmanCoding.h"
#include <fstream>
#include <queue>
#include <iostream>
#include <algorithm>
#include <bitset>


std::unordered_map<std::uint8_t, int> HuffmanCoding::BuildFrequencyTable(std::ifstream& input_file) {
	std::unordered_map<std::uint8_t, int> freq_table;
	std::vector<std::uint8_t> buffer(BUFFER_SIZE);

	// Count the frequency of each byte and store it in our map.
	
	while (input_file) {
		input_file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());

		size_t bytes_read = input_file.gcount();

		for (size_t i = 0; i < bytes_read; ++i) {
			++freq_table[buffer[i]];
		}
	}

	return freq_table;
}

std::shared_ptr<HuffmanCoding::Node> HuffmanCoding::BuildHuffmanTree(
	const std::unordered_map<std::uint8_t, int>& freq_table) {

	// Since C++ priority_queue is by default a max-heap we define our custom comparator that reverses the default behavior
	// which is the CompareNode struct. Now we guarantee that the element with lowest priority is at the top.
	std::priority_queue<std::shared_ptr<Node>, std::vector<std::shared_ptr<Node>>, CompareNode> min_heap;

	// Add nodes to a priority queue
	for (const auto& [byte, freq] : freq_table) {
		min_heap.push(std::make_shared<Node>(byte, freq));
	}

	// Organize the priority queue so node with lowest frequency has highest priority (min-heap based on frequency).
	// Building the tree:
	// 1. While more than one node in queue: Remove the two nodes with lowest frequencies from queue.
	// 2. Create a new internal node with these two nodes as children.
	// 3. Set the frequency of this new node to the sum of its children's frequencies
	// 4. Add this new node back into the queue.

	while (min_heap.size() > 1) {
		// Store lowest freq nodes and then remove it from the min-heap.
		auto left = min_heap.top(); min_heap.pop();
		auto right = min_heap.top(); min_heap.pop();

		// Frequency of new node should be the sum of its children's frequencies.
		auto parent = std::make_shared<Node>(left->frequency + right->frequency, left, right);
		
		// Add the new node back into the queue.
		min_heap.push(parent);
	}

	// 5. When only one nod remains in the queue, it becomes the root of the Huffman Tree.
	return min_heap.top();

}


void HuffmanCoding::BuildEncodingTable(const std::shared_ptr<Node>& root, std::vector<char>& code,
	std::unordered_map<std::uint8_t, std::string>& encoding_table) {

	if (!root) return;

	// When we hit a leaf node, store the current code for that
	// byte in the encoding table.
	if (!root->left && !root->right) {
		encoding_table[root->data] = std::string(code.begin(), code.end());
		return;
	}

	// Build up encoding table by traversing tree recursively.
	// 1. When moving left, append '0' to current code.
	// 1. When moving right, append '1' to current code.
	code.push_back('0');
	BuildEncodingTable(root->left, code, encoding_table);
	code.back() = '1';
	BuildEncodingTable(root->right, code, encoding_table);
	code.pop_back();


}

std::shared_ptr<HuffmanCoding::Node> HuffmanCoding::BuildDecodingTree(
	const std::unordered_map<std::uint8_t, std::string>& encoding_table) {

	auto root = std::make_shared<Node>(0, nullptr, nullptr);

	for (const auto& [byte, code] : encoding_table) {
		auto node = root;

		for (char bit : code) {
			if (bit == '0') {
				if (!node->left) {
					node->left = std::make_shared<Node>(0, nullptr, nullptr);
				}
				node = node->left;
			}
			else {
				if (!node->right) {
					node->right = std::make_shared<Node>(0, nullptr, nullptr);
				}
				node = node->right;
			}
		}
		node->data = byte;
	}
	return root;
}

void HuffmanCoding::WriteEncodingTable(const std::unordered_map<std::uint8_t, std::string>& encoding_table,
	BitWriter& bit_writer) {

	auto table_size = static_cast<uint16_t>(encoding_table.size());
	
	// Write table size as 16 bits directly
	for (int i = 15; i >= 0; --i) {
		bit_writer.WriteBit((table_size >> i) & 1);
	}

	// For every entry in the encoding table:
	// 1. Write the byte
	// 2. Write the length of the Huffman code for this byte.

	for (const auto& [byte, code] : encoding_table) {
		// Write byte directly as 8 bits
		for (int i = 7; i >= 0; --i) {
			bit_writer.WriteBit((byte >> i) & 1);
		}

		auto code_length = static_cast<uint8_t>(code.length());
		// Write code length directly as 8 bits
		for (int i = 7; i >= 0; --i) {
			bit_writer.WriteBit((code_length >> i) & 1);
		}

		bit_writer.WriteBits(code);
	}

}

std::unordered_map<std::uint8_t, std::string> HuffmanCoding::ReadEncodingTable(BitReader& bit_reader) {
	
	std::unordered_map<std::uint8_t, std::string> encoding_table;
	
	// Read table size directly as 16 bits
	uint16_t table_size = 0;
	for (int i = 0; i < 16; ++i) {
		bool bit;
		if (!bit_reader.ReadBit(bit)) {
			throw std::runtime_error("Unexpected end of file while reading encoding table size");
		}
		table_size = (table_size << 1) | bit;
	}

	// For each entry
	// 1. Read a byte
	// 2. Read the length of the Huffman code for this byte
	// 3. Read the huffman code.
		// We want to convert these bytes back into a string of '0' and '1's
		// We will also need to trim any padding bits from the last byte based on code length.

	for (uint16_t i = 0; i < table_size; ++i) {
		// Read byte directly as 8 bits
		uint8_t byte = 0;
		for (int j = 0; j < 8; ++j) {
			bool bit;
			if (!bit_reader.ReadBit(bit)) {
				throw std::runtime_error("Unexpected end of file while reading byte");
			}
			byte = (byte << 1) | bit;
		}

		// Read code length directly as 8 bits
		uint8_t code_length = 0;
		for (int j = 0; j < 8; ++j) {
			bool bit;
			if (!bit_reader.ReadBit(bit)) {
				throw std::runtime_error("Unexpected end of file while reading code length");
			}
			code_length = (code_length << 1) | bit;
		}

		// Read the code
		std::string code;
		for (uint8_t j = 0; j < code_length; ++j) {
			bool bit;
			if (!bit_reader.ReadBit(bit)) {
				throw std::runtime_error("Unexpected end of file while reading code bits");
			}
			code += bit ? '1' : '0';
		}
		encoding_table[byte] = code;
	}

	return encoding_table;

}

void HuffmanCoding::encode(std::ifstream& input_file, std::ofstream& output_file,
	std::optional<ProgressCallback> progress_callback) {

	// Build frequency table from input file.
	auto freq_table = BuildFrequencyTable(input_file);

	// Construct Huffman tree.
	auto root = BuildHuffmanTree(freq_table);

	// Create encoding table
	std::unordered_map<std::uint8_t, std::string> encoding_table;
	std::vector<char> code;
	BuildEncodingTable(root, code, encoding_table);

	// Write encoding table to output file
	BitWriter bit_writer(&output_file);
	WriteEncodingTable(encoding_table, bit_writer);

	// Calculate and write total encoded bits
	std::int64_t total_encoded_bits = 0;
	for (const auto& [byte, code] : encoding_table) {
		total_encoded_bits += code.length() * freq_table[byte];
	}
	for (int i = 63; i >= 0; --i) {
		bit_writer.WriteBit((total_encoded_bits >> i) & 1);
	}

	// Reset input file to beginning
	input_file.clear();
	input_file.seekg(0);


	// Encode the file
		// 1. For each byte, look up its Huffman code
		// append this code to bit buffer
		// 2. When the bit buffer has 8 or more bits, write the first 8 bits as a byte
		// to output file and remove from buffer
	// After processing all input, if there are any bits left in the buffer, pad to 8 bits and write the final byte.
	std::vector<std::uint8_t> buffer(BUFFER_SIZE);
	std::int64_t total_processed = 0;

	while (input_file) {
		input_file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
		
		size_t bytes_read = input_file.gcount();

		for (size_t i = 0; i < bytes_read; ++i) {
			// Look up huffman code in table and add it to the buffer.
			bit_writer.WriteBits(encoding_table[buffer[i]]);
		}

		total_processed += bytes_read;
		if (progress_callback) {
			(*progress_callback)(total_processed);
		}
	}

	// Ensure that any extra bits are flushed to the buffer.
	bit_writer.Flush();
	
}

// 1. Read Encoding table
// 2. Build Decoding tree
// 3. Decode data.
void HuffmanCoding::decode(std::ifstream& input_file, std::ofstream& output_file,
	std::optional<ProgressCallback> progress_callback) {

	BitReader bit_reader(&input_file);

	// Read the encoding table.
	auto encoding_table = ReadEncodingTable(bit_reader);

	// Build decode tree from the encoding table
	auto root = BuildDecodingTree(encoding_table);

	// Read total encoded bits
	std::int64_t total_encoded_bits = 0;
	for (int i = 0; i < 64; ++i) {
		bool bit;
		if (!bit_reader.ReadBit(bit)) {
			throw std::runtime_error("Unexpected end of file while reading total encoded bits");
		}
		total_encoded_bits = (total_encoded_bits << 1) | bit;
	}


	std::vector<std::uint8_t> output_buffer(BUFFER_SIZE);
	size_t buffer_index = 0;

	std::int64_t bits_processed = 0;
	std::int64_t bytes_decoded = 0;

	auto curr_node = root;

	while (bits_processed < total_encoded_bits) {
		bool bit;
		if (!bit_reader.ReadBit(bit)) {
			break;
		}
		++bits_processed;

		curr_node = bit ? curr_node->right : curr_node->left;

		if (!curr_node) {
			throw std::runtime_error("Invalid Huffman code encountered during decoding");
		}

		if (!curr_node->left && !curr_node->right) {
			// Add decoded byte to output buffer
			output_buffer[buffer_index++] = static_cast<std::uint8_t>(curr_node->data);
			++bytes_decoded;

			// If buffer is full, write it to the output file
			if (buffer_index == BUFFER_SIZE) {
				output_file.write(reinterpret_cast<char*>(output_buffer.data()), BUFFER_SIZE);
				buffer_index = 0;

				// Report progress
				if (progress_callback) {
					(*progress_callback)(bytes_decoded);
				}
			}

			curr_node = root;
		}
	}


	// Write any remaining bytes in the buffer
	if (buffer_index > 0) {
		output_file.write(reinterpret_cast<char*>(output_buffer.data()), buffer_index);
	}

	// Check if we've processed all expected bits
	if (bits_processed < total_encoded_bits) {
		throw std::runtime_error("Unexpected end of file: decoded fewer bits than expected");
	}

	if (curr_node != root) {
		throw std::runtime_error("Unexpected end of file: incomplete Huffman code");
	}



}