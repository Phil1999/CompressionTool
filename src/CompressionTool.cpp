#include "CompressionTool.h"
#include <iostream>
#include <fstream>
#include <ostream>

//TODOS
// Flesh out comments
// Make confirmation window on overrwite when decompressing/compressing?
// Write tests
// more error handling if necessary


CompressionTool::CompressionTool(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Compression Tool");
    resize(300, 200);

    SetupLayout();
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

// TODO add more robust error handling, (dont encode huffman then decode rle, etc)
void CompressionTool::CompressFile() {

    // Ensure a file is selected
    QString input_file_path = file_input_->text();
    if (input_file_path.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a file to compress."));
        return;
    }

    // Store the original file extension
    original_file_extension_ = QFileInfo(input_file_path).suffix();

    // Determine output file based on the selected algorithm
    QString output_file_path;
    if (selected_algorithm_ == Algorithm::kRle) {
        output_file_path = QFileInfo(input_file_path).absolutePath() + "/" +
                           QFileInfo(input_file_path).completeBaseName() + ".rle";
    } else if (selected_algorithm_ == Algorithm::kHuffman) {
        output_file_path = QFileInfo(input_file_path).absolutePath() + "/" +
                           QFileInfo(input_file_path).completeBaseName() + ".huff";
    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Unknown compression algorithm selected."));
        return;
    }

    // Create output file and write header
    std::ofstream output_file(output_file_path.toStdString(), std::ios::binary);
    if (!output_file) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to create output file."));
        return;
    }

    // Write the header (Magic Number, Version, Extension Length, Extension)
    if (selected_algorithm_ == Algorithm::kRle) {
        output_file.write("RLE", 3);  // Magic Number for RLE
    } else if (selected_algorithm_ == Algorithm::kHuffman) {
        output_file.write("HUF", 3);  // Magic Number for Huffman
    }

    uint8_t version = 1;
    output_file.write(reinterpret_cast<const char*>(&version), sizeof(version));

    uint8_t extension_length = static_cast<uint8_t>(original_file_extension_.length());
    output_file.write(reinterpret_cast<const char*>(&extension_length), sizeof(extension_length));
    output_file.write(original_file_extension_.toStdString().c_str(), extension_length);


    // Ensure header is on its own line
    output_file.close();

    // Perform compression based on the selected algorithm
    if (selected_algorithm_ == Algorithm::kRle) {
        RunLengthEncode(input_file_path.toStdString(), output_file_path.toStdString());
    } else if (selected_algorithm_ == Algorithm::kHuffman) {

        // TODO
        //HuffmanEncode(input_file_path.toStdString(), output_file_path.toStdString());
    }
}


