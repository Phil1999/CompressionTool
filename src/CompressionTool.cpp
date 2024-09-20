#include "CompressionTool.h"
#include "RLECoding.h"
#include "FileHeader.h"
#include "CompressionExceptions.h"
#include <qfileinfo.h>

//TODOS
// Flesh out comments
// Make confirmation window on overrwite when decompressing/compressing?
// Write tests
// Address clang tidy issues
// Clear file selection after running encode/decode


const int WINDOW_WIDTH = 300;
const int WINDOW_HEIGHT = 200;


CompressionTool::CompressionTool(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Compression Tool");
    SetupLayout();
    // Disable resizing
    this->setFixedSize(QSize(WINDOW_WIDTH, WINDOW_HEIGHT));
}

CompressionTool::~CompressionTool() = default;


void CompressionTool::SetupLayout() {

    // Setup file input selector
    file_input_ = new QLineEdit(this);
    file_input_->setPlaceholderText("Select a file...");

    // Buttons
    select_file_button_ = new QPushButton("Select File", this);
    compress_button_ = new QPushButton("Compress", this);
    decompress_button_ = new QPushButton("Decompress", this);

    status_label_ = new QLabel("Ready", this);

    // Algorithim selector
    algorithm_selector_ = new QComboBox(this);
    algorithm_selector_->addItem("Run-Length Encoding");
    algorithm_selector_->addItem("Huffman Coding");

    // Layout
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(10);
    layout->setContentsMargins(10, 10, 10, 10);

    layout->addWidget(file_input_);
    layout->addWidget(select_file_button_);
    layout->addWidget(algorithm_selector_);
    layout->addWidget(compress_button_);
    layout->addWidget(decompress_button_);
    layout->addWidget(status_label_);

    QWidget* central_widget = new QWidget(this);
    central_widget->setLayout(layout);
    setCentralWidget(central_widget);


    // Connect UI signals to appropriate slots
    connect(select_file_button_, &QPushButton::clicked, this, &CompressionTool::SelectFile);
    connect(compress_button_, &QPushButton::clicked, this, &CompressionTool::CompressFile);
    connect(decompress_button_, &QPushButton::clicked, this, &CompressionTool::DecompressFile);
    connect(algorithm_selector_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CompressionTool::OnAlgorithmChanged);
}

void CompressionTool::OnAlgorithmChanged(int index) {
    switch (index) {

    case 0:
        selected_algorithm_ = Algorithm::kRle;
        break;

    case 1:
        selected_algorithm_ = Algorithm::kHuffman;
        break;

    default:
        selected_algorithm_ = Algorithm::kRle;
        break;
    }
}

void CompressionTool::SelectFile() {

    QString file_path = QFileDialog::getOpenFileName(this, tr("Open File"), QString());

    if (!file_path.isEmpty()) {
        file_input_->setText(file_path);
    }
}

void CompressionTool::CompressFile() {

    // Ensure a file is selected
    QString input_file_path = file_input_->text();
    if (input_file_path.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a file to compress."));
        return;
    }


    // Determine output file based on the selected algorithm
    QString output_file_path = QFileInfo(input_file_path).absolutePath() + "/" +
        QFileInfo(input_file_path).completeBaseName() +
        (selected_algorithm_ == Algorithm::kRle ? ".rle" : ".huff");

    try {

        std::ifstream input_file(input_file_path.toStdString(), std::ios::binary);
        std::ofstream output_file(output_file_path.toStdString(), std::ios::binary);

        if (!input_file || !output_file) {
            throw FileOpenException(input_file ? output_file_path.toStdString() : input_file_path.toStdString());
        }


        // Write header
        std::array<char, FileHeader::MAGIC_NUMBER_SIZE> magic_number =
            selected_algorithm_ == Algorithm::kRle ? std::array<char, 3>{'R', 'L', 'E'} : std::array<char, 3>{'H', 'U', 'F'};

        FileHeader header(magic_number, QFileInfo(input_file_path).suffix().toStdString());
        header.write(output_file);


        if (selected_algorithm_ == Algorithm::kRle) {
            RLECoding::encode(input_file, output_file);
        }
        else if (selected_algorithm_ == Algorithm::kHuffman) {
            throw CompressionException("Huffman coding not yet implemented");
        }
        

        input_file.close();
        output_file.close();
    }
    catch (const CompressionException& e) {
        QMessageBox::critical(this, tr("Warning"), tr(e.what()));
        return;
    }

    status_label_->setText("File compressed successfully.");
}


void CompressionTool::DecompressFile() {

    // Ensure file is selected first.
    QString input_file_path = file_input_->text();
    if (input_file_path.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a file to decompress."));
        return;
    }

    try {

        std::ifstream input_file(input_file_path.toStdString(), std::ios::binary);
        if (!input_file) {
            throw FileOpenException(input_file_path.toStdString());
        }

        // Read and validate header first
        FileHeader header = FileHeader::read(input_file);

        Algorithm file_algorithm;
        if (header.is_valid_magic_number("RLE")) {
            file_algorithm = Algorithm::kRle;
        }
        else if (header.is_valid_magic_number("HUF")) {
            file_algorithm = Algorithm::kHuffman;
        }
        else {
            throw InvalidHeaderException("Unknown compressed file format");
        }

        if (selected_algorithm_ != file_algorithm) {
            throw CompressionException("The selected algorithm does not match the file's algorithm.");
        }

        // Set the output file path as the original filename with its original extension.
        QString output_file_path = QFileInfo(input_file_path).absolutePath() + "/" +
            QFileInfo(input_file_path).completeBaseName() + "." + QString::fromStdString(header.original_extension_);

        std::ofstream output_file(output_file_path.toStdString(), std::ios::binary);

        if (!output_file) {
            throw FileOpenException(output_file_path.toStdString());
        }

        // Perform decompression based on the selected algorithim.
        // We've already checked that header data matches up with selected algorithim.
        if (selected_algorithm_ == Algorithm::kRle) {
            RLECoding::decode(input_file, output_file);
        }
        else if (selected_algorithm_ == Algorithm::kHuffman) {
            throw CompressionException("Huffman decoding not yet implemented");
        }

        input_file.close();
        output_file.close();
    }
    catch (const CompressionException& e) {
        QMessageBox::critical(this, tr("Error"), tr(e.what()));
        return;
    }

    status_label_->setText("File decompressed successfully.");

}




