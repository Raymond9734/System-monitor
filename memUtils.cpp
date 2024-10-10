#include "header.h"
extern int TOTAL_PROCESSES;

// Utility function to split a string by spaces
std::vector<std::string> split(const std::string &s) {
    std::istringstream iss(s);
    return std::vector<std::string>((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
}
std::pair<std::pair<long, long>, std::pair<long, long>> getMemUsage() {
    std::array<char, 128> buffer;
    std::string result;

    // Use FILE* for pclose and provide correct type to unique_ptr
    std::unique_ptr<FILE, int(*)(FILE*)> pipe(popen("free -g", "r"), pclose);

    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    std::istringstream iss(result);
    std::string line;
    std::string line2;
    long totalMemory = 0;
    long usedMemory = 0;
    long totalSwap = 0;
    long usedSwap = 0;

    // Skip the first line which is the header
    std::getline(iss, line);
    // The second line contains the memory information (RAM)
    std::getline(iss, line);
    // The third line contains the swap information
    std::getline(iss, line2);

    std::istringstream memoryLine(line);
    std::istringstream swapLine(line2);
    std::string temp;

    // Parse the second line for memory (RAM) values
    memoryLine >> temp >> totalMemory >> usedMemory;
    
    // Parse the third line for swap values
    swapLine >> temp >> totalSwap >> usedSwap;

    // Return a pair of pairs: {{totalMemory, usedMemory}, {totalSwap, usedSwap}}
    return {{totalMemory, usedMemory}, {totalSwap, usedSwap}};
}
std::pair<long, long> getDiskUsage()
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

  return {usedStorage, totalStorage};
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