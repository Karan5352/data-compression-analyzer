#include "MainWindow.h"
#include "../compression/GzipCompressor.h"
#include "../compression/ArchiveCompressor.h"
#include "../utils/FileHandler.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <algorithm>
#include <thread>
#include <future>
#include <iostream>
#include <tinyfiledialogs.h>

#define GL_SILENCE_DEPRECATION

MainWindow::MainWindow() {
    initWindow();
    initImGui();
    initCompressors();
}

MainWindow::~MainWindow() {
    cleanupImGui();
    cleanup();
}

void MainWindow::initWindow() {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window_ = glfwCreateWindow(1600, 900, "Data Compression Analyzer", nullptr, nullptr);
    if (!window_) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);
}

void MainWindow::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void MainWindow::initCompressors() {
    compressors_.push_back(std::make_unique<GzipCompressor>());
    compressors_.push_back(std::make_unique<ArchiveCompressor>());
}

void MainWindow::run() {
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        render();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window_, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window_);
    }
}

void MainWindow::render() {
    ImGui::Begin("Data Compression Analyzer", nullptr, ImGuiWindowFlags_NoCollapse);

    renderFileSelection();
    renderCompressionOptions();
    renderResults();
    renderExportOptions();

    ImGui::End();
}

void MainWindow::renderFileSelection() {
    if (ImGui::CollapsingHeader("File Selection", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button("Open Files")) {
            openFileDialog();
        }

        ImGui::SameLine();
        ImGui::Text("Selected Files: %zu", selected_files_.size());

        if (!selected_files_.empty()) {
            ImGui::BeginChild("FileList", ImVec2(0, 200), true);
            for (size_t i = 0; i < selected_files_.size(); i++) {
                ImGui::PushID(i);
                ImGui::Text("%s", selected_files_[i].filename().string().c_str());
                ImGui::SameLine();
                if (ImGui::Button("Remove")) {
                    selected_files_.erase(selected_files_.begin() + i);
                    i--; // Adjust index after removal
                }
                ImGui::PopID();
            }
            ImGui::EndChild();
        }
    }
}

void MainWindow::renderCompressionOptions() {
    if (ImGui::CollapsingHeader("Compression Options", ImGuiTreeNodeFlags_DefaultOpen)) {
        static int selected_compressor = 0;
        static int gzip_level = 6;
        static bool archive_mode = false;

        if (ImGui::BeginCombo("Compressor", compressors_[selected_compressor]->getName().c_str())) {
            for (int i = 0; i < compressors_.size(); i++) {
                if (ImGui::Selectable(compressors_[i]->getName().c_str(), selected_compressor == i)) {
                    selected_compressor = i;
                    archive_mode = (i == 1); // Enable archive mode for ArchiveCompressor
                }
            }
            ImGui::EndCombo();
        }

        // Show Gzip level slider for both Gzip and Archive compressors
        if (selected_compressor == 0 || selected_compressor == 1) {
            ImGui::SliderInt("Gzip Compression Level", &gzip_level, 1, 9);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Higher levels provide better compression but are slower");
            }
        }

        if (ImGui::Button("Start Analysis") && !selected_files_.empty() && !is_processing_) {
            is_processing_ = true;
            int current_compressor = selected_compressor;
            int current_level = gzip_level;
            bool current_archive_mode = archive_mode;
            std::thread([this, current_compressor, current_level, current_archive_mode]() {
                processFiles(current_compressor, current_level, current_archive_mode);
                is_processing_ = false;
            }).detach();
        }
        if (is_processing_) {
            ImGui::SameLine();
            ImGui::Text("Processing...");
        }
    }
}

