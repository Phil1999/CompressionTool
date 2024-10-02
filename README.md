# CompressionTool

CompressionTool is a file compression and decompression utility that supports two lossless algorithms: **Run-Length Encoding (RLE)** and **Huffman Coding**. The tool is built using C++ with a Qt-based GUI.

![App GUI](https://github.com/user-attachments/assets/98d91bd7-2147-45a5-a196-dbb0cbd9531d)

## Features

- **Run-Length Encoding (RLE)**: Simple compression algorithm ideal for files with long sequences of repeated data.
- **Huffman Coding**: More complex, frequency-based compression algorithm for efficient storage.
- **GUI**: Built with Qt for an intuitive user experience with a responsive progress bar indicating operation status and operation progress.
- **Multithreading**: Multithreaded application separating UI and business logic on different threads to ensure responsive and efficient processing.
- **GoogleTest Integration**: Unit tests to ensure the functionality of key functionalities.

## Requirements

- **C++17 or later**
- **Qt6** (Widgets)
- **vcpkg** (for package management)
- **gtest** (for testing)

## Building the Project


### Using CMake from Command Line

1.  Install dependencies via **vcpkg**:
    
    ```bash
    vcpkg install qtbase[core,widgets,gui] gtest
    ```
2.  Clone the repository:
    
``` bash
    git clone https://github.com/Phil1999/CompressionTool.git
    cd CompressionTool 
```
3.  Configure the project with **CMake**:
    
    
     `cmake -B build DCMAKE_TOOLCHAIN_FILE=""
`
    
4.  Build the project:
    
    ```bash
    cmake --build build
    ```

## Running the Application

After building, run the `CompressionTool` executable located in the `build` directory:

```bash
./build/CompressionTool 
```

## Running Unit Tests

The tests are built as a separate executable (`CompressionToolTests`). To run the tests:

```bash
./build/CompressionToolTests
```


