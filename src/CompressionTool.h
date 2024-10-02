// CompressionTool.h
//
// CompressionTool is a Qt-based GUI application for compressing and decompressing
// files using algorithms like Run-Length Encoding (RLE) and Huffman Coding.
// It provides a user-friendly interface for selecting files, choosing algorithms, 
// and tracking the progress of compression and decompression operations.
//
// This class manages the UI components, file handling, and interactions with
// the CompressionWorker, which performs the actual compression and decompression 
// in a separate thread to keep the UI responsive.

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
#include <QThread>
#include <QTimer>
#include <filesystem>
#include "CompressionWorker.h"

/**
* @class CompressionTool
* @brief Main window class for the compression tool.
*
* The CompressionTool class provides a GUI for compressing and decompressing files.
* It allows the user to select a file, choose the compression algorithm, and view
* the progress of the operation. Compression and decompression tasks are handled
* by the CompressionWorker, which runs on a separate thread.
*/
class CompressionTool : public QMainWindow
{
    Q_OBJECT

public:


    /**
    * @brief Constructs the CompressionTool main window.
    *
    * Initializes the UI components and sets up the worker thread for handling
    * compression and decompression tasks. Also connects signals for user interaction
    * with the worker.
    *
    * @param parent: Optional parent widget
    */
    CompressionTool(QWidget *parent = nullptr);

    /**
    * @brief Destructor for CompressionTool.
    *
    * Ensures that the worker thread is properly stopped and cleaned up.
    */
    ~CompressionTool() override;

private slots:
    /**
    * @brief Opens a file selection dialog to choose a file for compression/decompression.
    *
    * The selected file path is displayed in the file input field, and stored for later use
    * during compression or decompression.
    */
    void SelectFile();

    /**
    * @brief Compresses the selected file using the chosen algorithm.
    *
    * Handles the file compression process, checks for valid input, and invokes the
    * compression task on the worker thread. Updates the UI with progress information.
    */
    void CompressFile();

    /**
    * @brief Decompresses the selected file using the chosen algorithm.
    *
    * Handles the file decompression process, checks for valid input, and invokes the
    * decompression task on the worker thread. Also updates the UI with progress information.
    */
    void DecompressFile();

    /**
    * @brief Changes the selected compression algorithm based on user selection.
    *
    * Updates the internal state to reflect the selected compression algorithm.
    * This function is triggered when the user changes the selection in the algorithm dropdown.
    *
    * @param index: The index of the selected algorithm in the dropdown.
    */
    void OnAlgorithmChanged(int index);


    /**
    * @brief Displays an informational dialog about the program.
    *
    * Opens a dialog window with information about the compression tool, including
    * details on the available algorithms and usage instructions.
    */
    void ShowInfoWindow();

    /**
    * @brief Updates the progress bar as the compression or decompression operation proceeds.
    *
    * @param percentage: The progress percentage (0-100) of the ongoing operation.
    */
    void UpdateProgress(int percentage);

    /**
    * @brief Slot triggered when the compression or decompression operation is completed.
    *
    * Resets the UI and displays a success message.
    */
    void OnCompressionCompleted();

    /**
    * @brief Slot triggered when an error occurs during compression or decompression.
    *
    * Displays an error message to the user and resets the UI to allow for further operations.
    *
    * @param errorMessage: The error message describing what went wrong.
    */
    void OnCompressionError(const QString& errorMessage);


private:

    /**
    * @brief Sets up the worker thread and connects signals to handle compression tasks.
    *
    * Moves the CompressionWorker to a separate thread and connects relevant signals
    * to keep the UI responsive during long-running operations.
    */
    void SetupWorkerThread();



    /**
    * @brief Sets up the layout and UI components for the main window.
    *
    * This function initializes and arranges the various UI elements like the file input,
    * buttons, progress bar, and status bar.
    */
    void SetupLayout();

    /**
    * @brief Resets the UI to its default state after an operation (compression/decompression) completes.
    *
    * This function restores the buttons, progress bar, and other UI elements to their
    * default states so that new operations can be started.
    */
    void ResetUIAfterOperation();

    /**
    * @brief Resets the status label in the status bar to "Ready" after a delay.
    *
    * This function is used to automatically clear any status messages after a
    * specified duration.
    */
    void ResetStatusLabel();

    // Default to RLE (because its where the selector starts by default)
    CompressionWorker::AlgorithmType selected_algorithm_ = CompressionWorker::AlgorithmType::RLE;
   
    std::filesystem::path original_file_path_;             ///< The path of the selected file for compression or decompression.


    QLineEdit* file_input_;
    QLabel* status_label_;
    QPushButton* select_file_button_;
    QPushButton* compress_button_;
    QPushButton* decompress_button_;
    QComboBox* algorithm_selector_;
    QStatusBar* status_bar_;
    QPushButton* info_button_;
    QProgressBar* progress_bar_;
    QTimer* status_reset_timer_;

    QThread worker_thread_;                                ///< Thread for running the CompressionWorker.
    CompressionWorker* worker_;                            ///< Worker object that performs compression/decompression.



    // Constants for window and timer configuration
    static constexpr int WINDOW_WIDTH = 300;              ///< Width of the main window.
    static constexpr int WINDOW_HEIGHT = 250;             ///< Height of the main window.
    static constexpr int TIMER_RESET_DURATION = 3000;     ///< Duration (in ms) before resetting the status label.
};
