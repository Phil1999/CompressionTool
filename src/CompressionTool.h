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
#include "FileHeader.h"


class CompressionTool : public QMainWindow
{
    Q_OBJECT

public:

    CompressionTool(QWidget *parent = nullptr);
    ~CompressionTool() override;

private slots:
    void SelectFile();
    void CompressFile();
    void DecompressFile();
    void OnAlgorithmChanged(int index);
    void ShowInfoWindow();


private:
    enum class Algorithm {
        kRle, // Run-Length Encoding
        kHuffman, // Huffman Coding
    };


    void SetupLayout();

    Algorithm selected_algorithm_ = Algorithm::kRle;
    QString original_file_extension_;

    QLineEdit* file_input_;
    QLabel* status_label_;
    QPushButton* select_file_button_;
    QPushButton* compress_button_;
    QPushButton* decompress_button_;
    QComboBox* algorithm_selector_;
    QStatusBar* status_bar_;
    QPushButton* info_button_;
};