void CompressionTool::DecompressFile() {

    // Ensure file is selected first.
    QString input_file_path = file_input_->text();
    if (input_file_path.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a file to decompress."));
        return;
    }

    std::ifstream input_file(input_file_path.toStdString(), std::ios::binary);
    if (!input_file) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to open input file."));
        return;
    }

    // Determine the algorithm used and check file extension.
    QString file_extension = QFileInfo(input_file_path).suffix();


    if (file_extension != "rle" && file_extension != "huff") {
        QMessageBox::warning(this, tr("Warning"), tr("Unsupported file extension. Only .rle and .huff files can be decompressed."));
        return;
    }
    

    // Read and validate header
    char file_magic_number[3];
    input_file.read(file_magic_number, 3);
    if (input_file.gcount() != 3) {
        QMessageBox::critical(this, tr("Error"), tr("Invalid compressed file format."));
        return;
    }

    // Determine the algorithm used from the magic number
    Algorithm file_algorithm;

    if (strncmp(file_magic_number, "RLE", 3) == 0) {
        file_algorithm = Algorithm::kRle;
    }
    else if (strncmp(file_magic_number, "HUF", 3) == 0) {
        file_algorithm = Algorithm::kHuffman;
    }
    else {
        QMessageBox::critical(this, tr("Error"), tr("Unknown compressed file format"));
        return;
    }

    if (selected_algorithm_ != file_algorithm) {
        QString error_message = tr("The selected algorithm does not match the file's compression algorithm. "
            "Please select the correct algorithm.");
        QMessageBox::critical(this, tr("Error"), error_message);
        return;
    }


    // Read version and validate
    uint8_t version;
    input_file.read(reinterpret_cast<char*>(&version), sizeof(version));

    if (input_file.gcount() != sizeof(version) || version != 1) {
        QMessageBox::critical(this, tr("Error"), tr("Unsupported file version."));
        return;
    }

    uint8_t extension_length;
    input_file.read(reinterpret_cast<char*>(&extension_length), sizeof(extension_length));
    if (input_file.gcount() != sizeof(extension_length) || extension_length == 0) {
        QMessageBox::critical(this, tr("Error"), tr("Invalid or missing metadata in compressed file."));
        return;
    }
    
    std::string original_extension(extension_length, '\0');
    input_file.read(&original_extension[0], extension_length);

    if (input_file.gcount() != extension_length) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to read original file extension."));
        return;
    }

    // Set the output file path as the original filename with its original extension.
    QString output_file_path = QFileInfo(input_file_path).absolutePath() + "/" +
        QFileInfo(input_file_path).completeBaseName() + "." + QString::fromStdString(original_extension);

    input_file.close();

    // Perform decompression based on the selected algorithim
    if (selected_algorithm_ == Algorithm::kRle) {
        RunLengthDecode(input_file_path.toStdString(), output_file_path.toStdString());
    }
    else if (selected_algorithm_ == Algorithm::kHuffman) {
        // placeholder
    }

}




// RLE Algoritimn
// 1. Open input/output files in binary mode
// 2. Read the input file char by char
// 3. Keep track of current char, prev char, and a count of ocnsecutive identical chars.
// 4. When we encounter a different char or reach the end of the file, write the prev char and its count to output file.
// 5. Repeat until we've processed the entire input file.
// Ex: 0x48 0x65 0x6C 0x6C 0x6F 0x0D 0x0A 0x57 0x6F 0x72 0x6C 0x64 = "Hello\r\nWorld"
// We can handle runs longer than 255 by having a special escape character 
// (255, 0), ('A', 255), (255, 0), ('A', 255) would represent a run of 510 'A'
void CompressionTool::RunLengthEncode(const std::string& input_file_path,
    const std::string& output_file_path) {

    // Open input/outfile files in binary mode.
    std::ifstream input_file(input_file_path, std::ios::binary);
    std::ofstream output_file(output_file_path, std::ios::binary | std::ios::app);

    if (!input_file || !output_file) {
        QMessageBox::critical(this, "Error", "Failed to open input or output file.");
        return;
    }

    const size_t BUFFER_SIZE = 8192; // 8 KB buffer
    const unsigned char ESCAPE = 255;
    std::vector<unsigned char> input_buffer(BUFFER_SIZE);
    std::vector<unsigned char> output_buffer;
    output_buffer.reserve(BUFFER_SIZE);

    unsigned char run_char = 0;
    unsigned char run_char_count = 0;


    auto writeRun = [&](unsigned char character, unsigned char character_count) {
        // Prevent a write for runs of 0 length.
        if (character_count > 0) {
            // If we hit out 255 byte limit. Mark it with our ESCAPE value.
            if (character_count == ESCAPE) {
                output_buffer.push_back(ESCAPE);
                output_buffer.push_back(0);
            }
            output_buffer.push_back(character);
            output_buffer.push_back(character_count);

            // Dump our buffer when we hit our intended BUFFER_SIZE (8kb).
            if (output_buffer.size() >= BUFFER_SIZE) {
                output_file.write(reinterpret_cast<const char*>(output_buffer.data()), output_buffer.size());
                output_buffer.clear();
            }
        }
    };

    while (input_file) {
        // Try to read in up to 8kb of data.
        input_file.read(reinterpret_cast<char*>(input_buffer.data()), input_buffer.size());

        // Keep track of the actual amount of bytes we've read.
        size_t bytes_read = input_file.gcount();

        for (size_t i = 0; i < bytes_read; ++i) {
            // Go through each byte in our buffer and process using RLE algorithm.
            unsigned char current_char = input_buffer[i];

            // We hit a repeating character so increase our count.
            if (current_char == run_char && run_char_count < ESCAPE) {
                run_char_count++;
            }
            // Hit a new character so reset our values and write to output file.
            else {
                writeRun(run_char, run_char_count);
                run_char = current_char;
                run_char_count = 1;
            }
        }
    }

    // Flush again to handle remaining data.
    writeRun(run_char, run_char_count);

    // Write any remaining data in the output buffer. (we may not fill 
    // our byte quota using writeRun since it only writes in 8kb chunks).
    if (!output_buffer.empty()) {
        output_file.write(reinterpret_cast<const char*>(output_buffer.data()), output_buffer.size());
    }

    input_file.close();
    output_file.close();

    status_label_->setText("File compressed successfully.");

}

