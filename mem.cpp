#include "header.h"
#include <queue>
#include <condition_variable>
#include <chrono>
#include <thread>


// Function to get physical and virtual memory usage
void GetMemoryUsage(float &physUsedPercentage, float &swapUsedPercentage, 
                    std::string &totalMemoryStr, std::string &usedMemoryStr, 
                    std::string &totalSwapStr, std::string &usedSwapStr) {
    try {
        // Retrieve memory usage, which now returns both numeric and string formats
        auto [memoryInfo, swapInfo] = getMemUsage();

        // Extract the numeric and string values for memory (RAM)
        long totalMemory = memoryInfo.first.first;     // Total RAM in KiB
        long usedMemory = memoryInfo.second.first;     // Used RAM in KiB
        totalMemoryStr = memoryInfo.first.second;      // Total RAM as string (e.g., "16G")
        usedMemoryStr = memoryInfo.second.second;      // Used RAM as string (e.g., "4G")

        // Extract the numeric and string values for swap
        long totalSwap = swapInfo.first.first;         // Total Swap in KiB
        long usedSwap = swapInfo.second.first;         // Used Swap in KiB
        totalSwapStr = swapInfo.first.second;          // Total Swap as string (e.g., "2G")
        usedSwapStr = swapInfo.second.second;          // Used Swap as string (e.g., "1G")

        // Calculate the percentage of used physical memory (RAM)
        if (totalMemory > 0) {
            physUsedPercentage = static_cast<float>(usedMemory) * 100.0f / static_cast<float>(totalMemory);
        } else {
            physUsedPercentage = 0.0f;  // Handle edge case of total RAM being 0
        }

        // Calculate the percentage of used swap memory
        if (totalSwap > 0) {
            swapUsedPercentage = static_cast<float>(usedSwap) * 100.0f / static_cast<float>(totalSwap);
        } else {
            swapUsedPercentage = 0.0f;  // Handle case where there's no swap space
        }

    } catch (const std::exception &e) {
        // Handle any exceptions that occur during memory usage retrieval
        std::cerr << "Error retrieving memory usage: " << e.what() << std::endl;
        physUsedPercentage = 0.0f;
        swapUsedPercentage = 0.0f;
        totalMemoryStr = usedMemoryStr = totalSwapStr = usedSwapStr = "N/A";
    }
}


// Function to get disk usage
void GetDiskUsage(float &diskUsedPercentage,std::string &usedStorageStr,std::string &totalStorageStr) {
    try {
        auto [storageNum, storageStr] = getDiskUsage();

        // Ensure total disk space is greater than zero to avoid division by zero
        if (storageNum.second > 0) {
            diskUsedPercentage = (static_cast<float>(storageNum.first) / static_cast<float>(storageNum.second)) * 100;
        } else {
            diskUsedPercentage = 0.0f;  // Handle edge case of total disk space being 0
        }
        usedStorageStr = storageStr.first;
        totalStorageStr = storageStr.second;
    }
    catch (const std::exception &e)
    {
        // Handle any exceptions that occur during disk usage retrieval
        std::cerr << "Error retrieving disk usage: " << e.what() << std::endl;
        diskUsedPercentage = 0.0f;  // Set to 0 in case of error
        usedStorageStr = "N/A";
        totalStorageStr = "N/A";
    }
}

// Function to fetch process information concurrently
ProcessInfo FetchProcessInfo(int pid) {
    ProcessInfo process;
    process.pid = pid;
    process.isActive = false;  // Set to false by default
    process.name = "Unknown";  // Initialize with default values
    process.state = "Unknown";
    process.cpuUsage = 0.0f;
    process.memoryUsage = 0.0f;

    try {
        if (pid <= 0) {
            throw std::runtime_error("Invalid PID");
        }

        // Fetch command line
        std::string cmdPath = "/proc/" + std::to_string(pid) + "/cmdline";
        std::ifstream cmdFile(cmdPath);
        std::string cmdLine;
        if (cmdFile && std::getline(cmdFile, cmdLine, '\0')) {
            if (!cmdLine.empty()) {
                // Extract just the command name without path
                size_t lastSlash = cmdLine.find_last_of('/');
                if (lastSlash != std::string::npos) {
                    process.name = cmdLine.substr(lastSlash + 1);
                } else {
                    process.name = cmdLine;
                }
            }
        }

        // Fetch process state and name from stat file
        std::string statPath = "/proc/" + std::to_string(pid) + "/stat";
        std::ifstream statFile(statPath);
        std::string line;
        
        if (statFile && std::getline(statFile, line)) {
            size_t firstParen = line.find('(');
            size_t lastParen = line.rfind(')');
            
            if (firstParen != std::string::npos && lastParen != std::string::npos && 
                firstParen < lastParen && lastParen + 2 < line.length()) {
                
                // Extract name between parentheses if cmdline was empty
                if (process.name == "Unknown") {
                    process.name = line.substr(firstParen + 1, lastParen - firstParen - 1);
                }
                
                // Extract state which comes after the closing parenthesis
                std::istringstream iss(line.substr(lastParen + 2)); // +2 to skip ') '
                std::string state;
                if (iss >> state) {
                    process.state = state;
                }
            }
        }

        // Fetch CPU and memory usage
        float cpuUsage = GetCPUUsage(pid);
        if (cpuUsage >= 0.0f) {
            process.cpuUsage = cpuUsage;
            process.memoryUsage = GetMemUsage(pid);
            process.isActive = true;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error fetching process info for PID " << pid << ": " << e.what() << std::endl;
    }

    return process;
}
void StartFetchingProcesses() {
    std::atomic<int> activeThreads(0);
    std::vector<int> pids;
    pids.reserve(1000); // Pre-allocate reasonable initial capacity

    while (true) {
        pids.clear(); // Reuse vector instead of recreating

        // Use raw pointer for faster directory access
        DIR* dir = opendir("/proc");
        if (!dir) {
            std::cerr << "Failed to open /proc directory." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Reduced sleep time
            continue;
        }

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type == DT_DIR && isNumber(entry->d_name)) {
                pids.push_back(std::stoi(entry->d_name));
            }
        }
        closedir(dir);

        // Process PIDs in batches
        const int BATCH_SIZE = 50;
        for (size_t i = 0; i < pids.size(); i += BATCH_SIZE) {
            size_t end = std::min(i + BATCH_SIZE, pids.size());
            
            // Launch batch of threads
            for (size_t j = i; j < end; j++) {
                activeThreads++;
                std::thread([pid = pids[j], &activeThreads]() {
                    try {
                        ProcessInfo process = FetchProcessInfo(pid);
                        g_completedProcesses.push(std::move(process));
                    } catch (...) { /* Silently ignore errors */ }
                    activeThreads--;
                }).detach();
            }

        }

        // Only sleep for minimal time since GetCPUUsage already has 3s sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }
}
