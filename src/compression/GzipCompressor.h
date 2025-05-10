#pragma once

#include "Compressor.h"
#include <zlib.h>

class GzipCompressor : public Compressor {
public:
    GzipCompressor() : Compressor("Gzip") {}
    CompressionResult compress(const std::vector<uint8_t>& data, int level = 6) override;
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& compressed_data) override;
    std::string getName() const override { return "Gzip"; }
    std::string getFileExtension() const override { return ".gz"; }

private:
    // Helper functions for zlib error handling
    static void checkZlibError(int ret, const char* operation);
    static std::string getZlibErrorMessage(int ret);
}; 