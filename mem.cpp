#include "header.h"



// Function to get physical and virtual memory usage
void GetMemoryUsage(float &physUsedPercentage, float &swapUsedPercentage) {
    try {
        // Retrieve memory usage by parsing `free -g`
        auto [memoryInfo, swapInfo] = getMemUsage();

        long totalMemory = memoryInfo.first;  // Total RAM
        long usedMemory = memoryInfo.second;   // Used RAM
        long totalSwap = swapInfo.first;       // Total Swap
        long usedSwap = swapInfo.second;       // Used Swap

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
    }
}


// Function to get disk usage
void GetDiskUsage(float &diskUsedPercentage) {
    try {
        auto [used, total] = getDiskUsage();

        // Ensure total disk space is greater than zero to avoid division by zero
        if (total > 0) {
        
            diskUsedPercentage = (static_cast<float>(used) / static_cast<float>(total)) * 100;
        } else {
            diskUsedPercentage = 0.0f;  // Handle edge case of total disk space being 0
        }
    } catch (const std::exception &e) {
        // Handle any exceptions that occur during disk usage retrieval
        std::cerr << "Error retrieving disk usage: " << e.what() << std::endl;
        diskUsedPercentage = 0.0f;  // Set to 0 in case of error
    }
}


// Function to fetch process information concurrently
ProcessInfo FetchProcessInfo(int pid) {
    ProcessInfo process;
    process.pid = pid;

    // Fetch command line
    std::string cmdPath = "/proc/" + std::to_string(pid) + "/cmdline";
    std::ifstream cmdFile(cmdPath);
    if (cmdFile) {
        std::getline(cmdFile, process.name, '\0'); // Read until null character
    } else {
        std::cerr << "Failed to read cmdline for PID " << pid << std::endl;
        return process; // Return with empty name
    }

    // Fetch process state
    std::ifstream statFile("/proc/" + std::to_string(pid) + "/stat");
    std::string state;
    if (statFile) {
        statFile >> pid >> process.name >> state;
    } else {
        std::cerr << "Failed to read stat for PID " << pid << std::endl;
        return process; // Return with empty state
    }

    // Determine process state
    if (state == "I") {
        process.state = "(Idle)";
    } else if (state == "R") {
        process.state = "Running";
    } else {
        process.state = "Sleeping";
    }

    // Fetch CPU and memory usage
    process.cpuUsage = GetCPUUsage(pid);
    process.memoryUsage = GetMemUsage(pid);

    return process;
}

std::vector<ProcessInfo> FetchProcessList() {
    std::vector<ProcessInfo> processes;
    
    DIR *dir = opendir("/proc");
    if (!dir) {
        std::cerr << "Failed to open /proc directory." << std::endl;
        return processes; // Return an empty vector if failed
    }
    
    struct dirent *entry;
    std::vector<std::future<ProcessInfo>> futures; // To store futures for concurrent processing

    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_DIR && isdigit(entry->d_name[0])) {
            int pid = std::stoi(entry->d_name);

            // Launch the process information fetching in a separate thread
            futures.emplace_back(std::async(std::launch::async, FetchProcessInfo, pid));
        }
    }

    // Collect results from futures
    for (auto& future : futures) {
        ProcessInfo process = future.get();
        if (process.pid != 0) { // Ensure we have a valid process
            processes.push_back(process);
        }
    }

    if (closedir(dir) != 0) {
        std::cerr << "Failed to close /proc directory." << std::endl;
    }
    
    return processes;
}