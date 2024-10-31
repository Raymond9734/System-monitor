// To make sure you don't declare the function more than once by including the header multiple times.
#ifndef header_H
#define header_H

//------------------------------------------------------------------------------
// ImGui Headers                                    // GUI framework and rendering
//------------------------------------------------------------------------------
#include "imgui/lib/imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

//------------------------------------------------------------------------------
// Standard Library Headers                         // Core C++ functionality
//------------------------------------------------------------------------------
#include <stdio.h>                                 // Input/output operations
#include <iostream>                                // Console I/O streams
#include <string>                                  // String handling
#include <vector>                                  // Dynamic arrays
#include <array>                                   // Fixed-size arrays
#include <map>                                     // Ordered key-value pairs
#include <unordered_map>                          // Hash table implementation
#include <queue>                                   // Queue data structure
#include <algorithm>                               // Common algorithms
#include <memory>                                  // Smart pointers
#include <iterator>                                // Container iteration
#include <sstream>                                 // String streams
#include <fstream>                                 // File I/O
#include <cmath>                                   // Mathematical operations
#include <ctime>                                   // Time and date
#include <cctype>                                  // Character classification
#include <cstdio>                                  // C-style I/O
#include <stdexcept>                               // Standard exceptions
#include <chrono>                                  // Time utilities
#include <thread>                                  // Threading support
#include <future>                                  // Asynchronous computations

//------------------------------------------------------------------------------
// System Headers                                   // Linux system functionality
//----------------------y--------------------------------------------------------
#include <dirent.h>                                // Directory handling
#include <unistd.h>                                // POSIX operating system API
#include <limits.h>                                // System limits
#include <cpuid.h>                                 // CPU identification
#include <sys/types.h>                             // System types
#include <sys/sysinfo.h>                           // System statistics
#include <sys/statvfs.h>                           // Filesystem statistics
#include <netdb.h>                                 // Network database operations
#include <ifaddrs.h>                               // Network interface addresses
#include <netinet/in.h>                            // Internet address family
#include <arpa/inet.h>                             // Internet operations

class CPUUsageCalculator;
using namespace std;

//------------------------------------------------------------------------------
// Data Structures
//------------------------------------------------------------------------------

// CPU Statistics Structure
struct CPUStats {
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
};

// Process Statistics Structure
struct ProcessStats {
    unsigned long long utime, stime, cutime, cstime;
};

// Process Information Structure
struct ProcessInfo {
    int pid;
    std::string name;
    std::string state;
    float cpuUsage;
    float memoryUsage;
    std::chrono::steady_clock::time_point lastCpuUpdateTime;
    bool isActive;
};

// Network Interface Structure
struct NetworkInterface {
    std::string name;
    std::string ipv4;
    struct {
        long long bytes, packets, errs, drop, fifo, frame, compressed, multicast;
    } rx;
    struct {
        long long bytes, packets, errs, drop, fifo, colls, carrier, compressed;
    } tx;
};

//------------------------------------------------------------------------------
// System Information Functions
//------------------------------------------------------------------------------
string CPUinfo();
std::string getLoggedInUser();
const char *getOsName();
int getTotalProcesses();
std::string getHostName();
std::string getCpuInfo();
float GetCPULoad();
float GetFanSPeed();
float GetTemprature();
void GetMemoryUsage(float &physUsedPercentage, float &swapUsedPercentage, 
                    std::string &totalMemoryStr, std::string &usedMemoryStr, 
                    std::string &totalSwapStr, std::string &usedSwapStr);
void GetDiskUsage(float &diskUsedPercentage, std::string &usedStorageStr, std::string &totalStorageStr);

//------------------------------------------------------------------------------
// Process & Memory Management Functions
//------------------------------------------------------------------------------
float GetCPUUsage(int pid);
float GetMemUsage(int pid);
std::vector<ProcessInfo> FetchProcessList();
void RenderProcessMonitorUI();
std::pair<std::pair<std::pair<long, std::string>, std::pair<long, std::string>>,
          std::pair<std::pair<long, std::string>, std::pair<long, std::string>>>
getMemUsage();
std::pair<std::pair<long, long>, std::pair<std::string, std::string>> getDiskUsage();
bool isNumber(const std::string &str);
std::vector<int> GetAllPIDs();

//------------------------------------------------------------------------------
// Network Functions
//------------------------------------------------------------------------------
std::vector<NetworkInterface> getNetworkInfo();
std::string formatBytes(long long bytes);
void StartFetchingProcesses();
void RenderNetworkTable(const char* label, const std::vector<NetworkInterface>& interfaces, bool isRX);

//------------------------------------------------------------------------------
// Process Queue Management
//------------------------------------------------------------------------------
class ProcessInfoQueue {
private:
    std::queue<ProcessInfo> queue;
    std::mutex mutex;
    std::condition_variable cond;

public:
    void push(ProcessInfo&& item);
    bool try_pop(ProcessInfo& item);
};

extern ProcessInfoQueue g_completedProcesses;
extern std::vector<ProcessInfo> updateProcessList;

//------------------------------------------------------------------------------
// Rendering Functions
//------------------------------------------------------------------------------
void RenderSystemInfo();
void RenderSystemMonitor();
void RenderGraph(const char *label, float *data, int data_size, float y_scale, bool animate);
void RenderMemoryProcessMonitor();
void RenderNetworkInfo();

#endif
