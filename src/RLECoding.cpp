#include "RLECoding.h"
#include <fstream>
#include <algorithm>


void RLECoding::encode(std::ifstream& input_file, std::ofstream& output_file) {
    std::vector<unsigned char> input_buffer(BUFFER_SIZE);
    std::vector<unsigned char> output_buffer;
    output_buffer.reserve(BUFFER_SIZE);

    unsigned char run_char = 0;
    unsigned char run_char_count = 0;


    while (input_file) {
        // Try to read in up to 8kb of data.
        input_file.read(reinterpret_cast<char*>(input_buffer.data()), input_buffer.size());

        // Keep track of the actual amount of bytes we've read.
        size_t bytes_read = input_file.gcount();

        for (size_t i = 0; i < bytes_read; ++i) {
            // Go through each byte in our buffer and process using RLE algorithm.
            unsigned char current_char = input_buffer[i];

            // We hit a repeating character so increase our count.
            if (current_char == run_char && run_char_count < ESCAPE) {
                run_char_count++;
            }
            // Hit a new character so reset our values and write to output file.
            else {
                writeRun(output_buffer, output_file, run_char, run_char_count);
                run_char = current_char;
                run_char_count = 1;
            }
        }
    }

    // Flush again to handle remaining data.
    writeRun(output_buffer, output_file, run_char, run_char_count);

    // Write any remaining data in the output buffer. (we may not fill 
    // our byte quota using writeRun since it only writes in 8kb chunks).
    if (!output_buffer.empty()) {
        flushBuffer(output_buffer, output_file);
    }

}

void RLECoding::decode(std::ifstream& input_file, std::ofstream& output_file) {
    std::vector<unsigned char> input_buffer(BUFFER_SIZE);
    std::vector<unsigned char> output_buffer;
    output_buffer.reserve(BUFFER_SIZE);


    while (input_file) {
        // Try reading up to 8kb of data.
        input_file.read(reinterpret_cast<char*>(input_buffer.data()), input_buffer.size());

        size_t bytes_read = input_file.gcount();

        for (size_t i = 0; i < bytes_read; i += 2) {
            // Read as a pair (char, count)
            if (i + 1 >= bytes_read) break;

            unsigned char character = input_buffer[i];
            unsigned char character_count = input_buffer[i + 1];

            // Handle our ESCAPE sequence
            if (character == ESCAPE && character_count == 0) {

                // Ensure we have next complete pair.
                if (i + 3 >= bytes_read) break;

                character = input_buffer[i + 2];
                character_count = ESCAPE;

                i += 2; // Skip the next pair as we've jsut processed it.
            }

            output_buffer.insert(output_buffer.end(), character_count, character);
            if (output_buffer.size() >= BUFFER_SIZE) {
                flushBuffer(output_buffer, output_file);
                output_buffer.clear();
            }
        }

    }

    // Write any remaining data in the output buffer.
    if (!output_buffer.empty()) {
        flushBuffer(output_buffer, output_file);
    }
}

void RLECoding::writeRun(std::vector<unsigned char>& buffer, std::ofstream& output_file, unsigned char character, unsigned char count) {
    // Prevent a write for runs of 0 length.
    if (count > 0) {
        // If we hit out 255 byte limit. Mark it with our ESCAPE value.
        if (count == ESCAPE) {
            buffer.push_back(ESCAPE);
            buffer.push_back(0);
        }
        buffer.push_back(character);
        buffer.push_back(count);

        // Dump our buffer when we hit our intended BUFFER_SIZE (8kb).
        if (buffer.size() >= BUFFER_SIZE) {
            flushBuffer(buffer, output_file);
            buffer.clear();
        }
    }
}

void RLECoding::flushBuffer(const std::vector<unsigned char>& buffer, std::ofstream& output_file) {
    output_file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
}