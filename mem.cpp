#include "header.h"



// Function to get physical and virtual memory usage
// Parameters:
//   physUsedPercentage: Output parameter for physical memory usage percentage
//   swapUsedPercentage: Output parameter for swap memory usage percentage 
//   totalMemoryStr: Output parameter for total physical memory as formatted string
//   usedMemoryStr: Output parameter for used physical memory as formatted string
//   totalSwapStr: Output parameter for total swap memory as formatted string
//   usedSwapStr: Output parameter for used swap memory as formatted string
void GetMemoryUsage(float &physUsedPercentage, float &swapUsedPercentage, 
                    std::string &totalMemoryStr, std::string &usedMemoryStr, 
                    std::string &totalSwapStr, std::string &usedSwapStr) {
    try {
        // Get memory info using system call wrapper to retrieve physical and swap memory stats
        const auto [memoryInfo, swapInfo] = getMemUsage();

        // Extract memory values from the returned pairs
        // memoryInfo contains physical memory stats (total and used)
        // swapInfo contains swap memory stats (total and used)
        const auto [totalMemory, totalMemoryStr_] = memoryInfo.first;     
        const auto [usedMemory, usedMemoryStr_] = memoryInfo.second;
        const auto [totalSwap, totalSwapStr_] = swapInfo.first;
        const auto [usedSwap, usedSwapStr_] = swapInfo.second;

        // Update the output string parameters with the formatted memory values
        totalMemoryStr = totalMemoryStr_;
        usedMemoryStr = usedMemoryStr_;
        totalSwapStr = totalSwapStr_;
        usedSwapStr = usedSwapStr_;

        // Calculate physical memory usage percentage, avoiding division by zero
        physUsedPercentage = totalMemory > 0 
            ? (static_cast<float>(usedMemory) * 100.0f / totalMemory)
            : 0.0f;

        // Calculate swap memory usage percentage, avoiding division by zero  
        swapUsedPercentage = totalSwap > 0
            ? (static_cast<float>(usedSwap) * 100.0f / totalSwap) 
            : 0.0f;

    } catch (const std::exception &e) {
        // On error, log the exception and set all output parameters to safe defaults
        std::cerr << "Error retrieving memory usage: " << e.what() << std::endl;
        physUsedPercentage = swapUsedPercentage = 0.0f;
        totalMemoryStr = usedMemoryStr = totalSwapStr = usedSwapStr = "N/A";
    }
}

// Function to get disk usage information
// Parameters:
//   diskUsedPercentage: Output parameter for disk usage percentage
//   usedStorageStr: Output parameter for used storage space as formatted string
//   totalStorageStr: Output parameter for total storage space as formatted string
void GetDiskUsage(float &diskUsedPercentage, std::string &usedStorageStr, std::string &totalStorageStr) {
    try {
        // Get disk usage info using system call wrapper
        // storageNum contains numeric values (used, total) in bytes
        // storageStr contains formatted strings (used, total)
        const auto [storageNum, storageStr] = getDiskUsage();

        // Calculate disk usage percentage
        // Use float division and avoid division by zero
        diskUsedPercentage = storageNum.second > 0 
            ? (static_cast<float>(storageNum.first) / storageNum.second) * 100.0f
            : 0.0f;

        // Copy formatted strings to output parameters
        usedStorageStr = storageStr.first;
        totalStorageStr = storageStr.second;
    }
    catch (const std::exception &e) {
        // Log error and set default values on failure
        std::cerr << "Error retrieving disk usage: " << e.what() << std::endl;
        diskUsedPercentage = 0.0f;
        usedStorageStr = totalStorageStr = "N/A";
    }
}