void MainWindow::renderResults() {
    if (ImGui::CollapsingHeader("Results", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (results_.empty()) {
            ImGui::Text("No results available. Run analysis first.");
            return;
        }
        // Add summary statistics
        if (ImGui::TreeNode("Summary Statistics")) {
            double avg_ratio = 0.0;
            double avg_entropy = 0.0;
            double avg_comp_time_ms = 0.0;
            double avg_decomp_time_ms = 0.0;
            double avg_comp_throughput = 0.0;
            double avg_decomp_throughput = 0.0;
            size_t total_original_size = 0;
            size_t total_compressed_size = 0;
            
            for (const auto& result : results_) {
                avg_ratio += result.ratio;
                avg_entropy += result.entropy;
                avg_comp_time_ms += result.compression_time_us / 1000.0;
                avg_decomp_time_ms += result.decompression_time_us / 1000.0;
                avg_comp_throughput += result.compression_throughput;
                avg_decomp_throughput += result.decompression_throughput;
                total_original_size += result.original_size;
                total_compressed_size += result.compressed_size;
            }
            size_t count = results_.size();
            avg_ratio /= count;
            avg_entropy /= count;
            avg_comp_time_ms /= count;
            avg_decomp_time_ms /= count;
            avg_comp_throughput /= count;
            avg_decomp_throughput /= count;
            
            ImGui::Text("Average Compression Ratio: %.2f", avg_ratio);
            ImGui::Text("Average Entropy: %.2f bits/byte", avg_entropy);
            ImGui::Text("Average Compression Time: %.3f ms", avg_comp_time_ms);
            ImGui::Text("Average Decompression Time: %.3f ms", avg_decomp_time_ms);
            ImGui::Text("Average Compression Speed: %.2f MB/s", avg_comp_throughput);
            ImGui::Text("Average Decompression Speed: %.2f MB/s", avg_decomp_throughput);
            ImGui::Text("Total Original Size: %.2f KB", total_original_size / 1024.0);
            ImGui::Text("Total Compressed Size: %.2f KB", total_compressed_size / 1024.0);
            ImGui::Text("Total Space Saved: %.2f KB (%.1f%%)", 
                (total_original_size - total_compressed_size) / 1024.0,
                (1.0 - static_cast<double>(total_compressed_size) / total_original_size) * 100.0);
            ImGui::TreePop();
        }
        // Detailed results table
        if (ImGui::BeginTable("ResultsTable", 10, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("File");
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Algorithm");
            ImGui::TableSetupColumn("Ratio");
            ImGui::TableSetupColumn("Entropy");
            ImGui::TableSetupColumn("Original Size (KB)");
            ImGui::TableSetupColumn("Compressed Size (KB)");
            ImGui::TableSetupColumn("Compression Speed (MB/s)");
            ImGui::TableSetupColumn("Decompression Speed (MB/s)");
            ImGui::TableSetupColumn("Time (ms)");
            ImGui::TableHeadersRow();
            for (const auto& result : results_) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", result.filename.c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%s", result.file_type.c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%s", result.algorithm.c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%.2f", result.ratio);
                ImGui::TableNextColumn();
                ImGui::Text("%.2f", result.entropy);
                ImGui::TableNextColumn();
                ImGui::Text("%.2f", result.original_size / 1024.0);
                ImGui::TableNextColumn();
                ImGui::Text("%.2f", result.compressed_size / 1024.0);
                ImGui::TableNextColumn();
                ImGui::Text("%.2f", result.compression_throughput);
                ImGui::TableNextColumn();
                ImGui::Text("%.2f", result.decompression_throughput);
                ImGui::TableNextColumn();
                ImGui::Text("%.3f", (result.compression_time_us + result.decompression_time_us) / 1000.0);
            }
            ImGui::EndTable();
        }
    }
}

void MainWindow::renderExportOptions() {
    if (ImGui::CollapsingHeader("Export Options", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button("Export as CSV")) {
            exportResults("results.csv", false);
        }
        ImGui::SameLine();
        if (ImGui::Button("Export as JSON")) {
            exportResults("results.json", true);
        }
    }
}

void MainWindow::openFileDialog() {
    const char* filters[] = { "*" };
    const char* default_path = "";
    const char* file_paths = tinyfd_openFileDialog(
        "Select Files",
        default_path,
        0,
        filters,
        "All Files",
        1
    );
    if (file_paths) {
        std::string paths(file_paths);
        size_t pos = 0;
        std::string token;
        while ((pos = paths.find("|")) != std::string::npos) {
            token = paths.substr(0, pos);
            selected_files_.push_back(token);
            paths.erase(0, pos + 1);
        }
        selected_files_.push_back(paths);
    }
}

