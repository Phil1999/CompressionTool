#include "CompressionWorker.h"
#include "RLECoding.h"
#include "CompressionExceptions.h"
#include "fstream"
#include <qfileinfo.h>


CompressionWorker::CompressionWorker(QObject* parent) 
	: QObject(parent) 
{}

void CompressionWorker::compress(const QString& input_file, const QString& output_file, AlgorithmType algo) {
	try {
		input_path_ = std::filesystem::path(input_file.toStdString());
		output_path_ = std::filesystem::path(output_file.toStdString());

		std::ifstream input(input_path_, std::ios::binary);
		std::ofstream output(output_path_, std::ios::binary);

		if (!input || !output) {
			throw FileOpenException((!input ? input_file : output_file).toStdString());
		}
		// Write metadata into file when encoding to determine original extension and algorithim used.
		FileHeader header(CompressionWorker::GetMagicNumber(algo), input_path_.extension().string());
		CompressionWorker::WriteHeader(output, header);

		qint64 total_size = QFileInfo(input_file).size();

		switch (algo) {
		case AlgorithmType::RLE:
			// Call RLE encode and update progress as it proceeds.
			RLECoding::encode(input, output, [this, total_size](std::int64_t processed_size) {
				int progress = static_cast<int>((processed_size * 100) / total_size);
				emit ProgressUpdated(progress);
				});
			break;
		case AlgorithmType::Huffman:
			throw CompressionException("Huffman coding not implemented yet");
		default:
			throw CompressionException("Unknown algorithm type");
		}

		emit completed();
	}
	catch (const std::exception& e) {
		emit error(QString::fromStdString(e.what()));
	}
}

void CompressionWorker::decompress(const QString& input_file, const QString& output_file) {
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
		FileHeader header = CompressionWorker::ReadHeader(input);
		if (!header.is_valid_magic_number("RLE") && !header.is_valid_magic_number("HUF")) {
			throw InvalidHeaderException("Unknown compression file format");
		}

		qint64 total_size = QFileInfo(input_file).size();

		// Handle RLE compression
		if (header.is_valid_magic_number("RLE")) {
			RLECoding::decode(input, output, [this, total_size](std::int64_t processed_size) {
				int progress = static_cast<int>((processed_size * 100) / total_size);
				emit ProgressUpdated(progress);
			});
		}
		else if (header.is_valid_magic_number("HUF")) {
			throw CompressionException("Huffman coding not implemented yet");
		}

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
		return { 'R', 'L', 'E' };

	case AlgorithmType::Huffman:
		return { 'H', 'U', 'F' };

	default:
		throw CompressionException("Unknown algorithm type");
	}
}