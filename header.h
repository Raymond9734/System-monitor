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

#include <string>

#include <array>
#include <memory>
#include <cstdio>
#include <iterator>
#include <stdexcept>
using namespace std;

struct CPUStats
{
    long long int user;
    long long int nice;
    long long int system;
    long long int idle;
    long long int iowait;
    long long int irq;
    long long int softirq;
    long long int steal;
    long long int guest;
    long long int guestNice;
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
void GetMemoryUsage(float &physUsedPercentage, float &swapUsedPercentage);
void GetDiskUsage(float &diskUsedPercentage);

// student TODO : memory and processes
float GetCPUUsage(int pid);
float GetMemUsage(int pid);
std::vector<ProcessInfo> FetchProcessList();
void RenderProcessMonitorUI();
std::pair<std::pair<long, long>, std::pair<long, long>> getMemUsage();
std::pair<long, long> getDiskUsage();

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