void CompressionTool::RunLengthDecode(const std::string& input_file_path,
    const std::string& output_file_path) {

    // Open input/outfile files in binary mode.
    std::ifstream input_file(input_file_path, std::ios::binary);
    std::ofstream output_file(output_file_path, std::ios::binary);

    if (!input_file || !output_file) {
        QMessageBox::critical(this, "Error", "Failed to open input or output file.");
        return;
    }

    const size_t BUFFER_SIZE = 8192; // 8 KB buffer
    const unsigned char ESCAPE = 255;
    std::vector<unsigned char> input_buffer(BUFFER_SIZE);
    std::vector<unsigned char> output_buffer;
    output_buffer.reserve(BUFFER_SIZE);

    // Skip the header in input file by advancing stream by header size.
   // 3 bytes for magic number, 1 for version, 1 for extension length, and extension bytes
    input_file.seekg(3 + 1 + 1 + original_file_extension_.length(), std::ios::beg);

    auto writeDecodedRun = [&](unsigned char character, unsigned char character_count) {

        for (unsigned char i = 0; i < character_count; ++i) {
            output_buffer.push_back(character);
            if (output_buffer.size() >= BUFFER_SIZE) {
                output_file.write(reinterpret_cast<const char*>(output_buffer.data()), output_buffer.size());
                output_buffer.clear();
            }
        }
    };

    while (input_file) {
        // Try reading up to 8kb of data.
        input_file.read(reinterpret_cast<char*>(input_buffer.data()), input_buffer.size());

        size_t bytes_read = input_file.gcount();

        for (size_t i = 0; i < bytes_read; i += 2) {
            // Read as a pair (char, count)
            if (i + 1 >= bytes_read) break;

            unsigned char character = input_buffer[i];
            unsigned char character_count = input_buffer[i + 1];

            // Handle our ESCAPE sequence
            if (character == ESCAPE && character_count == 0) {
                
                // Ensure we have next complete pair.
                if (i + 3 >= bytes_read) break;

                character = input_buffer[i + 2];
                character_count = ESCAPE;

                i += 2; // Skip the next pair as we've jsut processed it.
            }

            writeDecodedRun(character, character_count);
        }

    }

    // Write any remaining data in the output buffer.
    if (!output_buffer.empty()) {
        output_file.write(reinterpret_cast<const char*>(output_buffer.data()), output_buffer.size());
    }

    input_file.close();
    output_file.close();

    status_label_->setText("File decompressed successfully.");

}

void CompressionTool::HuffmanEncode(const std::string& input_file_path,
    const std::string& output_file_path) {

}

void CompressionTool::HuffmanDecode(const std::string& input_file_path,
    const std::string& output_file_path) {

}
