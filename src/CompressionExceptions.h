// CompressionExceptions.h
//
// Defines custom exception classes for the compression tool.
//
// These exceptions handle specific error scenarios that may arise during
// compression or decompression processes.

#pragma once
#include <stdexcept>
#include <string>


/**
* @class CompressionException
* @brief Base class for all compression-related exceptions.
*
* This class serves as the base exception type for errors encountered during
* the compression and decompression processes. It inherits from
* `std::runtime_error` and can be used as a generic exception for any compression error.
*/
class CompressionException : public std::runtime_error {


public:

	/**
	* @brief Constructs a CompressionException with a given error message.
	*
	* @param message: A description of the error that occurred during the
	*                compression or decompression process.
	*/
	explicit CompressionException(const std::string& message) : std::runtime_error(message) {}
};

/**
* @class FileOpenException
* @brief Exception for file opening errors.
*
* This class is thrown when a file fails to open during compression or decompression.
*/
class FileOpenException : public CompressionException {
public:

	/**
	* @brief Constructs a FileOpenException for a specific file.
	*
	* The exception message includes the file name that could not be opened.
	*
	* @param filename: The name of the file that failed to open.
	*/
	explicit FileOpenException(const std::string& filename) 
		: CompressionException("Failed to open file: " + filename) {}
};

/**
* @class InvalidHeaderException
* @brief Exception for invalid headers in compressed files.
*
* This class is thrown when the header of a compressed file is invalid or
* malformed.
*/
class InvalidHeaderException : public CompressionException {
public:

	/**
	* @brief Constructs an InvalidHeaderException with a given error message.
	*
	* The exception message provides details on the problem with the file header.
	*
	* @param message: A description of the issue with the file header.
	*/
	explicit InvalidHeaderException(const std::string& message)
		: CompressionException("Invalid header: " + message) {}
};

