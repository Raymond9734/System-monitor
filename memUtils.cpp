#include "header.h"

/**
 * Helper function to check if a string contains only digits.
 * Uses efficient std::all_of algorithm with no memory allocation.
 * @param str The string to check
 * @return true if string contains only digits, false otherwise
 */
bool isNumber(const std::string& str) {
    return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
}

/**
 * Utility function to split a string by whitespace.
 * Uses string stream for efficient tokenization.
 * @param s The input string to split
 * @return Vector containing the whitespace-separated tokens
 */
std::vector<std::string> split(const std::string& s) {
    std::istringstream iss(s);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) {
        tokens.push_back(std::move(token));
    }
    return tokens;
}

/**
 * Converts human-readable memory units to bytes.
 * Handles Ki (KiB), Mi (MiB), and Gi (GiB) units.
 * Uses constexpr multipliers for compile-time optimization.
 * @param value String containing number and unit (e.g. "5Gi")
 * @return Number of bytes
 */
long convertToBytes(const std::string& value) {
    // Extract the numeric part by removing the last character (unit)
    long number = std::stol(value.substr(0, value.size() - 1));
    
    // Extract the unit part (last 2 characters)
    std::string unit = value.substr(value.size() - 2);
    
    // Convert based on unit using binary (IEC) prefixes
    if (unit == "Gi") {
        return number * 1024 * 1024 * 1024;  // GiB: 1 GiB = 1024^3 bytes
    } else if (unit == "Mi") {
        return number * 1024 * 1024;  // MiB: 1 MiB = 1024^2 bytes 
    } else if (unit == "Ki") {
        return number * 1024;  // KiB: 1 KiB = 1024 bytes
    } else {
        return number;  // No unit or unrecognized unit - return raw number
    }
}
/**
 * Formats memory size strings by converting IEC units (Ki,Mi,Gi) to SI units (KB,MB,GB)
 * Optimized to minimize string allocations and comparisons
 * 
 * @param value Input string containing number and IEC unit (e.g. "5Gi")
 * @return Formatted string with SI unit (e.g. "5GB")
 */
std::string Format(const std::string& value) {
    // Early return if string is too short to have a unit
    if (value.size() < 2) {
        return value;
    }

    // Extract number and unit parts with single substr call
    const auto len = value.size();
    const auto unit = value.substr(len - 2);
    const auto number = value.substr(0, len - 2);

    // Use string_view for unit comparisons to avoid allocations
    static const std::string_view gi{"Gi"};
    static const std::string_view mi{"Mi"};
    static const std::string_view ki{"Ki"};

    // Reserve capacity for result string
    std::string result;
    result.reserve(number.size() + 2);
    result = number;

    // Use if-else chain ordered by most common case first
    if (unit == ki) {
        result += "KB";
    } else if (unit == mi) {
        result += "MB"; 
    } else if (unit == gi) {
        result += "GB";
    }

    return result;
}

