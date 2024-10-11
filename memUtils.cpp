#include "header.h"
extern int TOTAL_PROCESSES;

// Utility function to split a string by spaces
std::vector<std::string> split(const std::string &s) {
    std::istringstream iss(s);
    return std::vector<std::string>((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
}
// Function to convert human-readable memory units to bytes (KB, MB, GB)
long convertToBytes(const std::string& value) {
    long number = std::stol(value.substr(0, value.size() - 1));
    std::string unit = value.substr(value.size() - 2);
    
    if (unit == "Gi") {
        return number * 1024 * 1024 * 1024;  // Convert GB to bytes
    } else if (unit == "Mi") {
        return number * 1024 * 1024;  // Convert MB to bytes
    } else if (unit == "Ki") {
        return number * 1024;  // Convert KB to bytes
    } else {
        return number;  // No recognized unit, return the number as is
    }
}
std::string Format(const std::string& value) {
    std::string upperV = value.substr(0, value.size() - 2);
    std::string unit = value.substr(value.size() - 2);
    
     if (unit == "Gi") {
       return upperV + "GB";   // Convert GB to bytes
    } else if (unit == "Mi") {
        return upperV + "MB";   // Convert MB to bytes
    } else if (unit == "Ki") {
        return upperV + "KB"; // Convert KB to bytes
    } else {
        return upperV;  // No recognized unit, return the number as is
    }
  
}

// Function to return memory and swap usage as numeric values and strings
std::pair<std::pair<std::pair<long, std::string>, std::pair<long, std::string>>, 
          std::pair<std::pair<long, std::string>, std::pair<long, std::string>>> getMemUsage() {
    std::array<char, 128> buffer;
    std::string result;

    // Use FILE* for pclose and provide correct type to unique_ptr
    std::unique_ptr<FILE, int(*)(FILE*)> pipe(popen("free -h", "r"), pclose);

    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    std::istringstream iss(result);
    std::string line;
    std::string memoryLine;
    std::string swapLine;
    std::string totalMemoryStr, usedMemoryStr;
    std::string totalSwapStr, usedSwapStr;

    // Skip the first line which is the header
    std::getline(iss, line);
    // The second line contains the memory information (RAM)
    std::getline(iss, memoryLine);
    // The third line contains the swap information
    std::getline(iss, swapLine);

    std::istringstream memStream(memoryLine);
    std::istringstream swapStream(swapLine);
    std::string temp;

    // Parse the second line for memory (RAM) values
    memStream >> temp >> totalMemoryStr >> usedMemoryStr;
    
    // Parse the third line for swap values
    swapStream >> temp >> totalSwapStr >> usedSwapStr;

    // Convert human-readable values to bytes
    long totalMemory = convertToBytes(totalMemoryStr);
    long usedMemory = convertToBytes(usedMemoryStr);
    long totalSwap = convertToBytes(totalSwapStr);
    long usedSwap = convertToBytes(usedSwapStr);

    std::string newTotalMemoryStr = Format(totalMemoryStr);
    std::string newUsedMemoryStr = Format(usedMemoryStr);
    std::string newTotalSwapStr = Format(totalSwapStr);
    std::string newUsedSwapStr = Format(usedSwapStr);

    // Return a pair of pairs: {{totalMemory, usedMemory}, {totalSwap, usedSwap}} in both numeric and string format
    return {{{totalMemory, newTotalMemoryStr}, {usedMemory, newUsedMemoryStr}}, 
            {{totalSwap, newTotalSwapStr}, {usedSwap, newUsedSwapStr}}};
}
std::pair< std::pair<long, long>, std::pair<std::string, std::string> > getDiskUsage()
{
  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, int(*)(FILE*)> pipe(popen("df -h /", "r"), pclose);
  if (!pipe)
  {
    throw std::runtime_error("popen() failed!");
  }
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

  std::istringstream diskLine(line);
  std::string filesystem, size, used, available, usagePercent;
  diskLine >> filesystem >> size >> used >> available >> usagePercent;

  // Convert used and available strings to long
  long usedStorage = std::stol(used);
  long totalStorage = std::stol(size);

  return {{usedStorage, totalStorage},{used+"B", size+"B"}};
}

float GetMemUsage(int pid){
    std::string path = "/proc/" + std::to_string(pid) + "/status";
    std::ifstream statusFile(path);
    std::string line;
    float memUsage = 0.0f;

    while(std::getline(statusFile,line)){
        if (line.find("VmRSS:") == 0) {
            std::istringstream iss(line);
            std::string key;
            int value;
            std::string unit;
            iss >> key >> value >> unit;
            memUsage = static_cast<float>(value) / (sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGE_SIZE) / 1024) * 100;
            break;
        }
    }
    return memUsage;
}
// Function to get CPU usage for a specific process
float GetCPUUsage(int pid) {
    std::string pidStr = std::to_string(pid);
    std::string statPath = "/proc/" + pidStr + "/stat";

    // Take two measurements with a delay
    auto measure = [&]() -> std::pair<long, float> {
        std::ifstream statFile(statPath);
        if (!statFile.is_open()) {
            std::cerr << "Could not open file: " << statPath << std::endl;
            return {-1, -1.0};
        }

        std::string statLine;
        std::getline(statFile, statLine);
        std::istringstream iss(statLine);
        std::vector<std::string> statFields;
        std::string field;

        while (iss >> field) {
            statFields.push_back(field);
        }

        long utime = std::stol(statFields[13]);
        long stime = std::stol(statFields[14]);
        long total_time = utime + stime;

        float uptime;
        std::ifstream uptimeFile("/proc/uptime");
        if (uptimeFile.is_open()) {
            uptimeFile >> uptime;
        } else {
            std::cerr << "Could not open /proc/uptime" << std::endl;
            return {-1, -1.0};
        }

        return {total_time, uptime};
    };

    auto [total_time1, uptime1] = measure();
    if (total_time1 < 0) return -1.0;

    std::this_thread::sleep_for(std::chrono::milliseconds(3000));  

    auto [total_time2, uptime2] = measure();
    if (total_time2 < 0) return -1.0;

    long hertz = sysconf(_SC_CLK_TCK);
    
    float total_time_diff = static_cast<float>(total_time2 - total_time1) / hertz;
    float uptime_diff = uptime2 - uptime1;

    // Calculate CPU usage percentage
    float cpuUsage = 100.0 * (total_time_diff / uptime_diff);

    return cpuUsage;
}