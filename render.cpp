#include "header.h"
#include <chrono>  // For time tracking
#include <unordered_set>

// Cache for process list and synchronization primitives
static std::vector<ProcessInfo> processList;
std::atomic<bool> g_isFetchingProcesses(false); 
std::vector<ProcessInfo> displayProcessList;
std::vector<ProcessInfo> updateProcessList;
std::mutex processListMutex;
std::atomic<bool> isUpdating(false);

// Renders basic system information like OS, user, hostname etc.
void RenderSystemInfo() {
    ImGuiIO& io = ImGui::GetIO();
    
    // Push larger font if available
    if (io.Fonts->Fonts.Size > 0) {
        ImGui::PushFont(io.Fonts->Fonts[1]);
    }

    // Render header with cyan color
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 255, 255));
    ImGui::Text("System Information");
    ImGui::PopStyleColor();
    ImGui::Separator();

    // Display system info with spacing
    ImGui::Spacing();
    ImGui::Text("OS: %s", getOsName());
    ImGui::Spacing(); 
    ImGui::Text("Logged-in User: %s", getLoggedInUser().c_str());
    ImGui::Spacing();
    ImGui::Text("Hostname: %s", getHostName().c_str());
    ImGui::Spacing();
    ImGui::Text("CPU: %s", CPUinfo().c_str());
    ImGui::Spacing();

    // Display process count with mutex protection
    {
        std::lock_guard<std::mutex> lock(processListMutex);
        ImGui::Text("Total Processes: %zu", displayProcessList.size());
    }

    if (io.Fonts->Fonts.Size > 0) {
        ImGui::PopFont();
    }
}

// Renders a graph with given data and styling
void RenderGraph(const char* label, float* data, int data_size, float y_scale, bool animate) {
    // Cyan text for label
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 255, 255));
    ImGui::Text("%s", label);
    ImGui::PopStyleColor();
    
    // Style the graph with cyan lines and dark background
    ImGui::PushStyleColor(ImGuiCol_PlotLines, IM_COL32(0, 255, 255, 255));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(30, 30, 30, 255));
    ImGui::PlotLines("##Graph", data, data_size, 0, NULL, 0.0f, y_scale, 
                     ImVec2(ImGui::GetContentRegionAvail().x, 120));
    ImGui::PopStyleColor(2);
}