// Function to return memory and swap usage as both numeric values and formatted strings
// Returns nested pairs containing:
// - Memory info: {{total_bytes, total_str}, {used_bytes, used_str}}
// - Swap info: {{total_bytes, total_str}, {used_bytes, used_str}} 
std::pair<std::pair<std::pair<long, std::string>, std::pair<long, std::string>>, 
          std::pair<std::pair<long, std::string>, std::pair<long, std::string>>> getMemUsage() {
    // Buffer for reading command output
    std::array<char, 128> buffer;
    std::string result;

    // Execute 'free -h' command to get memory usage info
    // Use FILE* with unique_ptr for automatic cleanup
    std::unique_ptr<FILE, int(*)(FILE*)> pipe(popen("free -h", "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    // Read entire command output into result string
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    // Set up string streams and variables for parsing
    std::istringstream iss(result);
    std::string line;
    std::string memoryLine;
    std::string swapLine;
    std::string totalMemoryStr, usedMemoryStr;
    std::string totalSwapStr, usedSwapStr;

    // Parse the command output line by line
    std::getline(iss, line);        // Skip header line
    std::getline(iss, memoryLine);  // Get memory info line
    std::getline(iss, swapLine);    // Get swap info line

    // Create streams for parsing memory and swap lines
    std::istringstream memStream(memoryLine);
    std::istringstream swapStream(swapLine);
    std::string temp;

    // Extract memory values (skipping 'Mem:' label)
    memStream >> temp >> totalMemoryStr >> usedMemoryStr;
    
    // Extract swap values (skipping 'Swap:' label)
    swapStream >> temp >> totalSwapStr >> usedSwapStr;

    // Convert human-readable values to bytes
    long totalMemory = convertToBytes(totalMemoryStr);
    long usedMemory = convertToBytes(usedMemoryStr);
    long totalSwap = convertToBytes(totalSwapStr);
    long usedSwap = convertToBytes(usedSwapStr);

    // Format the strings to use SI units (KB/MB/GB)
    std::string newTotalMemoryStr = Format(totalMemoryStr);
    std::string newUsedMemoryStr = Format(usedMemoryStr);
    std::string newTotalSwapStr = Format(totalSwapStr);
    std::string newUsedSwapStr = Format(usedSwapStr);

    // Return nested pairs containing both numeric and string representations
    // Structure: {{memory_total, memory_total_str}, {memory_used, memory_used_str}},
    //           {{swap_total, swap_total_str}, {swap_used, swap_used_str}}
    return {{{totalMemory, newTotalMemoryStr}, {usedMemory, newUsedMemoryStr}}, 
            {{totalSwap, newTotalSwapStr}, {usedSwap, newUsedSwapStr}}};
}
/**
 * Gets disk usage information for the root filesystem
 * Uses 'df' command to get disk space information
 * 
 * @return Pair containing:
 *   - First pair: Numeric values for used and total storage in bytes
 *   - Second pair: Human readable strings for used and total storage
 */
std::pair< std::pair<long, long>, std::pair<std::string, std::string> > getDiskUsage()
{
  // Buffer for reading command output
  std::array<char, 128> buffer;
  std::string result;
  
  // Execute df command to get disk usage info
  std::unique_ptr<FILE, int(*)(FILE*)> pipe(popen("df -h /", "r"), pclose);
  if (!pipe)
  {
    throw std::runtime_error("popen() failed!");
  }
  
  // Read command output into result string
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
  {
    result += buffer.data();
  }

  std::istringstream iss(result);
  std::string line;

  // Skip the first line which is the header
  std::getline(iss, line);

  // The second line contains the disk usage information
  std::getline(iss, line);

  // Parse disk usage line into components
  std::istringstream diskLine(line);
  std::string filesystem, size, used, available, usagePercent;
  diskLine >> filesystem >> size >> used >> available >> usagePercent;

  // Convert used and available strings to long
  long usedStorage = std::stol(used);
  long totalStorage = std::stol(size);

  // Return numeric values and formatted strings
  return {{usedStorage, totalStorage},{used+"B", size+"B"}};
}

/**
 * Gets memory usage percentage for a specific process
 * Reads from /proc/<pid>/status file to get resident memory usage
 * 
 * @param pid Process ID to check memory usage for
 * @return Memory usage as percentage of total system memory
 */
float GetMemUsage(int pid){
    // Open process status file
    std::string path = "/proc/" + std::to_string(pid) + "/status";
    std::ifstream statusFile(path);
    std::string line;
    float memUsage = 0.0f;

    // Look for VmRSS line which shows resident memory usage
    while(std::getline(statusFile,line)){
        if (line.find("VmRSS:") == 0) {
            std::istringstream iss(line);
            std::string key;
            int value;
            std::string unit;
            iss >> key >> value >> unit;
            // Calculate percentage of total system memory
            memUsage = static_cast<float>(value) / (sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGE_SIZE) / 1024) * 100;
            break;
        }
    }
    return memUsage;
}

/**
 * Gets CPU usage percentage for a specific process
 * Takes two measurements of process and system CPU time to calculate usage
 * 
 * @param pid Process ID to check CPU usage for
 * @return CPU usage percentage or -1.0 on error
 */
float GetCPUUsage(int pid) {
    std::string pidStr = std::to_string(pid);
    std::string statPath = "/proc/" + pidStr + "/stat";

    // Helper lambda to measure process and system CPU time
    auto measure = [&]() -> std::pair<long, long> {
        // Open process stat file
        std::ifstream statFile(statPath);
        if (!statFile.is_open()) {
            // std::cerr << "Could not open file: " << statPath << std::endl;
            return {-1, -1};
        }

        // Read process stats
        std::string statLine;
        if (!std::getline(statFile, statLine)) {
            std::cerr << "Failed to read from file: " << statPath << std::endl;
            return {-1, -1};
        }

        // Parse stat line into fields
        std::istringstream iss(statLine);
        std::vector<std::string> statFields;
        std::string field;

        while (iss >> field) {
            statFields.push_back(field);
        }

        // Check we have enough fields
        if (statFields.size() < 15) {
            std::cerr << "Insufficient data in stat file for PID: " << pid << std::endl;
            return {-1, -1};
        }

        // Get process CPU times
        long utime = std::stol(statFields[13]);  // user mode time
        long stime = std::stol(statFields[14]);  // system mode time
        long total_time = utime + stime;

        // Get system-wide CPU times
        std::ifstream cpuFile("/proc/stat");
        if (!cpuFile.is_open()) {
            std::cerr << "Could not open /proc/stat" << std::endl;
            return {-1, -1};
        }

        std::string cpuLine;
        if (!std::getline(cpuFile, cpuLine)) {
            std::cerr << "Failed to read from /proc/stat" << std::endl;
            return {-1, -1};
        }

        // Parse CPU times from stat file
        std::istringstream cpuStream(cpuLine);
        std::string cpuLabel;
        long user, nice, system, idle, iowait, irq, softirq, steal;
        if (!(cpuStream >> cpuLabel >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal)) {
            std::cerr << "Failed to parse CPU data from /proc/stat" << std::endl;
            return {-1, -1};
        }
        long total_system_time = user + nice + system + idle + iowait + irq + softirq + steal;

        return {total_time, total_system_time};
    };

    // Take first measurement
    auto [total_time1, system_time1] = measure();
    if (total_time1 < 0 || system_time1 < 0) return -1.0;

    // Wait 3 seconds between measurements
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));  // 1-second interval

    // Take second measurement
    auto [total_time2, system_time2] = measure();
    if (total_time2 < 0 || system_time2 < 0) return -1.0;

    // Get system constants
    long hertz = sysconf(_SC_CLK_TCK);
    if (hertz <= 0) {
        std::cerr << "Invalid clock ticks per second" << std::endl;
        return -1.0;
    }

    long numCores = sysconf(_SC_NPROCESSORS_ONLN);
    if (numCores <= 0) {
        std::cerr << "Invalid number of CPU cores" << std::endl;
        return -1.0;
    }

    // Calculate time differences
    float total_time_diff = static_cast<float>(total_time2 - total_time1) ;
    float system_time_diff = static_cast<float>(system_time2 - system_time1) ;

    if (system_time_diff <= 0) {
        std::cerr << "Invalid system time difference" << std::endl;
        return -1.0;
    }

    // Calculate CPU usage percentage (total_time_diff is the process time, system_time_diff is overall CPU time)
    float cpuUsage = 100.0 * (total_time_diff / system_time_diff);

    // Multiply by the number of cores to match top's calculation
    cpuUsage *= numCores;

    return cpuUsage;
}
