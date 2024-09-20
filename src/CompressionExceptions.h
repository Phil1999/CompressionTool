#pragma once
#include <stdexcept>
#include <string>

class CompressionException : public std::runtime_error {
public:
	explicit CompressionException(const std::string& message) : std::runtime_error(message) {}
};

class FileOpenException : public CompressionException {
public:
	explicit FileOpenException(const std::string& filename) 
		: CompressionException("Failed to open file: " + filename) {}
};

class InvalidHeaderException : public CompressionException {
public:
	explicit InvalidHeaderException(const std::string& message)
		: CompressionException("Invalid header: " + message) {}
};

