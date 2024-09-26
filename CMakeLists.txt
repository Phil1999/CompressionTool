cmake_minimum_required(VERSION 3.10)

project(CompressionTool)

# Set the vcpkg toolchain file
set(CMAKE_TOOLCHAIN_FILE "C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake")

# Find Qt and enable the MOC
find_package(Qt6 COMPONENTS Widgets REQUIRED)

# Tell CMake to enable AUTOMOC, which automatically runs moc for files containing Q_OBJECT
set(CMAKE_AUTOMOC ON)

# Add your sources
set(SOURCES
    src/main.cpp
    src/CompressionWorker.cpp 
    src/HuffmanCoding.cpp       
    src/RLECoding.cpp           
    src/FileHeader.cpp         
    src/BitReader.cpp           
    src/BitWriter.cpp         
    src/CompressionTool.cpp
)

# Define the main application executable
add_executable(CompressionTool ${SOURCES})

# Link the application with Qt
target_link_libraries(CompressionTool PRIVATE Qt6::Widgets)

# Find GoogleTest (from vcpkg)
find_package(GTest REQUIRED)

# Add the test executable
add_executable(CompressionToolTests
    tests/compression_tool_test.cpp
    src/RLECoding.cpp
    src/HuffmanCoding.cpp
    src/BitReader.cpp
    src/BitWriter.cpp
)

# Link the test executable with GTest and Qt
target_link_libraries(CompressionToolTests PRIVATE GTest::gtest_main Qt6::Widgets)

# Enable testing
enable_testing()
add_test(NAME CompressionToolTests COMMAND CompressionToolTests)
