// To make sure you don't declare the function more than once by including the header multiple times.
#ifndef header_H
#define header_H

#include "imgui/lib/imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <dirent.h>
#include <vector>
#include <iostream>
#include <cmath>
// lib to read from file
#include <fstream>
// for the name of the computer and the logged in user
#include <unistd.h>
#include <limits.h>
// this is for us to get the cpu information
// mostly in unix system
// not sure if it will work in windows
#include <cpuid.h>
// this is for the memory usage and other memory visualization
// for linux gotta find a way for windows
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
// for time and date
#include <ctime>
// ifconfig ip addresses
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <map>
//for string stream operations.
#include <sstream>

#include <netdb.h>
#include <chrono>
#include <thread>
#include <future>
#include <string>

#include <array>
#include <memory>
#include <cstdio>
#include <iterator>
#include <stdexcept>
using namespace std;

// Struct to hold the system CPU times
struct CPUStats {
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
};

// Struct to hold the process CPU times
struct ProcessStats {
    unsigned long long utime, stime, cutime, cstime;
};

// Structure to hold process information
struct ProcessInfo {
    int pid;
    std::string name;
    std::string state;
    float cpuUsage;
    float memoryUsage;
};


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

// student TODO : system stats
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
void GetDiskUsage(float &diskUsedPercentage,std::string &usedStorageStr,std::string &totalStorageStr) ;

// student TODO : memory and processes
float GetCPUUsage(int pid);
float GetMemUsage(int pid);
std::vector<ProcessInfo> FetchProcessList();
void RenderProcessMonitorUI();
std::pair<std::pair<std::pair<long, std::string>, std::pair<long, std::string>>,
          std::pair<std::pair<long, std::string>, std::pair<long, std::string>>>
getMemUsage();
std::pair< std::pair<long, long>, std::pair<std::string, std::string> > getDiskUsage();

// student TODO : network
std::vector<NetworkInterface> getNetworkInfo();
std::string formatBytes(long long bytes);

//Render  Functions
void RenderSystemInfo();
void RenderSystemMonitor();
void RenderGraph(const char *label, float *data, int data_size, float y_scale, bool animate);
void RenderMemoryProcessMonitor();
void RenderNetworkInfo();

#endif
