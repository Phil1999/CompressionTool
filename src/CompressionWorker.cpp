#include "CompressionWorker.h"
#include "EncodingAlgorithms.h"
#include "CompressionExceptions.h" 
#include "fstream"
#include <qfileinfo.h>


CompressionWorker::CompressionWorker(QObject* parent) 
	: QObject(parent) 
{}

void CompressionWorker::compress(const QString& input_file, const QString& output_file, AlgorithmType selected_algo) {
	try {
		input_path_ = std::filesystem::path(input_file.toStdString());
		output_path_ = std::filesystem::path(output_file.toStdString());

		std::ifstream input(input_path_, std::ios::binary);
		std::ofstream output(output_path_, std::ios::binary);

		if (!input || !output) {
			throw FileOpenException((!input ? input_file : output_file).toStdString());
		}
		// Write metadata into file when encoding to determine original extension and algorithim used.
		FileHeader header(CompressionWorker::GetMagicNumber(selected_algo), input_path_.extension().string());
		WriteHeader(output, header);

		qint64 total_size = QFileInfo(input_file).size();

		switch (selected_algo) {
		case AlgorithmType::RLE:
			// Call RLE encode and update progress as it proceeds.
			EncodingAlgorithms::RLECoding::encode(input, output, [this, total_size](std::int64_t processed_size) {
				int progress = static_cast<int>((processed_size * 100) / total_size);
				emit ProgressUpdated(progress);
			});

			break;
		case AlgorithmType::Huffman:
			EncodingAlgorithms::HuffmanCoding::encode(input, output, [this, total_size](std::int64_t processed_size) {
				int progress = static_cast<int>((processed_size * 100) / total_size);
				emit ProgressUpdated(progress);
			});
			break;
		default:
			throw CompressionException("Unknown algorithm type");
		}

		// Ensure we always end at 100%
		emit ProgressUpdated(100);
		emit completed();
	}
	catch (const std::exception& e) {
		emit error(QString::fromStdString(e.what()));
	}
}

void CompressionWorker::decompress(const QString& input_file, const QString& output_file, AlgorithmType selected_algo) {
	try {
		input_path_ = std::filesystem::path(input_file.toStdString());
		output_path_ = std::filesystem::path(output_file.toStdString());

		std::ifstream input(input_path_, std::ios::binary);
		std::ofstream output(output_path_, std::ios::binary);

		// Early return if either file fails to open
		if (!input || !output) {
			throw FileOpenException((!input ? input_file : output_file).toStdString());
		}

		// Read file header and validate magic number
		FileHeader header = ReadHeader(input);


		AlgorithmType file_algo;

		if (header.is_valid_magic_number(RLE_MAGIC_NUMBER)) {
			file_algo = AlgorithmType::RLE;
		}
		else if (header.is_valid_magic_number(HUFFMAN_MAGIC_NUMBER)) {
			file_algo = AlgorithmType::Huffman;
		}
		else {
			throw InvalidHeaderException("Unknown compression file format");
		}

		// Check if selected algorithm matches the file's algorithm
		if (selected_algo != file_algo) {
			throw InvalidHeaderException("Selected algorithm does not match the file's compression method");
		}

		qint64 total_size = QFileInfo(input_file).size();

		switch (file_algo) {
		case AlgorithmType::RLE:
			EncodingAlgorithms::RLECoding::decode(input, output, [this, total_size](std::int64_t processed_size) {
				int progress = static_cast<int>((processed_size * 100) / total_size);
				emit ProgressUpdated(progress);
				});
			break;
		case AlgorithmType::Huffman:
			EncodingAlgorithms::HuffmanCoding::decode(input, output, [this, total_size](std::int64_t processed_size) {
				int progress = static_cast<int>((processed_size * 100) / total_size);
				emit ProgressUpdated(progress);
				});
			break;
		}

		// Ensure we always end at 100%
		emit ProgressUpdated(100);
		emit completed();
	}
	catch (const std::exception& e) {
		emit error(QString::fromStdString(e.what()));
	}
}

FileHeader CompressionWorker::ReadHeader(std::ifstream& input_file) {
	return FileHeader::read(input_file);
}

void CompressionWorker::WriteHeader(std::ofstream& output_file, const FileHeader& header) {
	header.write(output_file);
}

std::array<char, FileHeader::MAGIC_NUMBER_SIZE> CompressionWorker::GetMagicNumber(AlgorithmType algo) {
	switch (algo) {
	case AlgorithmType::RLE:
		return RLE_MAGIC_NUMBER;

	case AlgorithmType::Huffman:
		return HUFFMAN_MAGIC_NUMBER;

	default:
		throw CompressionException("Unknown algorithm type");
	}
}