#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <optional>

class FileHandler {
public:
    // Read file contents into a byte vector
    static std::vector<uint8_t> readFile(const std::filesystem::path& file_path);
    
    // Write data to a file
    static void writeFile(const std::filesystem::path& file_path, const std::vector<uint8_t>& data);
    
    // Get file size
    static size_t getFileSize(const std::filesystem::path& file_path);
    
    // Check if file exists
    static bool fileExists(const std::filesystem::path& file_path);
    
    // Get file extension
    static std::string getFileExtension(const std::filesystem::path& file_path);
    
    // Get file name without extension
    static std::string getFileNameWithoutExtension(const std::filesystem::path& file_path);
    
    // Create a unique output file path
    static std::filesystem::path createOutputPath(const std::filesystem::path& input_path,
                                                const std::string& suffix);
    
    // Export results to CSV
    static void exportToCSV(const std::filesystem::path& output_path,
                           const std::vector<std::string>& headers,
                           const std::vector<std::vector<std::string>>& rows);
    
    // Export results to JSON
    static void exportToJSON(const std::filesystem::path& output_path,
                            const std::vector<std::string>& headers,
                            const std::vector<std::vector<std::string>>& rows);
}; 