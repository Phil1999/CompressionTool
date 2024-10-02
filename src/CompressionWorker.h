// CompressionWorker.h
//
// CompressionWorker is a Qt-based worker class that handles file compression and
// decompression tasks. It supports multiple algorithms, such as Run-Length Encoding (RLE)
// and Huffman Coding. The worker runs in a separate thread to ensure that
// the main GUI remains responsive during long-running compression or decompression operations.


#pragma once

#include <qobject.h>
#include <qstring.h>
#include <filesystem>
#include "FileHeader.h"


/**
* @class CompressionWorker
* @brief Worker class that performs file compression and decompression.
*
* CompressionWorker executes file compression and decompression operations
* in a background thread, allowing the main GUI to remain responsive.
* It supports encoding algorithims, and informs the UI of the progress.
*/
class CompressionWorker : public QObject
{
	Q_OBJECT


public:

	/**
	* @brief Constructs a CompressionWorker.
	*
	* Initializes the worker object and prepares it for handling
	* compression and decompression tasks.
	*
	* @param parent: Optional parent QObject
	*/
	CompressionWorker(QObject* parent = nullptr);


	/**
	* @enum AlgorithmType
	* @brief Enum representing the supported compression algorithms.
	*/
	enum class AlgorithmType {
		RLE,
		Huffman
	};

public slots:


	/**
	* @brief Compresses a file using the selected algorithm.
	*
	* This slot is triggered to compress a file. It reads the input file, compresses
	* the data using the selected algorithm, and writes the compressed data
	* to the output file. Progress updates are emitted during the operation.
	*
	* @param input_file: Path to the file to be compressed.
	* @param output_file: Path to the output compressed file.
	* @param selected_algo: The compression algorithm to use.
	*/
	void compress(const QString& input_file, const QString& output_file, AlgorithmType selected_algo);

	/**
	* @brief Decompresses a file using the selected algorithm.
	*
	* This slot is triggered to decompress a file. It reads the input compressed file,
	* validates its header, decompresses the data, and writes the original file
	* to the output. Progress updates are emitted during the operation.
	*
	* @param input_file: Path to the compressed file to be decompressed.
	* @param output_file: Path to the output decompressed file.
	* @param selected_algo: The algorithm to use for decompression (must match the compression method used).
	*/
	void decompress(const QString& input_file, const QString& output_file, AlgorithmType selected_algo);

signals:

	/**
	* @brief Signal emitted when the progress of compression or decompression is updated.
	*
	* @param percentage: The current progress as a percentage (0-100).
	*/
	void ProgressUpdated(int percentage);

	/**
	* @brief Signal emitted when the compression or decompression task is completed.
	* 
	*/
	void completed();

	/**
	* @brief Signal emitted when an error occurs during compression or decompression.
	*
	* @param message: A descriptive error message detailing the issue.
	*/
	void error(const QString& message);

private:

	/**
	* @brief Reads the header of a compressed file to retrieve metadata.
	*
	* This function reads the header from the input file to extract the metadata
	*
	* @param input_file: The input file stream from which to read the header.
	* @return: A FileHeader object storing the metadata.
	*/
	static FileHeader ReadHeader(std::ifstream& input_file);

	/**
	* @brief Writes the header to the output file during compression.
	*
	* This function writes metadata to the output file.
	*
	* @param output_file: The output file stream to which the header is written.
	* @param header: The FileHeader object containing metadata to be written.
	*/
	static void WriteHeader(std::ofstream& output_File, const FileHeader& header);


	/**
	* @brief Retrieves the magic number associated with the selected compression algorithm.
	*
	* This function returns a magic number that is written to
	* the file header to indicate which compression algorithm was used.
	*
	* @param algo: The compression algorithm.
	* @return: A 3-character array representing the magic number.
	*/
	static std::array<char, FileHeader::MAGIC_NUMBER_SIZE> GetMagicNumber(AlgorithmType algo);

	std::filesystem::path input_path_;
	std::filesystem::path output_path_;

	// Magic numbers identifying the compression algorithm in the file header.
	static constexpr std::array<char, FileHeader::MAGIC_NUMBER_SIZE> RLE_MAGIC_NUMBER = { 'R', 'L', 'E' };
	static constexpr std::array<char, FileHeader::MAGIC_NUMBER_SIZE> HUFFMAN_MAGIC_NUMBER = { 'H', 'U', 'F' };
};

