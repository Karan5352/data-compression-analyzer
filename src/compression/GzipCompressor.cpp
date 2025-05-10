#include "GzipCompressor.h"
#include <stdexcept>
#include <chrono>

Compressor::CompressionResult GzipCompressor::compress(const std::vector<uint8_t>& data, int level) {
    Compressor::CompressionResult result;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Initialize zlib stream
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    
    // Initialize deflate with the specified compression level
    int ret = deflateInit2(&strm, level, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);
    checkZlibError(ret, "deflateInit2");
    
    // Set up input and output buffers
    strm.avail_in = data.size();
    strm.next_in = const_cast<Bytef*>(data.data());
    
    // Calculate maximum output size
    size_t max_output_size = deflateBound(&strm, data.size());
    std::vector<uint8_t> compressed_data(max_output_size);
    
    // Compress the data
    strm.avail_out = max_output_size;
    strm.next_out = compressed_data.data();
    
    ret = deflate(&strm, Z_FINISH);
    if (ret != Z_STREAM_END) {
        deflateEnd(&strm);
        checkZlibError(ret, "deflate");
    }
    
    // Resize the output buffer to the actual compressed size
    compressed_data.resize(max_output_size - strm.avail_out);
    
    // Clean up
    deflateEnd(&strm);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    result.compression_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    // Measure decompression time
    start_time = std::chrono::high_resolution_clock::now();
    auto decompressed = decompress(compressed_data);
    end_time = std::chrono::high_resolution_clock::now();
    result.decompression_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    // Calculate compression ratio
    result.compression_ratio = static_cast<double>(compressed_data.size()) / data.size();
    
    // Get memory usage
    result.memory_used = getCurrentMemoryUsage();
    
    return result;
}

std::vector<uint8_t> GzipCompressor::decompress(const std::vector<uint8_t>& compressed_data) {
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = compressed_data.size();
    strm.next_in = const_cast<Bytef*>(compressed_data.data());
    
    int ret = inflateInit2(&strm, 15 + 16);
    checkZlibError(ret, "inflateInit2");
    
    std::vector<uint8_t> decompressed_data;
    uint8_t buffer[4096];
    
    do {
        strm.avail_out = sizeof(buffer);
        strm.next_out = buffer;
        
        ret = inflate(&strm, Z_NO_FLUSH);
        if (ret != Z_STREAM_END && ret != Z_OK) {
            inflateEnd(&strm);
            checkZlibError(ret, "inflate");
        }
        
        size_t bytes_decompressed = sizeof(buffer) - strm.avail_out;
        decompressed_data.insert(decompressed_data.end(), buffer, buffer + bytes_decompressed);
    } while (ret != Z_STREAM_END);
    
    inflateEnd(&strm);
    return decompressed_data;
}

void GzipCompressor::checkZlibError(int ret, const char* operation) {
    if (ret != Z_OK) {
        throw std::runtime_error(std::string("zlib error during ") + operation + ": " + zError(ret));
    }
}

std::string GzipCompressor::getZlibErrorMessage(int ret) {
    switch (ret) {
        case Z_ERRNO:
            return "Error reading/writing file";
        case Z_STREAM_ERROR:
            return "Invalid compression level";
        case Z_DATA_ERROR:
            return "Invalid or incomplete deflate data";
        case Z_MEM_ERROR:
            return "Out of memory";
        case Z_VERSION_ERROR:
            return "zlib version mismatch";
        default:
            return "Unknown error";
    }
} 