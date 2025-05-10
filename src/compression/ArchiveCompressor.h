#pragma once

#include "Compressor.h"
#include <map>
#include <string>

class ArchiveCompressor : public Compressor {
public:
    ArchiveCompressor() : Compressor("Archive+Gzip") {}
    
    // Compress multiple files into a single archive
    CompressionResult compress(const std::vector<std::pair<std::string, std::vector<uint8_t>>>& files, int level = 6);
    
    // Override single file compression to throw an error
    CompressionResult compress(const std::vector<uint8_t>& data, int level = 6) override {
        throw std::runtime_error("ArchiveCompressor does not support single file compression");
    }
    
    // Decompress the archive and return a map of filenames to their contents
    std::map<std::string, std::vector<uint8_t>> decompressArchive(const std::vector<uint8_t>& compressed_data);
    
    // Override single file decompression to throw an error
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& compressed_data) override {
        throw std::runtime_error("ArchiveCompressor does not support single file decompression");
    }
    
    std::string getName() const override { return "Archive+Gzip"; }
    std::string getFileExtension() const override { return ".tar.gz"; }

private:
    // Helper functions for zlib error handling
    static void checkZlibError(int ret, const char* operation);
}; 