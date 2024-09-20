#pragma once

#include <qobject.h>
#include <qstring.h>
#include "RLECoding.h"
#include <filesystem>


class CompressionWorker : public QObject
{
	Q_OBJECT


public:
	CompressionWorker(QObject* parent = nullptr);

	enum class AlgorithmType {
		RLE,
		Huffman
	};

public slots:
	void compress(const QString& input_file, const QString& output_file, AlgorithmType algo);
	void decompress(const QString& input_file, const QString& output_file);

signals:
	void ProgressUpdated(int percentage);
	void completed();
	void error(const QString& message);

private:
	static FileHeader ReadHeader(std::ifstream& input_file);
	static void WriteHeader(std::ofstream& output_File, const FileHeader& header);

	static std::array<char, FileHeader::MAGIC_NUMBER_SIZE> GetMagicNumber(AlgorithmType algo);

	std::filesystem::path input_path_;
	std::filesystem::path output_path_;

};