// Renders system monitor with CPU, Fan and Thermal tabs
void RenderSystemMonitor() {
    // Animation and display settings
    static bool animateCPU = true, animateFan = true, animateThermal = true;
    static float yScale = 50.0f;
    static float fpsCPU = 30.0f, fpsFan = 30.0f, fpsThermal = 30.0f;
    static float cpuRefreshInterval = 1.0f;

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

    if (ImGui::BeginTabBar("System Info", ImGuiTabBarFlags_FittingPolicyScroll)) {
        // CPU monitoring tab
        if (ImGui::BeginTabItem("CPU")) {
            static std::vector<float> cpuData(100, 0.0f);
            static float cpuLoad = 0.0f;
            static float frameCountCPU = 0.0f;
            static float cpuLoadRefreshTimer = 0.0f;

            // Update timers
            frameCountCPU += ImGui::GetIO().DeltaTime;
            cpuLoadRefreshTimer += ImGui::GetIO().DeltaTime;

            // Refresh CPU load periodically
            if (cpuLoadRefreshTimer >= cpuRefreshInterval) {
                cpuLoad = GetCPULoad();
                cpuLoadRefreshTimer = 0.0f;
            }

            // Animate graph if enabled
            if (frameCountCPU >= 4.0f / fpsCPU && animateCPU) {
                std::rotate(cpuData.rbegin(), cpuData.rbegin() + 1, cpuData.rend());
                cpuData[0] = cpuLoad;
                frameCountCPU = 0.0f;
            }

            ImGui::Spacing();
            RenderGraph("CPU Load", cpuData.data(), cpuData.size(), yScale, animateCPU);

            ImGui::Spacing();
            ImGui::Text("Current CPU Load: %.1f%%", cpuLoad);
            ImGui::Checkbox("Animate CPU", &animateCPU);
            ImGui::SliderFloat("CPU FPS", &fpsCPU, 1.0f, 60.0f);

            ImGui::EndTabItem();
        }

        // Fan monitoring tab
        if (ImGui::BeginTabItem("Fan")) {
            static std::vector<float> fanData(100, 0.0f);
            float fanSpeed = GetFanSPeed();

            static float frameCountFan = 0.0f;
            frameCountFan += ImGui::GetIO().DeltaTime;

            // Animate fan graph
            if (frameCountFan >= 1.0f / fpsFan && animateFan) {
                std::rotate(fanData.rbegin(), fanData.rbegin() + 1, fanData.rend());
                fanData[0] = fanSpeed;
                frameCountFan = 0.0f;
            }

            ImGui::Spacing();
            RenderGraph("Fan Speed", fanData.data(), fanData.size(), yScale, animateFan);

            ImGui::Spacing();
            ImGui::Text("Fan Status: Active");
            ImGui::Text("Current Fan Speed: %.1f RPM", fanSpeed);
            ImGui::Checkbox("Animate Fan", &animateFan);
            ImGui::SliderFloat("Fan FPS", &fpsFan, 1.0f, 60.0f);

            ImGui::EndTabItem();
        }

        // Thermal monitoring tab
        if (ImGui::BeginTabItem("Thermal")) {
            static std::vector<float> thermalData(100, 0.0f);
            float temperature = GetTemprature();

            static float frameCountThermal = 0.0f;
            frameCountThermal += ImGui::GetIO().DeltaTime;

            // Animate temperature graph
            if (frameCountThermal >= 1.0f / fpsThermal && animateThermal) {
                std::rotate(thermalData.rbegin(), thermalData.rbegin() + 1, thermalData.rend());
                thermalData[0] = temperature;
                frameCountThermal = 0.0f;
            }

            ImGui::Spacing();
            RenderGraph("Temperature", thermalData.data(), thermalData.size(), yScale, animateThermal);

            ImGui::Spacing();
            ImGui::Text("Current Temperature: %.1f °C", temperature);
            ImGui::Checkbox("Animate Thermal", &animateThermal);
            ImGui::SliderFloat("Thermal FPS", &fpsThermal, 1.0f, 60.0f);

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    // Graph scale control
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 255, 255));
    ImGui::Text("Graph Scale");
    ImGui::PopStyleColor();
    ImGui::SliderFloat("Y Scale", &yScale, 0.0f, 200.0f);

    ImGui::PopFont();
}

// Renders memory and process monitor UI
void RenderMemoryProcessMonitor() {
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

    // Memory usage variables
    float physMemUsage = 0.0f;
    float swapMemUsage = 0.0f;
    float diskUsage = 0.0f;
    std::string totalMemoryStr, usedMemoryStr, totalSwapStr, usedSwapStr;
    std::string usedStorageStr, totalStorageStr;

    // Get current system metrics
    GetMemoryUsage(physMemUsage, swapMemUsage, totalMemoryStr, usedMemoryStr, totalSwapStr, usedSwapStr);
    GetDiskUsage(diskUsage, usedStorageStr, totalStorageStr);

    // Create memory monitor window
    ImGui::BeginChild("MemoryMonitor", ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight() * 0.4f), 
                     true, ImGuiWindowFlags_AlwaysUseWindowPadding);

    // Header
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 255, 255));
    ImGui::Text("Memory and Storage");
    ImGui::PopStyleColor();

    ImGui::Spacing();

    // RAM usage
    ImGui::Text("Physical Memory (RAM)");
    ImGui::SameLine(ImGui::GetWindowWidth() - 200);
    ImGui::Text("%s / %s", usedMemoryStr.c_str(), totalMemoryStr.c_str());

    char physMemLabel[32];
    sprintf(physMemLabel, "%.1f%%", physMemUsage);
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(0, 255, 255, 255));
    ImGui::ProgressBar(physMemUsage / 100.0f, ImVec2(-1.0f, 0.0f), physMemLabel);
    ImGui::PopStyleColor();

    ImGui::Spacing();

    // Swap usage
    ImGui::Text("Virtual Memory (SWAP)");
    ImGui::SameLine(ImGui::GetWindowWidth() - 200);
    ImGui::Text("%s / %s", usedSwapStr.c_str(), totalSwapStr.c_str());

    char swapMemLabel[32];
    sprintf(swapMemLabel, "%.1f%%", swapMemUsage);
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(0, 255, 255, 255));
    ImGui::ProgressBar(swapMemUsage / 100.0f, ImVec2(-1.0f, 0.0f), swapMemLabel);
    ImGui::PopStyleColor();

    ImGui::Spacing();

    // Disk usage
    ImGui::Text("Disk Usage");
    ImGui::SameLine(ImGui::GetWindowWidth() - 200);
    ImGui::Text("%s / %s", usedStorageStr.c_str(), totalStorageStr.c_str());

    char diskUsageLabel[32];
    sprintf(diskUsageLabel, "%.1f%%", diskUsage);
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(0, 255, 255, 255));
    ImGui::ProgressBar(diskUsage / 100.0f, ImVec2(-1.0f, 0.0f), diskUsageLabel);
    ImGui::PopStyleColor();

    ImGui::EndChild();

    ImGui::PopFont();
}

