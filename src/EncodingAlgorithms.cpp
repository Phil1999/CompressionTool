#include "EncodingAlgorithms.h"
#include "BitReader.h"
#include "BitWriter.h"
#include <queue>
#include <iostream>
#include <bitset>

namespace EncodingAlgorithms {

    // HuffmanCoding implementation.
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


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // RLECoding implementation.
    void RLECoding::encode(std::ifstream& input_file, std::ofstream& output_file,
        std::optional<ProgressCallback> progress_callback) {

        std::vector<std::byte> input_buffer(BUFFER_SIZE);
        std::vector<std::byte> output_buffer;
        output_buffer.reserve(BUFFER_SIZE);

        std::byte run_char{};
        std::byte run_char_count{};


        std::int64_t total_processed = 0;

        while (input_file) {
            // Try to read in up to 8kb of data.
            input_file.read(reinterpret_cast<char*>(input_buffer.data()), input_buffer.size());

            // Keep track of the actual amount of bytes we've read.
            size_t bytes_read = input_file.gcount();

            for (size_t i = 0; i < bytes_read; ++i) {
                // Go through each byte in our buffer and process using RLE algorithm.
                std::byte current_char = input_buffer[i];

                // We hit a repeating character so increase our count.
                if (current_char == run_char && run_char_count < ESCAPE) {
                    // Since we are dealing with std::byte we need to static cast when performing arithmetic operations
                    run_char_count = static_cast<std::byte>(std::to_integer<unsigned char>(run_char_count) + 1);
                }
                // Hit a new character so reset our values and write to output file.
                else {
                    writeRun(output_buffer, output_file, run_char, run_char_count);
                    run_char = current_char;
                    run_char_count = std::byte{ 1 };
                }
            }

            total_processed += bytes_read;

            if (progress_callback) {
                (*progress_callback)(total_processed);
            }
        }

        // Flush again to handle remaining data.
        writeRun(output_buffer, output_file, run_char, run_char_count);

        // Write any remaining data in the output buffer. (we may not fill 
        // our byte quota using writeRun since it only writes in 8kb chunks).
        if (!output_buffer.empty()) {
            flushBuffer(output_buffer, output_file);
            output_buffer.clear();
        }

    }

    void RLECoding::decode(std::ifstream& input_file, std::ofstream& output_file,
        std::optional<ProgressCallback> progress_callback) {

        std::vector<std::byte> input_buffer(BUFFER_SIZE);
        std::vector<std::byte> output_buffer;
        output_buffer.reserve(BUFFER_SIZE);


        std::int64_t total_processed = 0;

        while (input_file) {
            // Try reading up to BUFFER_SIZE of data.
            input_file.read(reinterpret_cast<char*>(input_buffer.data()), input_buffer.size());

            size_t bytes_read = input_file.gcount();

            for (size_t i = 0; i < bytes_read; i += 2) {
                // Read as a pair (char, count)
                if (i + 1 >= bytes_read) break;

                std::byte character = input_buffer[i];
                std::byte character_count = input_buffer[i + 1];

                // Handle our ESCAPE sequence
                if (character == ESCAPE && character_count == std::byte{ 0 }) {

                    // Ensure we have next complete pair.
                    if (i + 3 >= bytes_read) break;

                    character = input_buffer[i + 2];
                    character_count = ESCAPE;

                    i += 2; // Skip the next pair as we've just processed it.
                }

                auto repeat_count = std::to_integer<size_t>(character_count);
                output_buffer.insert(output_buffer.end(), repeat_count, character);

                if (output_buffer.size() >= BUFFER_SIZE) {
                    flushBuffer(output_buffer, output_file);
                    output_buffer.clear();
                }
            }

            total_processed += bytes_read;
            if (progress_callback) {
                (*progress_callback)(total_processed);
            }

        }

        // Write any remaining data in the output buffer.
        if (!output_buffer.empty()) {
            flushBuffer(output_buffer, output_file);
        }
    }

    void RLECoding::writeRun(std::vector<std::byte>& buffer, std::ofstream& output_file, std::byte character, std::byte count) {
        // Prevent a write for runs of 0 length.
        if (count > std::byte{ 0 }) {
            // If we hit out 255 byte limit. Mark it with our ESCAPE value.
            if (count == ESCAPE) {
                buffer.push_back(ESCAPE);
                buffer.push_back(std::byte{ 0 });
            }
            buffer.push_back(character);
            buffer.push_back(count);

            // Dump our buffer when we hit our intended BUFFER_SIZE.
            if (buffer.size() >= BUFFER_SIZE) {
                flushBuffer(buffer, output_file);
                buffer.clear();
            }
        }
    }

    void RLECoding::flushBuffer(const std::vector<std::byte>& buffer, std::ofstream& output_file) {
        output_file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());

    }

}