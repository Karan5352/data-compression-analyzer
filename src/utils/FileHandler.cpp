#include "FileHandler.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::vector<uint8_t> FileHandler::readFile(const std::filesystem::path& file_path) {
    if (!fileExists(file_path)) {
        throw std::runtime_error("File does not exist: " + file_path.string());
    }

    std::ifstream file(file_path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + file_path.string());
    }

    // Get file size
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Read file contents
    std::vector<uint8_t> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size);
    
    if (!file) {
        throw std::runtime_error("Failed to read file: " + file_path.string());
    }

    return data;
}

void FileHandler::writeFile(const std::filesystem::path& file_path, const std::vector<uint8_t>& data) {
    std::ofstream file(file_path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to create file: " + file_path.string());
    }

    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    if (!file) {
        throw std::runtime_error("Failed to write to file: " + file_path.string());
    }
}

size_t FileHandler::getFileSize(const std::filesystem::path& file_path) {
    if (!fileExists(file_path)) {
        throw std::runtime_error("File does not exist: " + file_path.string());
    }
    return std::filesystem::file_size(file_path);
}

bool FileHandler::fileExists(const std::filesystem::path& file_path) {
    return std::filesystem::exists(file_path);
}

std::string FileHandler::getFileExtension(const std::filesystem::path& file_path) {
    return file_path.extension().string();
}

std::string FileHandler::getFileNameWithoutExtension(const std::filesystem::path& file_path) {
    return file_path.stem().string();
}

std::filesystem::path FileHandler::createOutputPath(const std::filesystem::path& input_path,
                                                  const std::string& suffix) {
    std::filesystem::path output_path = input_path;
    output_path.replace_extension(suffix);
    
    // If file already exists, append a number
    int counter = 1;
    while (fileExists(output_path)) {
        output_path = input_path;
        output_path.replace_extension(suffix + std::to_string(counter++));
    }
    
    return output_path;
}

void FileHandler::exportToCSV(const std::filesystem::path& output_path,
                             const std::vector<std::string>& headers,
                             const std::vector<std::vector<std::string>>& rows) {
    std::ofstream file(output_path);
    if (!file) {
        throw std::runtime_error("Failed to create CSV file: " + output_path.string());
    }

    // Write headers
    for (size_t i = 0; i < headers.size(); ++i) {
        file << headers[i];
        if (i < headers.size() - 1) {
            file << ",";
        }
    }
    file << "\n";

    // Write rows
    for (const auto& row : rows) {
        for (size_t i = 0; i < row.size(); ++i) {
            file << row[i];
            if (i < row.size() - 1) {
                file << ",";
            }
        }
        file << "\n";
    }
}

void FileHandler::exportToJSON(const std::filesystem::path& output_path,
                              const std::vector<std::string>& headers,
                              const std::vector<std::vector<std::string>>& rows) {
    json j;
    j["headers"] = headers;
    
    json data = json::array();
    for (const auto& row : rows) {
        json row_obj;
        for (size_t i = 0; i < headers.size() && i < row.size(); ++i) {
            row_obj[headers[i]] = row[i];
        }
        data.push_back(row_obj);
    }
    j["data"] = data;

    std::ofstream file(output_path);
    if (!file) {
        throw std::runtime_error("Failed to create JSON file: " + output_path.string());
    }
    file << std::setw(4) << j << std::endl;
}

std::string FileHandler::detectFileType(const std::vector<uint8_t>& data) {
    if (data.size() < 4) return "Unknown";
    
    // Check for common file signatures
    if (data[0] == 0x89 && data[1] == 'P' && data[2] == 'N' && data[3] == 'G') return "PNG";
    if (data[0] == 0xFF && data[1] == 0xD8 && data[2] == 0xFF) return "JPEG";
    if (data[0] == 'P' && data[1] == 'K' && data[2] == 0x03 && data[3] == 0x04) return "ZIP";
    if (data[0] == 0x1F && data[1] == 0x8B) return "GZIP";
    if (data[0] == 0x37 && data[1] == 0x7A && data[2] == 0xBC && data[3] == 0xAF) return "7ZIP";
    if (data[0] == 0x42 && data[1] == 0x5A && data[2] == 0x68) return "BZIP2";
    if (data[0] == 0x28 && data[1] == 0xB5 && data[2] == 0x2F && data[3] == 0xFD) return "ZSTD";
    
    // Check for text files
    bool is_text = true;
    for (size_t i = 0; i < std::min(data.size(), size_t(1024)); i++) {
        if (data[i] == 0) {
            is_text = false;
            break;
        }
    }
    if (is_text) return "Text";
    
    return "Binary";
}

double FileHandler::calculateEntropy(const std::vector<uint8_t>& data) {
    if (data.empty()) return 0.0;
    
    // Count byte frequencies
    std::map<uint8_t, size_t> frequencies;
    for (uint8_t byte : data) {
        frequencies[byte]++;
    }
    
    // Calculate entropy
    double entropy = 0.0;
    double size = static_cast<double>(data.size());
    for (const auto& [byte, count] : frequencies) {
        double probability = static_cast<double>(count) / size;
        entropy -= probability * std::log2(probability);
    }
    
    return entropy;
}

double FileHandler::calculateThroughput(size_t bytes, long long microseconds) {
    if (microseconds == 0) return 0.0;
    return (bytes * 1000000.0) / (microseconds * 1024.0 * 1024.0); // Convert to MB/s
} 