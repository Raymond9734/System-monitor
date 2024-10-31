#include "header.h"
#ifdef _WIN32
    #include <windows.h>
    #include <Lmcons.h> // for UNLEN (max username length)
#else
    #include <sys/utsname.h>
    #include <unistd.h>
    #include <pwd.h>
    #include <fstream>
#endif
  
/**
 * Gets CPU identification and information by querying CPU directly
 * Uses CPUID instruction to get CPU brand string
 * Works on x86/x64 processors that support CPUID
 * 
 * @return CPU brand string containing model name and details
 */
string CPUinfo()
{
    char CPUBrandString[0x40];
    unsigned int CPUInfo[4] = {0, 0, 0, 0};

    // Get number of extended CPUID functions
    __cpuid(0x80000000, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3]);
    unsigned int nExIds = CPUInfo[0];

    memset(CPUBrandString, 0, sizeof(CPUBrandString));

    // Get CPU brand string by calling CPUID with extended function codes
    for (unsigned int i = 0x80000000; i <= nExIds; ++i)
    {
        __cpuid(i, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3]);

        // CPU brand string is split across three CPUID calls
        if (i == 0x80000002)
            memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
        else if (i == 0x80000003)
            memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
        else if (i == 0x80000004)
            memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
    }
    string str(CPUBrandString);
    return str;
}

/**
 * Detects and returns the operating system name
 * Uses preprocessor macros to identify the OS at compile time
 * 
 * @return String containing the OS name
 */
const char *getOsName()
{
#ifdef _WIN32
    return "Windows 32-bit";
#elif _WIN64
    return "Windows 64-bit";
#elif __APPLE__ || __MACH__
    return "Mac OSX";
#elif __linux__
    return "Linux";
#elif __FreeBSD__
    return "FreeBSD";
#elif __unix || __unix__
    return "Unix";
#else
    return "Other";
#endif
}

/**
 * Gets the currently logged in username
 * Uses platform-specific APIs to retrieve the username
 * 
 * @return String containing the username
 */
std::string getLoggedInUser() {

    #ifdef _WIN32
        char username[UNLEN + 1];
        DWORD username_len = UNLEN + 1;
        GetUserName(usernam, &usernam_len);
        return std::string(username);
    #else
    struct passwd *pw = getpwuid(getuid());
    return std::string(pw->pw_name);

#endif
}

/**
 * Gets the system hostname
 * Uses gethostname() system call
 * 
 * @return String containing the hostname, or NULL on error
 */
std::string getHostName(){
    char hostName[1024];
    if (gethostname(hostName,sizeof(hostName))!= 0){
        perror("getHostName");
        return NULL;
    };
    return std::string(hostName);
}

/**
 * Gets total number of processes running on the system
 * Uses platform-specific methods:
 * - Windows: ToolHelp API to enumerate processes
 * - Linux: Reads from /proc/stat
 * 
 * @return Total number of processes
 */
int getTotalProcesses() {
    int totalProcesses = 0;
    #ifdef _WIN32
        // On Windows, use the ToolHelp API to enumerate processes
        HANDLE hProcessSnap;
        PROCESSENTRY32 pe32;
        
        // Create a snapshot of all processes in the system
        hProcessSnap = CreateToolhelp32Snapshot(TH32CS_PROCESS, 0);
        if (hProcessSnap == INVALID_HANDLE_VALUE) {
            return 0; // Error occurred while creating snapshot
        }

        // Set the size of the structure before using it
        pe32.dwSize = sizeof(PROCESSENTRY32);

        // Retrieve information about the first process
        if (Process32First(hProcessSnap, &pe32)) {
            do {
                totalProcesses++; // Count each process
            } while (Process32Next(hProcessSnap, &pe32));
        }

        // Clean up the snapshot handle
        CloseHandle(hProcessSnap);
        return totalProcesses;

    #else
    std::ifstream statfile("/proc/stat");
    std::string line;
    while (std::getline(statfile,line)) {
        if (line.find("processes") != std::string::npos){
            totalProcesses = std::stoi(line.substr(line.find(" ") + 1));
            break;
        }
    }
    return totalProcesses;
#endif   
}

/**
 * Gets CPU information based on platform
 * Windows: Returns number of processors
 * Linux: Returns CPU model name from /proc/cpuinfo
 * 
 * @return String containing CPU info
 */
std::string getCpuInfo() {
    #ifdef _WIN32
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        return std::to_string(sysInfo.dwNumberOfProcessors);
    #else
    std::ifstream cpuInfo("/proc/cpuinfo");
    std::string line, cpuModel;
    while (std::getline(cpuInfo,line)) {
        if (line.find("model name") != std::string::npos){
            cpuModel = line.substr(line.find(":") + 2);
            break;
        }
    }
    return cpuModel;
#endif
}

/**
 * Calculates current CPU load percentage
 * Reads CPU time statistics from /proc/stat
 * Compares current values with previous values to calculate load
 * 
 * @return CPU load as percentage (0-100)
 */
float GetCPULoad() {
    std::ifstream file("/proc/stat");
    std::string line;

    if (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string cpu;
        unsigned long user, nice, system, idle, iowait, irq, softirq, steal;
        // user: Time spent in user mode.
        // nice: Time spent in user mode with low priority (nice).
        // system: Time spent in system mode (kernel).
        // idle: Time spent idle.
        // iowait: Time spent waiting for I/O operations to complete.
        // irq: Time spent servicing hardware interrupts.
        // softirq: Time spent servicing software interrupts.
        // steal: Time spent in other operating systems when running in a virtualized environment.
        iss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

        unsigned long totalIdle = idle + iowait;
        unsigned long totalActive = user + nice + system + irq + softirq + steal;

        static unsigned long prevTotalActive = totalActive;
        static unsigned long prevTotalIdle = totalIdle;

        unsigned long total = totalActive + totalIdle;
        unsigned long totalDiff = total - (prevTotalActive + prevTotalIdle);
        unsigned long idleDiff = totalIdle - prevTotalIdle;

        prevTotalActive = totalActive;
        prevTotalIdle = totalIdle;

        return (totalDiff == 0) ? 0.0f : (1.0 - (double)idleDiff / (double)totalDiff) * 100.0f; // CPU Load percentage

    };
    return 0.0f;
}

/**
 * Gets current fan speed
 * Reads from hwmon sysfs interface
 * 
 * @return Fan speed in RPM
 */
float GetFanSPeed(){
    std::ifstream file("/sys/class/hwmon/hwmon5/fan1_input");
    float fanSpeed = 0.0f;
    if (file.is_open()) {
        file >> fanSpeed; // read fan speed
    }
    return fanSpeed;
}

/**
 * Gets current CPU temperature
 * Reads from thermal zone sysfs interface
 * 
 * @return Temperature in degrees Celsius
 */
float GetTemprature(){
    std::ifstream file("/sys/class/thermal/thermal_zone0/temp");
    float temprature = 0.0f;
    if (file.is_open()) {
        file >> temprature;    // Read temperature in millidegrees Celsius
        temprature /= 1000.0f; // Convert to degrees Celsius
    }
    return temprature;
}