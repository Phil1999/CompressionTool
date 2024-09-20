#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QStatusBar>
#include <memory>
#include <filesystem>
#include "CompressionWorker.h"


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

    // Handle CompressionWorker signals
    void UpdateProgress(int percentage);
    void OnCompressionCompleted();
    void OnCompressionError(const QString& errorMessage);


private:

    void SetupLayout();
    // Just default to RLE
    CompressionWorker::AlgorithmType selected_algorithm_ = CompressionWorker::AlgorithmType::RLE;
   
    std::filesystem::path original_file_path_;


    QLineEdit* file_input_;
    QLabel* status_label_;
    QPushButton* select_file_button_;
    QPushButton* compress_button_;
    QPushButton* decompress_button_;
    QComboBox* algorithm_selector_;
    QStatusBar* status_bar_;
    QPushButton* info_button_;
    QProgressBar* progress_bar_;

    std::unique_ptr<CompressionWorker> worker_;

    static constexpr int WINDOW_WIDTH = 300;
    static constexpr int WINDOW_HEIGHT = 200;
};
