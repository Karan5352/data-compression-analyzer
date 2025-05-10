# Data Compression Analyzer

A modern GUI application for analyzing and comparing compression algorithms, built with Dear ImGui and C++17.

## Features

- Support for multiple compression algorithms:
  - Gzip (with configurable compression levels)
  - Archive+Gzip (for compressing multiple files into a single archive)
- Modern GUI built with Dear ImGui
- Comprehensive performance metrics:
  - Compression ratio
  - Compression time
  - Decompression time
  - Memory usage
- Multi-file selection and processing
- Export results in CSV or JSON format
- Cross-platform support (Windows, macOS, Linux)

## Requirements

- C++17 or later
- CMake 3.15 or later
- Required libraries:
  - Dear ImGui
  - zlib (for Gzip compression)
  - GLFW3
  - OpenGL

## Building the Project

```bash
# Clone the repository
git clone https://github.com/yourusername/data-compression-analyzer.git
cd data-compression-analyzer

# Create and enter build directory
mkdir build && cd build

# Configure and build
cmake ..
make

# Run the application
./DataCompressionAnalyzer
```

## Usage

1. Launch the application
2. Click "Open Files" to select one or more files for analysis
3. Choose the compression algorithm:
   - Gzip: Compresses individual files
   - Archive+Gzip: Compresses multiple files into a single archive
4. Adjust compression level (1-9) for both algorithms
5. Click "Start Analysis" to begin compression
6. View results in the interactive interface:
   - Summary statistics
   - Detailed results table
7. Export results in CSV or JSON format

## Project Structure

```
.
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── gui/
│   │   ├── MainWindow.cpp
│   │   └── MainWindow.h
│   ├── compression/
│   │   ├── Compressor.cpp
│   │   ├── Compressor.h
│   │   ├── GzipCompressor.cpp
│   │   ├── GzipCompressor.h
│   │   ├── ArchiveCompressor.cpp
│   │   └── ArchiveCompressor.h
│   └── utils/
│       ├── FileHandler.cpp
│       └── FileHandler.h
├── LICENSE
└── README.md
```

## Features in Detail

### Gzip Compression
- Configurable compression levels (1-9)
- Individual file compression
- Detailed performance metrics

### Archive+Gzip Compression
- Combines multiple files into a single archive
- Uses Gzip compression internally
- Configurable compression levels
- Preserves file structure and names

### Results Analysis
- Compression ratio calculation
- Compression and decompression timing
- Memory usage tracking
- Export functionality for further analysis

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request. For major changes, please open an issue first to discuss what you would like to change.