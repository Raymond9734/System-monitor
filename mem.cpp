#include "header.h"



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
    } else if (state == "S") {
        process.state = "Sleeping";
    } else if (state == "D") {
        process.state = "USleep";
    } else if (state == "T") {
        process.state = "Stopped";
    } else if (state == "Z") {
        process.state = "Zombie";
    } else if (state == "X") {
        process.state = "Dead";
    } else {
        process.state = "Unknown State";
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