#include "header.h"
/**
 * Formats a byte count into a human-readable string with appropriate units
 * Uses SI units (B, KB, MB, GB) and shows 2 decimal places
 * Optimized to use constexpr array and minimize divisions
 * 
 * @param bytes The number of bytes to format
 * @return Formatted string with units (e.g. "1.23 MB")
 */
std::string formatBytes(long long bytes) {
    // Define units array as constexpr for compile-time optimization
    static constexpr const char* units[] = {"B", "KB", "MB", "GB"};
    static constexpr double SCALE = 1024.0;
    
    // Start with bytes and track which unit we're using
    std::size_t unitIndex = 0; // Changed from int to std::size_t to match array index type
    double size = static_cast<double>(bytes);

    // Scale down by 1024 until we hit an appropriate unit or reach max unit
    while (size >= SCALE && unitIndex < std::size(units) - 1) {
        size /= SCALE;
        unitIndex++;
    }

    // Pre-allocate buffer with ample space for number + unit
    char buffer[32];
    // Format with 2 decimal places for readability
    snprintf(buffer, sizeof(buffer), "%.2f %s", size, units[unitIndex]);
    return std::string(buffer);
}
