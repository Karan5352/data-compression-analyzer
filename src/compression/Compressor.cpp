#include "Compressor.h"
#include <sys/resource.h>

size_t Compressor::getCurrentMemoryUsage() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss; // Return in bytes
} 