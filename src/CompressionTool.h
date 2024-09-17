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

    enum class Algorithm {
        kRle, // Run-Length Encoding
        kHuffman, // Huffman Coding
    };

    Algorithm selected_algorithm_ = Algorithm::kRle;

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


    void SetupLayout();


};
