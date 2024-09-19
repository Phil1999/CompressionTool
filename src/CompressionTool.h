#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QComboBox>
#include <string>
#include <array>
#include <string_view>
#include <ostream>
#include <fstream>


class CompressionTool : public QMainWindow
{
    Q_OBJECT

public:

    CompressionTool(QWidget *parent = nullptr);
    ~CompressionTool();

private slots:
    void SelectFile();
    void CompressFile();
    void DecompressFile();
    void OnAlgorithmChanged(int index);


private:


    static constexpr size_t MAGIC_NUMBER_SIZE = 3;
    static constexpr size_t VERSION_SIZE = 1;
    static constexpr size_t EXTENSION_LENGTH_SIZE = 1;
    static constexpr size_t VERSION_NUMBER = 1;
    static constexpr size_t BUFFER_SIZE = 8192; // 8 KB buffer
    static constexpr unsigned char ESCAPE = 255;


    enum class Algorithm {
        kRle, // Run-Length Encoding
        kHuffman, // Huffman Coding
    };

    struct FileHeader {
        std::array<char, MAGIC_NUMBER_SIZE> magic_number;
        uint8_t version = VERSION_NUMBER;
        std::string original_extension;

        [[nodiscard]] bool is_valid_magic_number(std::string_view expected) const {
            return std::string_view(magic_number.data(), magic_number.size()) == expected;
        }

    };
 

    FileHeader ReadHeader(std::ifstream& input_file);
    void WriteHeader(std::ofstream& output_file, const FileHeader& header);


    void SetupLayout();

    Algorithm selected_algorithm_ = Algorithm::kRle;
    QString original_file_extension_;

    QLineEdit* file_input_;
    QLabel* status_label_;
    QPushButton* select_file_button_;
    QPushButton* compress_button_;
    QPushButton* decompress_button_;
    QComboBox* algorithm_selector_;


    // RLE
    void RunLengthEncode(const std::string& input_file_path, const std::string& output_file_path);
    void RunLengthDecode(const std::string& input_file_path, const std::string& output_file_path);

    // Huffman
    void HuffmanEncode(const std::string& input_file_path, const std::string& output_file_path);
    void HuffmanDecode(const std::string& input_file_path, const std::string& output_file_path);



};
