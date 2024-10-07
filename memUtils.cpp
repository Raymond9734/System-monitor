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

float GetCPUUsage(int pid) {
    std::string statPath = "/proc/" + std::to_string(pid) + "/stat";
    std::ifstream statFile(statPath);

    if (!statFile.is_open()) {
        std::cerr << "Error: Unable to open " << statPath << std::endl;
        return -1.0;
    }

    std::string statLine;
    std::getline(statFile, statLine);
    statFile.close();

    std::istringstream iss(statLine);
    std::vector<std::string> statFields;
    std::string field;

    while (iss >> field) {
        statFields.push_back(field);
    }

    if (statFields.size() < 22) {
        std::cerr << "Error: Unexpected format in " << statPath << std::endl;
        return -1.0;
    }

    unsigned long long utime, stime;
    try {
        utime = std::stoull(statFields[13]);
        stime = std::stoull(statFields[14]);
    } catch (const std::exception& e) {
        std::cerr << "Error: Unable to parse utime and stime" << std::endl;
        return -1.0;
    }

    unsigned long long total_time = utime + stime;
    long hertz = sysconf(_SC_CLK_TCK);
    double total_time_sec = static_cast<double>(total_time) / hertz;

    double uptime;
    std::ifstream uptimeFile("/proc/uptime");
    if (!(uptimeFile >> uptime)) {
        std::cerr << "Error: Unable to read system uptime" << std::endl;
        return -1.0;
    }
    uptimeFile.close();

    double starttime;
    try {
        starttime = std::stod(statFields[21]) / hertz;
    } catch (const std::exception& e) {
        std::cerr << "Error: Unable to parse process start time" << std::endl;
        return 0.0;
    }

    double seconds = uptime - starttime;
    double cpu_usage = 100.0 * (total_time_sec / seconds);
    // TOTAL_PROCESSES++;

    return cpu_usage;
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