// Renders process monitor UI with filtering and sorting
void RenderProcessMonitorUI() {
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
    
    static char filterText[64] = "";
    static std::unordered_set<int> selectedPIDs;
    static bool fetchThreadStarted = false;

    // Start process fetching thread once
    if (!fetchThreadStarted) {
        fetchThreadStarted = true;
        std::thread([]() {
            StartFetchingProcesses();
        }).detach();
    }

    // Process new results
    ProcessInfo newProcess;
    while (g_completedProcesses.try_pop(newProcess)) {
        if (newProcess.cpuUsage > -1) {
            std::lock_guard<std::mutex> lock(processListMutex);
            
            auto it = std::find_if(displayProcessList.begin(), displayProcessList.end(),
                                 [&](const ProcessInfo& p) { return p.pid == newProcess.pid; });
            
            if (it != displayProcessList.end()) {
                *it = newProcess;
            } else {
                displayProcessList.push_back(newProcess);
            }    
        }
    }

    // Clean up and sort process list
    {
        std::lock_guard<std::mutex> lock(processListMutex);
        displayProcessList.erase(
            std::remove_if(displayProcessList.begin(), displayProcessList.end(),
                          [](const ProcessInfo& p) { return p.cpuUsage <= -1 || !p.isActive; }),
            displayProcessList.end()
        );

        std::sort(displayProcessList.begin(), displayProcessList.end(),
                 [](const ProcessInfo& a, const ProcessInfo& b) { return a.pid < b.pid; });
    }

    ImGui::Text("Total Number of Processes: %zu", displayProcessList.size());
    ImGui::InputTextWithHint("##Filter", "Filter processes...", filterText, IM_ARRAYSIZE(filterText));

    if (displayProcessList.empty()) {
        ImGui::Text("No processes found.");
        ImGui::PopFont();
        return;
    }

    // Render process table
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10, 5));
    if (ImGui::BeginTable("ProcessTable", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | 
                         ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable)) {
        // Setup columns
        ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_DefaultSort);
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("State");
        ImGui::TableSetupColumn("CPU Usage (%)", ImGuiTableColumnFlags_PreferSortDescending);
        ImGui::TableSetupColumn("Memory Usage (%)", ImGuiTableColumnFlags_PreferSortDescending);
        ImGui::TableHeadersRow();

        // Display processes
        for (const auto& process : displayProcessList) {
            if (process.name.empty() || process.state.empty() || process.cpuUsage <= -1) {
                continue;
            }
            if (strlen(filterText) > 0 && process.name.find(filterText) == std::string::npos) {
                continue;
            }

            ImGui::TableNextRow();
            ImGui::PushID(process.pid);

            bool isSelected = selectedPIDs.find(process.pid) != selectedPIDs.end();

            if (isSelected) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, IM_COL32(0, 128, 0, 100));
            }

            // Display process information
            ImGui::TableSetColumnIndex(0);
            if (ImGui::Selectable(std::to_string(process.pid).c_str(), isSelected, 
                                ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap)) {
                if (isSelected) {
                    selectedPIDs.erase(process.pid);
                } else {
                    selectedPIDs.insert(process.pid);
                }
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(process.name.c_str());
            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(process.state.c_str());
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%.2f", process.cpuUsage);
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%.2f", process.memoryUsage);

            ImGui::PopID();
        }

        ImGui::EndTable();
    }
    ImGui::PopStyleVar();

    ImGui::PopFont();
}

