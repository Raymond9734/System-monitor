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
    }
}

// Function to fetch process information concurrently
ProcessInfo FetchProcessInfo(int pid) {
    ProcessInfo process;
    process.pid = pid;
    process.isActive = false;  // Set to false by default

    try {
        // Fetch command line
        std::string cmdPath = "/proc/" + std::to_string(pid) + "/cmdline";
        std::ifstream cmdFile(cmdPath);
        if (cmdFile) {
            std::getline(cmdFile, process.name, '\0'); // Read until null character
        } else {
            throw std::runtime_error("Failed to read cmdline for PID " + std::to_string(pid));
        }

        // Fetch process state
        std::ifstream statFile("/proc/" + std::to_string(pid) + "/stat");
        std::string state;
        if (statFile) {
            statFile >> pid >> process.name >> state;
        } else {
            throw std::runtime_error("Failed to read stat for PID " + std::to_string(pid));
        }

        // Determine process state
        process.state = (state == "R") ? "Running" :
                        (state == "S") ? "Sleeping" :
                        (state == "D") ? "Uninterruptible Sleep" :
                        (state == "T") ? "Stopped" :
                        (state == "t") ? "Tracing Stop" :
                        (state == "Z") ? "Zombie" : "Unknown State";

        // Fetch CPU usage
        process.cpuUsage = GetCPUUsage(pid);
        if (process.cpuUsage < 0) {
            process.name = "Unknown";
            process.state = "Error";
            process.cpuUsage = 0.0f;
            process.memoryUsage = 0.0f;
            process.isActive = false;
            return process;
        }
        process.memoryUsage = GetMemUsage(pid); // Assuming GetMemUsage is defined elsewhere

        // If we've made it this far without exceptions, the process is active
        process.isActive = true;
    } catch (const std::exception& e) {
        // Log the error but don't throw, to prevent UI crashes
        std::cerr << "Error fetching process info for PID " << pid << ": " << e.what() << std::endl;
        // Set default values for the process info
        process.name = "Unknown";
        process.state = "Error";
        process.cpuUsage = 0.0f;
        process.memoryUsage = 0.0f;
        process.isActive = false;
    }

    return process;
}
void StartFetchingProcesses() {
    while (true) { // Infinite loop to keep fetching processes
        DIR* dir = opendir("/proc");
        if (!dir) {
            std::cerr << "Failed to open /proc directory." << std::endl;
            return;
        }

        struct dirent* entry;

        while ((entry = readdir(dir)) != nullptr) {
            std::string dirName(entry->d_name);
            if (entry->d_type == DT_DIR && isNumber(dirName)) {
                int pid = std::stoi(dirName);
                // Launch thread and detach immediately so it can send results independently
                std::thread([pid]() {
                    try {
                        ProcessInfo process = FetchProcessInfo(pid);
                        g_completedProcesses.push(std::move(process));
                    } catch (const std::exception& e) {
                        std::cerr << "Error processing PID " << pid << ": " << e.what() << std::endl;
                    }
                }).detach();
            }
        }

        closedir(dir);
        std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // Delay for 3 seconds
    }
}
