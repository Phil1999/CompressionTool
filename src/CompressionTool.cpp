#include "CompressionTool.h"
#include "CompressionExceptions.h"
#include <QFileInfo>
#include <QTextEdit>
#include <QThread>
#include <QMessageBox>

//TODOS
// Flesh out comments wayy more
// Write tests
// Address clang tidy issues
// ?parallel processing.
// Fix UI status label should be more intuitive.
// Need some serious refactoring
// improve performance?
// Double check error handling



CompressionTool::CompressionTool(QWidget *parent)
    : QMainWindow(parent),
    file_input_(nullptr),
    status_label_(nullptr),
    select_file_button_(nullptr),
    compress_button_(nullptr),
    decompress_button_(nullptr),
    algorithm_selector_(nullptr),
    status_bar_(nullptr),
    status_reset_timer_(new QTimer(this)),
    info_button_(nullptr),
    progress_bar_(nullptr),
    worker_(nullptr)

{

    setWindowTitle("Compression Tool");
    SetupLayout();
    // Disable resizing
    setFixedSize(QSize(WINDOW_WIDTH, WINDOW_HEIGHT));

    // Setup worker/worker thread
    SetupWorkerThread();

    status_reset_timer_->setSingleShot(true);
    connect(status_reset_timer_, &QTimer::timeout, this, &CompressionTool::ResetStatusLabel);
    
}

CompressionTool::~CompressionTool() {
    worker_thread_.quit();
    worker_thread_.wait();
}

void CompressionTool::SetupWorkerThread() {

    worker_ = new CompressionWorker();
    worker_->moveToThread(&worker_thread_);


    // Connect signals
    connect(&worker_thread_, &QThread::finished, worker_, &QObject::deleteLater);
    connect(worker_, &CompressionWorker::ProgressUpdated, this, &CompressionTool::UpdateProgress);
    connect(worker_, &CompressionWorker::completed, this, &CompressionTool::OnCompressionCompleted);
    connect(worker_, &CompressionWorker::error, this, &CompressionTool::OnCompressionError);


    worker_thread_.start();
}


void CompressionTool::SetupLayout() {
    auto* central_widget = new QWidget(this);
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

    // Progress bar
    progress_bar_ = new QProgressBar(this);

    progress_bar_->setStyleSheet(R"(
    QProgressBar {
        border: 2px solid grey;
        border-radius: 5px;
        background-color: #F0F0F0;
        text-align: center;
        color: #333;
    }
    QProgressBar::chunk {
        background-color: #0078d7;
        width: 20px;
    }
    )");
    progress_bar_->setRange(0, 100); 
    progress_bar_->setValue(0);
    progress_bar_->setVisible(false);
    main_layout->addWidget(progress_bar_);

    // Status bar
    status_bar_ = new QStatusBar(this);
    status_bar_->setSizeGripEnabled(false);
    setStatusBar(status_bar_);

    auto* status_widget = new QWidget(status_bar_);
    auto* status_layout = new QHBoxLayout(status_widget);
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
            <li><b>Run-Length Encoding (RLE):</b> A simple lossless compression algorithm that works well for files with many repeated data sequences.</li>
            <li><b>Huffman Coding:</b> An efficient lossless compression technique that assigns variable-length codes to characters based on their frequency.</li>
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
        selected_algorithm_ = CompressionWorker::AlgorithmType::RLE;
        break;

    case 1:
        selected_algorithm_ = CompressionWorker::AlgorithmType::Huffman;
        break;

    default:
        break;
    }
}

void CompressionTool::SelectFile() {

    QString file_path = QFileDialog::getOpenFileName(this, tr("Open File"), QString());

    if (!file_path.isEmpty()) {
        original_file_path_ = std::filesystem::path(file_path.toStdString());
        file_input_->setText(file_path);
    }
}

