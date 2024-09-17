#include "CompressionTool.h"

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


void CompressionTool::CompressFile() {
   
}

void CompressionTool::DecompressFile() {

}


void CompressionTool::RunLengthEncode(const std::string& input_file_path,
    const std::string& output_file_path) {

}

void CompressionTool::RunLengthDecode(const std::string& input_file_path,
    const std::string& output_file_path) {

}

void CompressionTool::HuffmanEncode(const std::string& input_file_path,
    const std::string& output_file_path) {

}

void CompressionTool::HuffmanDecode(const std::string& input_file_path,
    const std::string& output_file_path) {

}