// Function to fetch detailed information about a specific process
// Parameters:
//   pid: Process ID to fetch information for
// Returns:
//   ProcessInfo struct containing process details like name, state, CPU/memory usage
ProcessInfo FetchProcessInfo(int pid) {
    // Initialize ProcessInfo struct with default values
    ProcessInfo process;
    process.pid = pid;
    process.isActive = false;  // Process is considered inactive until proven otherwise
    process.name = "Unknown";  // Default name if we can't determine the real name
    process.state = "Unknown"; // Default state if we can't determine the real state
    process.cpuUsage = 0.0f;  // Default CPU usage
    process.memoryUsage = 0.0f; // Default memory usage

    try {
        // Validate PID
        if (pid <= 0) {
            throw std::runtime_error("Invalid PID");
        }

        // Try to get process name from cmdline file
        // This usually contains the full command line used to start the process
        std::string cmdPath = "/proc/" + std::to_string(pid) + "/cmdline";
        std::ifstream cmdFile(cmdPath);
        std::string cmdLine;
        if (cmdFile && std::getline(cmdFile, cmdLine, '\0')) {
            if (!cmdLine.empty()) {
                // Extract just the executable name without the full path
                size_t lastSlash = cmdLine.find_last_of('/');
                if (lastSlash != std::string::npos) {
                    process.name = cmdLine.substr(lastSlash + 1);
                } else {
                    process.name = cmdLine;
                }
            }
        }

        // Get process state and fallback name from stat file
        // Format: pid (name) state ...
        std::string statPath = "/proc/" + std::to_string(pid) + "/stat";
        std::ifstream statFile(statPath);
        std::string line;
        
        if (statFile && std::getline(statFile, line)) {
            size_t firstParen = line.find('(');
            size_t lastParen = line.rfind(')');
            
            // Ensure we found valid parentheses and have content after them
            if (firstParen != std::string::npos && lastParen != std::string::npos && 
                firstParen < lastParen && lastParen + 2 < line.length()) {
                
                // If we couldn't get name from cmdline, use the one from stat
                if (process.name == "Unknown") {
                    process.name = line.substr(firstParen + 1, lastParen - firstParen - 1);
                }
                
                // Parse process state character (R:running, S:sleeping, etc)
                std::istringstream iss(line.substr(lastParen + 2)); // Skip ') '
                std::string state;
                if (iss >> state) {
                    process.state = state;
                }
            }
        }

        // Get CPU and memory usage statistics
        float cpuUsage = GetCPUUsage(pid);
        if (cpuUsage >= 0.0f) {  // Negative value indicates error
            process.cpuUsage = cpuUsage;
            process.memoryUsage = GetMemUsage(pid);
            process.isActive = true;  // Mark process as active if we got valid CPU usage
        }

    } catch (const std::exception& e) {
        // Log any errors but continue - return process with default values
        std::cerr << "Error fetching process info for PID " << pid << ": " << e.what() << std::endl;
    }

    return process;
}
/**
 * Continuously fetches process information from the system in a multi-threaded way.
 * This function runs in an infinite loop, periodically scanning /proc directory
 * for active processes and gathering their information.
 */
void StartFetchingProcesses() {
    // Counter for tracking number of threads currently running
    std::atomic<int> activeThreads(0);
    
    // Vector to store process IDs found in /proc
    std::vector<int> pids;
    pids.reserve(1000); // Pre-allocate reasonable initial capacity for efficiency

    while (true) {
        pids.clear(); // Reuse vector instead of recreating to avoid memory allocation

        // Open /proc directory using raw pointer for faster directory access
        DIR* dir = opendir("/proc");
        if (!dir) {
            std::cerr << "Failed to open /proc directory." << std::endl;
            // Brief sleep before retry if directory open fails
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // Scan directory entries for process directories (numbered directories)
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            // Check if entry is a directory and name is a number (valid PID)
            if (entry->d_type == DT_DIR && isNumber(entry->d_name)) {
                pids.push_back(std::stoi(entry->d_name));
            }
        }
        closedir(dir);

        // Process PIDs in batches to limit concurrent threads
        const int BATCH_SIZE = 50;  // Maximum number of concurrent threads per batch
        for (size_t i = 0; i < pids.size(); i += BATCH_SIZE) {
            // Calculate end index for current batch
            size_t end = std::min(i + BATCH_SIZE, pids.size());
            
            // Launch a new thread for each PID in the batch
            for (size_t j = i; j < end; j++) {
                activeThreads++;
                // Create detached thread to process individual PID
                std::thread([pid = pids[j], &activeThreads]() {
                    try {
                        // Fetch process info and add to global queue
                        ProcessInfo process = FetchProcessInfo(pid);
                        g_completedProcesses.push(std::move(process));
                    } catch (...) { /* Silently ignore errors to prevent thread crashes */ }
                    activeThreads--;  // Decrease active thread count when done
                }).detach();
            }
        }

        // Wait before starting next scan cycle
        // Shorter sleep time since GetCPUUsage already includes a 3s delay
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }
}