void CompressionTool::CompressFile() {
    try {
        if (original_file_path_.empty()) {
            QMessageBox::warning(this, tr("Warning"), tr("Please select a file to compress."));
            return;
        }

        std::ifstream input_file(original_file_path_, std::ios::binary);
        if (!input_file) {
            throw FileOpenException(original_file_path_.string());
        }

        // Check if the file is empty
        input_file.seekg(0, std::ios::end);
        if (input_file.tellg() == 0) {
            QMessageBox::warning(this, tr("Warning"), tr("The selected file is empty. Compression aborted."));
            return;
        }
        input_file.seekg(0, std::ios::beg);

        QString original_extension = QString::fromStdString(original_file_path_.extension().string()).toLower();
        if (original_extension == ".rle" || original_extension == ".huff") {
            QMessageBox::warning(this, tr("Warning"),
                tr("The selected file is already compressed. "
                    "Compressing it again is not recommended."));
            return;
        }

        status_label_->setText(tr("Compressing..."));

        std::string output_extension;

        switch (selected_algorithm_) {

        case CompressionWorker::AlgorithmType::RLE:
            output_extension = ".rle";
            break;

        case CompressionWorker::AlgorithmType::Huffman:
            output_extension = ".huff";
            break;

        default:
            QMessageBox::warning(this, tr("Warning"), tr("Something went wrong determing compression algorithim."));
            return;
        }

        
        // Determine output file based on the selected algorithm
        auto output_path = original_file_path_.parent_path() / (original_file_path_.stem().string() +
            output_extension);

        // Unhide progress bar and disable buttons
        progress_bar_->setValue(0);
        progress_bar_->setVisible(true);
        compress_button_->setEnabled(false);
        decompress_button_->setEnabled(false);
        algorithm_selector_->setEnabled(false);

        QMetaObject::invokeMethod(worker_, "compress", Qt::QueuedConnection,
            Q_ARG(QString, QString::fromStdString(original_file_path_.string())),
            Q_ARG(QString, QString::fromStdString(output_path.string())),
            Q_ARG(CompressionWorker::AlgorithmType, selected_algorithm_));
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, tr("Compression Error"), tr(e.what()));
        status_label_->setText(tr("Failed"));
        status_reset_timer_->start(TIMER_RESET_DURATION);
        return;
    }
}


void CompressionTool::DecompressFile() {
    try {
        if (original_file_path_.empty()) {
            QMessageBox::warning(this, tr("Warning"), tr("Please select a file to decompress."));
            return;
        }

        std::ifstream input_file(original_file_path_, std::ios::binary);
        if (!input_file) {
            throw FileOpenException(original_file_path_.string());
        }

        // Check if the file is empty
        input_file.seekg(0, std::ios::end);
        if (input_file.tellg() == 0) {
            QMessageBox::warning(this, tr("Warning"), tr("The selected file is empty. Decompression aborted."));
            return;
        }
        input_file.seekg(0, std::ios::beg);

        QString file_extension = QString::fromStdString(original_file_path_.extension().string()).toLower();

        if (file_extension != ".rle" && file_extension != ".huff") {
            QMessageBox::warning(this, tr("Warning"),
                tr("The selected file does not appear to be compressed by this tool. "
                    "Please select a .rle or .huff file for decompression."));
            return;
        }

        if ((file_extension == ".rle" && selected_algorithm_ != CompressionWorker::AlgorithmType::RLE) ||
            (file_extension == ".huff" && selected_algorithm_ != CompressionWorker::AlgorithmType::Huffman)) {
            QMessageBox::warning(this, tr("Warning"),
                tr("The selected algorithm does not match the file extension. "
                    "Please select the correct algorithm for the file type."));
            return;
        }

        status_label_->setText(tr("Decompressing..."));
  
        FileHeader header = FileHeader::read(input_file);
        
        auto output_path = original_file_path_.parent_path() / (original_file_path_.stem().string() + header.original_extension_);

        progress_bar_->setValue(0);
        progress_bar_->setVisible(true);
        compress_button_->setEnabled(false);
        decompress_button_->setEnabled(false);
        algorithm_selector_->setEnabled(false);

        QMetaObject::invokeMethod(worker_, "decompress", Qt::QueuedConnection,
            Q_ARG(QString, QString::fromStdString(original_file_path_.string())),
            Q_ARG(QString, QString::fromStdString(output_path.string())),
            Q_ARG(CompressionWorker::AlgorithmType, selected_algorithm_));

    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, tr("Decompression Error"), tr(e.what()));
        status_label_->setText(tr("Failed"));
        status_reset_timer_->start(TIMER_RESET_DURATION);
        return;
    }
}

void CompressionTool::ResetStatusLabel() {
    status_label_->setText(tr("Ready"));
}
void CompressionTool::ResetUIAfterOperation() {
    progress_bar_->setVisible(false);
    compress_button_->setEnabled(true);
    decompress_button_->setEnabled(true);
    algorithm_selector_->setEnabled(true);
}

void CompressionTool::OnCompressionCompleted() {
    status_label_->setText(tr("Success"));
    status_reset_timer_->start(TIMER_RESET_DURATION);
    ResetUIAfterOperation();
}

void CompressionTool::UpdateProgress(int percentage) {
    progress_bar_->setValue(percentage);
}

void CompressionTool::OnCompressionError(const QString& errorMessage) {
    QMessageBox::critical(this, tr("Operation Failed"), errorMessage);
    status_label_->setText(tr("Operation Failed"));
    status_reset_timer_->start(TIMER_RESET_DURATION);
    ResetUIAfterOperation();
}