// Renders network interface information
void RenderNetworkInfo() {
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

    auto interfaces = getNetworkInfo();

    // Header
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 255, 255));
    ImGui::Text("Network Interfaces");
    ImGui::PopStyleColor();
    ImGui::Separator();

    // Display interface information
    for (const auto& iface : interfaces) {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(30, 30, 30, 255));
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200, 200, 200, 255));

        ImGui::BeginChild(iface.name.c_str(), ImVec2(0, 60), true);
        ImGui::Text("%s: %s", iface.name.c_str(), iface.ipv4.c_str());
        
        float usage = static_cast<float>(iface.rx.bytes + iface.tx.bytes) / (30.0f * 1024 * 1024 * 1024);
        std::string label = formatBytes(iface.rx.bytes + iface.tx.bytes);
        
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(0, 255, 255, 255));
        ImGui::ProgressBar(usage, ImVec2(-1, 0), label.c_str());
        ImGui::PopStyleColor();

        ImGui::EndChild();

        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();
        ImGui::Spacing();
    }

    ImGui::Separator();
    ImGui::Spacing();

    // Network tabs
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    ImGui::PushStyleColor(ImGuiCol_Tab, IM_COL32(40, 40, 40, 255));
    ImGui::PushStyleColor(ImGuiCol_TabHovered, IM_COL32(60, 60, 60, 255));
    ImGui::PushStyleColor(ImGuiCol_TabActive, IM_COL32(0, 255, 255, 100));

    if (ImGui::BeginTabBar("NetworkTabs")) {
        if (ImGui::BeginTabItem("RX")) {
            RenderNetworkTable("RX", interfaces, true);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("TX")) {
            RenderNetworkTable("TX", interfaces, false);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();
    ImGui::PopFont();
}

// Renders network statistics table
void RenderNetworkTable(const char* label, const std::vector<NetworkInterface>& interfaces, bool isRX) {
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5, 5));
    ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, IM_COL32(40, 40, 40, 255));
    ImGui::PushStyleColor(ImGuiCol_TableRowBg, IM_COL32(20, 20, 20, 255));
    ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, IM_COL32(30, 30, 30, 255));

    // Create table with network statistics
    if (ImGui::BeginTable(label, 9, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
        // Setup columns
        ImGui::TableSetupColumn("Interface");
        ImGui::TableSetupColumn("Bytes");
        ImGui::TableSetupColumn("Packets");
        ImGui::TableSetupColumn("Errs");
        ImGui::TableSetupColumn("Drop");
        ImGui::TableSetupColumn("Fifo");
        ImGui::TableSetupColumn(isRX ? "Frame" : "Colls");
        ImGui::TableSetupColumn("Compressed");
        ImGui::TableSetupColumn(isRX ? "Multicast" : "Carrier");
        ImGui::TableHeadersRow();

        // Display interface statistics
        for (const auto& iface : interfaces) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", iface.name.c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%lld", isRX ? iface.rx.bytes : iface.tx.bytes);
            ImGui::TableNextColumn();
            ImGui::Text("%lld", isRX ? iface.rx.packets : iface.tx.packets);
            ImGui::TableNextColumn();
            ImGui::Text("%lld", isRX ? iface.rx.errs : iface.tx.errs);
            ImGui::TableNextColumn();
            ImGui::Text("%lld", isRX ? iface.rx.drop : iface.tx.drop);
            ImGui::TableNextColumn();
            ImGui::Text("%lld", isRX ? iface.rx.fifo : iface.tx.fifo);
            ImGui::TableNextColumn();
            ImGui::Text("%lld", isRX ? iface.rx.frame : iface.tx.colls);
            ImGui::TableNextColumn();
            ImGui::Text("%lld", isRX ? iface.rx.compressed : iface.tx.compressed);
            ImGui::TableNextColumn();
            ImGui::Text("%lld", isRX ? iface.rx.multicast : iface.tx.carrier);
        }
        ImGui::EndTable();
    }

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    // Display usage bars
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 255, 255));
    ImGui::Text("%s Usage", label);
    ImGui::PopStyleColor();

    for (const auto& iface : interfaces) {
        float usage = static_cast<float>(isRX ? iface.rx.bytes : iface.tx.bytes) / (30.0f * 1024 * 1024 * 1024);
        std::string usageLabel = formatBytes(isRX ? iface.rx.bytes : iface.tx.bytes);
        ImGui::Text("%s", iface.name.c_str());
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(0, 255, 255, 255));
        ImGui::ProgressBar(usage, ImVec2(-1, 0), usageLabel.c_str());
        ImGui::PopStyleColor();
    }
}
