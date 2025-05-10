#include "ArchiveCompressor.h"
#include <zlib.h>
#include <sstream>
#include <iomanip>
#include <chrono>

void ArchiveCompressor::checkZlibError(int ret, const char* operation) {
    if (ret != Z_OK) {
        throw std::runtime_error(std::string("zlib error during ") + operation + ": " + zError(ret));
    }
}

Compressor::CompressionResult ArchiveCompressor::compress(
    const std::vector<std::pair<std::string, std::vector<uint8_t>>>& files, int level) {
    
    CompressionResult result;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Create a tar-like header for each file
    std::vector<uint8_t> archive_data;
    size_t total_original_size = 0;
    
    for (const auto& [filename, data] : files) {
        total_original_size += data.size();
        
        // Create a simple header: filename length (4 bytes) + filename + data length (8 bytes) + data
        uint32_t name_len = filename.length();
        uint64_t data_len = data.size();
        
        // Add header
        archive_data.insert(archive_data.end(), 
            reinterpret_cast<uint8_t*>(&name_len),
            reinterpret_cast<uint8_t*>(&name_len) + sizeof(name_len));
        
        archive_data.insert(archive_data.end(), 
            reinterpret_cast<const uint8_t*>(filename.c_str()),
            reinterpret_cast<const uint8_t*>(filename.c_str()) + name_len);
        
        archive_data.insert(archive_data.end(),
            reinterpret_cast<uint8_t*>(&data_len),
            reinterpret_cast<uint8_t*>(&data_len) + sizeof(data_len));
        
        // Add file data
        archive_data.insert(archive_data.end(), data.begin(), data.end());
    }
    
    // Compress the archive using zlib
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    
    int ret = deflateInit2(&strm, level, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);
    checkZlibError(ret, "deflateInit2");
    
    strm.avail_in = archive_data.size();
    strm.next_in = archive_data.data();
    
    size_t max_output_size = deflateBound(&strm, archive_data.size());
    std::vector<uint8_t> compressed_data(max_output_size);
    
    strm.avail_out = max_output_size;
    strm.next_out = compressed_data.data();
    
    ret = deflate(&strm, Z_FINISH);
    if (ret != Z_STREAM_END) {
        deflateEnd(&strm);
        checkZlibError(ret, "deflate");
    }
    
    compressed_data.resize(max_output_size - strm.avail_out);
    deflateEnd(&strm);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    result.compression_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    // Measure decompression time
    start_time = std::chrono::high_resolution_clock::now();
    auto decompressed = decompressArchive(compressed_data);
    end_time = std::chrono::high_resolution_clock::now();
    result.decompression_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    // Calculate compression ratio
    result.compression_ratio = static_cast<double>(compressed_data.size()) / total_original_size;
    
    return result;
}

std::map<std::string, std::vector<uint8_t>> ArchiveCompressor::decompressArchive(
    const std::vector<uint8_t>& compressed_data) {
    
    // First decompress the data
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    
    int ret = inflateInit2(&strm, 15 + 16);
    checkZlibError(ret, "inflateInit2");
    
    strm.avail_in = compressed_data.size();
    strm.next_in = const_cast<Bytef*>(compressed_data.data());
    
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
    
    // Now extract the files from the decompressed data
    std::map<std::string, std::vector<uint8_t>> files;
    size_t pos = 0;
    
    while (pos < decompressed_data.size()) {
        // Read filename length
        uint32_t name_len;
        std::memcpy(&name_len, &decompressed_data[pos], sizeof(name_len));
        pos += sizeof(name_len);
        
        // Read filename
        std::string filename(reinterpret_cast<char*>(&decompressed_data[pos]), name_len);
        pos += name_len;
        
        // Read data length
        uint64_t data_len;
        std::memcpy(&data_len, &decompressed_data[pos], sizeof(data_len));
        pos += sizeof(data_len);
        
        // Read file data
        std::vector<uint8_t> file_data(&decompressed_data[pos], &decompressed_data[pos] + data_len);
        pos += data_len;
        
        files[filename] = std::move(file_data);
    }
    
    return files;
} 