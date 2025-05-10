#pragma once

#include <string>
#include <vector>
#include <memory>
#include <chrono>

class Compressor {
public:
    struct CompressionResult {
        double compression_ratio;
        std::chrono::microseconds compression_time;
        std::chrono::microseconds decompression_time;
        size_t memory_used;
    };

    explicit Compressor(const std::string& name) : name_(name) {}
    virtual ~Compressor() = default;

    virtual CompressionResult compress(const std::vector<uint8_t>& data, int level = 6) = 0;
    virtual std::vector<uint8_t> decompress(const std::vector<uint8_t>& compressed_data) = 0;
    virtual std::string getName() const { return name_; }
    virtual std::string getFileExtension() const = 0;

protected:
    std::string name_;
    size_t getCurrentMemoryUsage();
}; 