#pragma once

#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "../compression/Compressor.h"
#include "tinyfiledialogs.h"

class MainWindow {
public:
    MainWindow();
    ~MainWindow();

    void run();

private:
    // Window management
    GLFWwindow* window_;
    void initWindow();
    void cleanup();
    
    // ImGui setup
    void initImGui();
    void cleanupImGui();
    
    // Main rendering loop
    void render();
    void renderFileSelection();
    void renderCompressionOptions();
    void renderResults();
    void renderExportOptions();
    
    // File handling
    void openFileDialog();
    void processFiles(int selected_compressor, int gzip_level, bool archive_mode);
    
    // Compression handling
    std::vector<std::unique_ptr<Compressor>> compressors_;
    void initCompressors();
    
    // Results storage
    struct CompressionResult {
        std::string filename;
        std::string algorithm;
        std::string file_type;
        double ratio;
        double entropy;
        long long compression_time_us;
        long long decompression_time_us;
        double compression_throughput;  // MB/s
        double decompression_throughput;  // MB/s
        size_t memory_used;
        size_t original_size;    // Size in bytes
        size_t compressed_size;  // Size in bytes
    };
    std::vector<CompressionResult> results_;
    
    // UI state
    std::vector<std::filesystem::path> selected_files_;
    bool show_compression_options_ = true;
    bool show_results_ = false;
    bool is_processing_ = false;
    
    // Export options
    void exportResults(const std::string& default_filename, bool as_json);
    
    // Helper functions
    void showError(const std::string& message);
    void showSuccess(const std::string& message);
}; 