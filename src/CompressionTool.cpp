#include "CompressionTool.h"
#include "RLECoding.h"
#include "FileHeader.h"
#include "CompressionExceptions.h"
#include <qfileinfo.h>
#include <qtextedit.h>
#include <qstatusbar.h>

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
    setFixedSize(QSize(WINDOW_WIDTH, WINDOW_HEIGHT));
}

CompressionTool::~CompressionTool() = default;


void CompressionTool::SetupLayout() {
    QWidget* central_widget = new QWidget(this);
    setCentralWidget(central_widget);

    auto* main_layout = new QVBoxLayout(central_widget);

    // Setup file input selector
    file_input_ = new QLineEdit(this);
    file_input_->setPlaceholderText(tr("Select a file..."));
    main_layout->addWidget(file_input_);

    // Select file
    select_file_button_ = new QPushButton(tr("Select File"), this);
    main_layout->addWidget(select_file_button_);

    // Algorithim selector
    algorithm_selector_ = new QComboBox(this);
    algorithm_selector_->addItem(tr("Run-Length Encoding"));
    algorithm_selector_->addItem(tr("Huffman Coding"));
    main_layout->addWidget(algorithm_selector_);

    // Buttons
    compress_button_ = new QPushButton(tr("Compress"), this);
    main_layout->addWidget(compress_button_);

    decompress_button_ = new QPushButton(tr("Decompress"), this);
    main_layout->addWidget(decompress_button_);


    // Status bar
    status_bar_ = new QStatusBar(this);
    status_bar_->setSizeGripEnabled(false);
    setStatusBar(status_bar_);

    auto status_widget = new QWidget(status_bar_);
    auto status_layout = new QHBoxLayout(status_widget);
    status_layout->setContentsMargins(0, 0, 0, 0);
    status_layout->setSpacing(0);


    // Status Label
    status_label_ = new QLabel(tr("Ready"), status_widget);
    status_label_->setAlignment(Qt::AlignCenter);
    status_bar_->addPermanentWidget(status_label_);

    // Spacer to push items to left and right
    status_layout->addStretch();


    // Info button
    info_button_ = new QPushButton(status_widget);
    info_button_->setIcon(QIcon::fromTheme("dialog-information"));
    info_button_->setToolTip(tr("About this program."));
    info_button_->setFixedSize(24, 24);
    info_button_->setFlat(true);
    status_layout->addWidget(info_button_);

    status_bar_->addPermanentWidget(status_widget);
    
    // Remove seperator
    status_bar_->setStyleSheet("QStatusBar::item {border: None;}");

    // Connect UI signals to appropriate slots
    connect(select_file_button_, &QPushButton::clicked, this, &CompressionTool::SelectFile);
    connect(compress_button_, &QPushButton::clicked, this, &CompressionTool::CompressFile);
    connect(decompress_button_, &QPushButton::clicked, this, &CompressionTool::DecompressFile);
    connect(algorithm_selector_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CompressionTool::OnAlgorithmChanged);
    connect(info_button_, &QPushButton::clicked, this, &CompressionTool::ShowInfoWindow);
}

void CompressionTool::ShowInfoWindow() {
    // Create dialog on stack to ensure no memory leaks
    QDialog info_dialog(this);

    info_dialog.setWindowTitle(tr("About"));
    info_dialog.setFixedSize(QSize(WINDOW_WIDTH * 2, WINDOW_HEIGHT * 2));

    auto* layout = new QVBoxLayout(&info_dialog);

    auto* info_text = new QTextEdit(&info_dialog);
    info_text->setReadOnly(true);
    info_text->setHtml(R"(
        <h2>Compression Tool By Philip Lee</h2>
        <p>This tool allows you to compress and decompress files using various algorithms:</p>
        <ul>
            <li><b>Run-Length Encoding (RLE):</b> A simple compression algorithm that works well for files with many repeated data sequences.</li>
            <li><b>Huffman Coding:</b> (Coming soon) An efficient compression technique that assigns variable-length codes to characters based on their frequency.</li>
        </ul>
        <p>To use the tool:</p>
        <ol>
            <li>Select a file to compress or decompress</li>
            <li>Choose the compression algorithm</li>
            <li>Click 'Compress' or 'Decompress' as needed</li>
        </ol>
        <p>Note: Compressed files (.rle or .huff) cannot be opened directly and must be decompressed using this tool before viewing.</p>
    )");

    layout->addWidget(info_text);

    info_dialog.setLayout(layout);
    info_dialog.exec();
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

    }
    catch (const CompressionException& e) {
        QMessageBox::critical(this, tr("Error"), tr(e.what()));
        return;
    }

    status_label_->setText("File decompressed successfully.");

}