void MainWindow::processFiles(int selected_compressor, int gzip_level, bool archive_mode) {
    results_.clear();
    
    if (archive_mode && selected_compressor == 1) { // Archive mode
        try {
            std::vector<std::pair<std::string, std::vector<uint8_t>>> files;
            size_t total_original_size = 0;
            for (const auto& file_path : selected_files_) {
                auto file_data = FileHandler::readFile(file_path);
                total_original_size += file_data.size();
                files.emplace_back(file_path.filename().string(), std::move(file_data));
            }
            
            auto result = static_cast<ArchiveCompressor*>(compressors_[selected_compressor].get())
                ->compress(files, gzip_level);
            
            CompressionResult ui_result;
            ui_result.filename = "Archive (" + std::to_string(selected_files_.size()) + " files)";
            ui_result.algorithm = compressors_[selected_compressor]->getName() + " (Level " + std::to_string(gzip_level) + ")";
            ui_result.file_type = "Archive";
            ui_result.ratio = result.compression_ratio;
            ui_result.entropy = FileHandler::calculateEntropy(files[0].second); // Calculate entropy of first file
            ui_result.compression_time_us = result.compression_time.count();
            ui_result.decompression_time_us = result.decompression_time.count();
            ui_result.compression_throughput = FileHandler::calculateThroughput(total_original_size, result.compression_time.count());
            ui_result.decompression_throughput = FileHandler::calculateThroughput(total_original_size, result.decompression_time.count());
            ui_result.original_size = total_original_size;
            ui_result.compressed_size = static_cast<size_t>(total_original_size * result.compression_ratio);
            results_.push_back(ui_result);
        } catch (const std::exception& e) {
            showError("Error processing archive: " + std::string(e.what()));
        }
    } else { // Individual file mode
        for (const auto& file_path : selected_files_) {
            try {
                auto file_data = FileHandler::readFile(file_path);
                auto result = compressors_[selected_compressor]->compress(file_data, gzip_level);
                CompressionResult ui_result;
                ui_result.filename = file_path.filename().string();
                ui_result.algorithm = compressors_[selected_compressor]->getName() + 
                                    (selected_compressor == 0 ? " (Level " + std::to_string(gzip_level) + ")" : "");
                ui_result.file_type = FileHandler::detectFileType(file_data);
                ui_result.ratio = result.compression_ratio;
                ui_result.entropy = FileHandler::calculateEntropy(file_data);
                ui_result.compression_time_us = result.compression_time.count();
                ui_result.decompression_time_us = result.decompression_time.count();
                ui_result.compression_throughput = FileHandler::calculateThroughput(file_data.size(), result.compression_time.count());
                ui_result.decompression_throughput = FileHandler::calculateThroughput(file_data.size(), result.decompression_time.count());
                ui_result.original_size = file_data.size();
                ui_result.compressed_size = static_cast<size_t>(file_data.size() * result.compression_ratio);
                results_.push_back(ui_result);
            } catch (const std::exception& e) {
                showError("Error processing file " + file_path.string() + ": " + e.what());
            }
        }
    }
}

void MainWindow::exportResults(const std::string& default_filename, bool as_json) {
    const char* filters[] = { as_json ? "*.json" : "*.csv" };
    const char* default_path = default_filename.c_str();
    const char* save_path = tinyfd_saveFileDialog(
        "Save Results",
        default_path,
        1,
        filters,
        as_json ? "JSON Files" : "CSV Files"
    );
    if (save_path) {
        try {
            std::vector<std::string> headers = {
                "File", "Type", "Algorithm", "Ratio", "Entropy", "Original Size (KB)", "Compressed Size (KB)",
                "Compression Speed (MB/s)", "Decompression Speed (MB/s)", "Time (ms)"
            };
            std::vector<std::vector<std::string>> rows;
            for (const auto& result : results_) {
                rows.push_back({
                    result.filename,
                    result.file_type,
                    result.algorithm,
                    std::to_string(result.ratio),
                    std::to_string(result.entropy),
                    std::to_string(result.original_size / 1024.0),
                    std::to_string(result.compressed_size / 1024.0),
                    std::to_string(result.compression_throughput),
                    std::to_string(result.decompression_throughput),
                    std::to_string((result.compression_time_us + result.decompression_time_us) / 1000.0)
                });
            }
            if (as_json) {
                FileHandler::exportToJSON(save_path, headers, rows);
            } else {
                FileHandler::exportToCSV(save_path, headers, rows);
            }
            showSuccess("Results exported successfully to " + std::string(save_path));
        } catch (const std::exception& e) {
            showError("Failed to export results: " + std::string(e.what()));
        }
    }
}

void MainWindow::showError(const std::string& message) {
    // TODO: Implement proper error display
    std::cerr << "Error: " << message << std::endl;
}

void MainWindow::showSuccess(const std::string& message) {
    // TODO: Implement proper success display
    std::cout << "Success: " << message << std::endl;
}

void MainWindow::cleanupImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void MainWindow::cleanup() {
    glfwDestroyWindow(window_);
    glfwTerminate();
} 