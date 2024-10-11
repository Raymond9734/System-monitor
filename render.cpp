#include "header.h"
#include <chrono>  // For time tracking
#include <unordered_set>

static std::vector<ProcessInfo> processList; // Cache process list

void RenderSystemInfo() {


    ImGui::Text("OS: %s", getOsName());

    ImGui::Text("Logged-in-User: %s",getLoggedInUser().c_str());

    ImGui::Text("Hostname: %s", getHostName().c_str());

    ImGui::Text("CPU: %s", CPUinfo().c_str());

    ImGui::Text("Total Processes: %d", processList.size());

}
void RenderGraph(const char* label, float* data, int data_size, float y_scale, bool animate) {
    ImGui::Text(label);
    
    // Create a graph (you can use ImGui::PlotLines or any custom plotting function)
    ImGui::PlotLines("Graph", data, data_size, 0, NULL, 0.0f, y_scale, ImVec2(0, 80));
}
void RenderSystemMonitor() {
    static bool animateCPU = true, animateFan = true, animateThermal = true;
    static float yScale = 50.0f;
    static float fpsCPU = 30.0f, fpsFan = 30.0f, fpsThermal = 30.0f;
    static float cpuRefreshInterval = 1.0f;  // CPU load refresh interval in seconds

    ImGui::BeginTabBar("System Info");

    // CPU Tab
    if (ImGui::BeginTabItem("CPU")) {
        static float cpuData[100]; // Buffer for CPU data
        static int cpuDataSize = 0;
        static float cpuLoad = 0.0f;  // Hold current CPU load

        static float frameCountCPU = 0.0f;  // Timer for animation
        static float cpuLoadRefreshTimer = 0.0f;  // Timer for CPU refresh

        // Increment timers with delta time
        frameCountCPU += ImGui::GetIO().DeltaTime;
        cpuLoadRefreshTimer += ImGui::GetIO().DeltaTime;

        // Refresh CPU load after specified interval
        if (cpuLoadRefreshTimer >= cpuRefreshInterval) {
            cpuLoad = GetCPULoad();  // Fetch new CPU load
            cpuLoadRefreshTimer = 0.0f;  // Reset the timer
        }

        // Store CPU load data in a circular buffer if animating
        if (frameCountCPU >= 4.0f / fpsCPU && animateCPU) {
            cpuData[cpuDataSize++ % 100] = cpuLoad;
            frameCountCPU = 0.0f;  // Reset frame count
        }

        // Dummy space before the graph
        ImGui::Dummy(ImVec2(0.0f, 20.0f));

        // Render CPU graph
        RenderGraph("CPU Load", cpuData, cpuDataSize, yScale, animateCPU);

        // Display current CPU load and controls
        ImGui::Text("Current CPU Load: %.1f%%", cpuLoad);
        ImGui::Checkbox("Animate CPU", &animateCPU);
        ImGui::SliderFloat("CPU FPS", &fpsCPU, 1.0f, 60.0f);

        ImGui::EndTabItem();
    }

    // Fan Tab
    if (ImGui::BeginTabItem("Fan")) {
        static float fanData[100]; // Buffer for Fan data
        static int fanDataSize = 0;

        float fanSpeed = GetFanSPeed();

        static float frameCountFan = 0.0f;
        frameCountFan += ImGui::GetIO().DeltaTime;
        if (frameCountFan >= 1.0f / fpsFan && animateFan) {
            fanData[fanDataSize++ % 100] = fanSpeed;
            frameCountFan = 0.0f;
        }

        RenderGraph("Fan Speed", fanData, fanDataSize, yScale, animateFan);
        ImGui::Text("Fan Status: Active");
        ImGui::Text("Current Fan Speed: %.1f RPM", fanSpeed);
        ImGui::Checkbox("Animate Fan", &animateFan);
        ImGui::SliderFloat("Fan FPS", &fpsFan, 1.0f, 60.0f);

        ImGui::EndTabItem();
    }

    // Thermal Tab
    if (ImGui::BeginTabItem("Thermal")) {
        static float thermalData[100]; // Buffer for Thermal data
        static int thermalDataSize = 0;

        float temperature = GetTemprature();

        static float frameCountThermal = 0.0f;
        frameCountThermal += ImGui::GetIO().DeltaTime;
        if (frameCountThermal >= 1.0f / fpsThermal && animateThermal) {
            thermalData[thermalDataSize++ % 100] = temperature;
            frameCountThermal = 0.0f;
        }

        RenderGraph("Temperature", thermalData, thermalDataSize, yScale, animateThermal);
        ImGui::Text("Current Temperature: %.1f °C", temperature);
        ImGui::Checkbox("Animate Thermal", &animateThermal);
        ImGui::SliderFloat("Thermal FPS", &fpsThermal, 1.0f, 60.0f);

        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();

    // Global controls for Y scale
    ImGui::SliderFloat("Y Scale", &yScale, 0.0f, 200.0f);
}

// Function to render the memory and process monitor
void RenderMemoryProcessMonitor() {

    // Variables to store memory and disk usage percentages
    float physMemUsage = 0.0f;
    float swapMemUsage = 0.0f;
    float diskUsage = 0.0f;
    std::string totalMemoryStr;
    std::string usedMemoryStr;
    std::string totalSwapStr;
    std::string usedSwapStr;
    std::string usedStorageStr;
    std::string totalStorageStr;

    // Get current system metrics
    GetMemoryUsage(physMemUsage, swapMemUsage, totalMemoryStr, usedMemoryStr, totalSwapStr, usedSwapStr);
    GetDiskUsage(diskUsage,usedStorageStr,totalStorageStr);

    // Display memory information with human-readable strings
    ImGui::Text("Physical Memory (RAM):");
    ImGui::Text("Total: %s, Used: %s", totalMemoryStr.c_str(), usedMemoryStr.c_str());

    // Display RAM usage with a progress bar
    char physMemLabel[32];
    sprintf(physMemLabel, "%.0f%%", physMemUsage);  // Format the percentage label
    ImGui::ProgressBar(physMemUsage / 100.0f, ImVec2(0.0f, 30.0f), physMemLabel);

    // Display swap information with human-readable strings
    ImGui::Text("Virtual Memory (SWAP):");
    ImGui::Text("Total: %s, Used: %s", totalSwapStr.c_str(), usedSwapStr.c_str());

    // Display swap usage with a progress bar
    char swapMemLabel[32];
    sprintf(swapMemLabel, "%.0f%%", swapMemUsage);  // Format the percentage label
    ImGui::ProgressBar(swapMemUsage / 100.0f, ImVec2(0.0f, 30.0f), swapMemLabel);

    // Display disk usage with a progress bar
    ImGui::Text("Disk Usage:");
    ImGui::Text("Total: %s, Used: %s", totalStorageStr.c_str(), usedStorageStr.c_str());
    char diskUsageLabel[32];
    sprintf(diskUsageLabel, "%.0f%%", diskUsage);  // Format the percentage label
    ImGui::ProgressBar(diskUsage / 100.0f, ImVec2(0.0f, 30.0f), diskUsageLabel);
}

void RenderProcessMonitorUI() {
    static char filterText[64] = ""; // Filter text buffer
    static std::unordered_set<int> selectedPIDs; // To track selected processes by PID
    static int totalProcesses = 0; // To track total number of processes
  
    static std::future<void> futureTask;

    // Refresh process list asynchronously if more than 1 second has passed
    if (!futureTask.valid() || futureTask.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
        futureTask = std::async(std::launch::async, [&]()
        {
            processList = FetchProcessList(); 
            totalProcesses = processList.size(); 
        });
    }

    // Tab bar
    if (ImGui::BeginTabBar("##Tabs")) {
        if (ImGui::BeginTabItem("Processes")) {

            ImGui::Text("Total Number of Processes: %d", totalProcesses); 
            // Filter text box
            ImGui::InputText("Filter", filterText, IM_ARRAYSIZE(filterText));

            // Handle empty case gracefully
            if (processList.empty()) {
                ImGui::Text("No processes found.");
                ImGui::EndTabItem();
                ImGui::EndTabBar();
                return;
            }

            // Table displaying processes
            if (ImGui::BeginTable("ProcessTable", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable)) {
                // Table headers
                ImGui::TableSetupColumn("PID");
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("State");
                ImGui::TableSetupColumn("CPU Usage (%)");
                ImGui::TableSetupColumn("Memory Usage (%)");
                ImGui::TableHeadersRow();

                // Loop over process list and display filtered results
                for (const auto& process : processList) {
                    if (process.name.empty() || process.state.empty()) {
                        continue; // Skip if any required fields are empty
                    }
                    // Filter based on user input
                    if (strlen(filterText) > 0 && process.name.find(filterText) == std::string::npos) {
                        continue; // Skip this process if it doesn't match the filter
                    }

                    ImGui::TableNextRow();
                    ImGui::PushID(process.pid);

                    bool isSelected = selectedPIDs.find(process.pid) != selectedPIDs.end();

                    // Set the background color for selected rows
                    if (isSelected) {
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, IM_COL32(0, 128, 0, 255)); // Highlight color
                    }

                    // Make the entire row selectable
                    if (ImGui::TableSetColumnIndex(0)) { // Start from the first column
                        if (ImGui::Selectable("##WholeRowSelectable", isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap)) {
                            if (isSelected) {
                                selectedPIDs.erase(process.pid);
                            } else {
                                selectedPIDs.insert(process.pid);
                            }
                        }
                    }

                    // Display process information in table columns
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%d", process.pid);  // PID
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", process.name.c_str());  // Name
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%s", process.state.c_str());  // State
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%.2f", process.cpuUsage);  // CPU usage
                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text("%.2f", process.memoryUsage);  // Memory usage

                    ImGui::PopID();
                }

                ImGui::EndTable();
            }

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}
// Function to render network statistics with progress bars
void RenderNetworkInfo() {
    auto interfaces = getNetworkInfo();
    // Display network interfaces and their IPv4 addresses
    ImGui::Text("Network Interfaces:");
    ImGui::Separator();

    for (const auto& iface : interfaces) {
        ImGui::Text("%s: %s", iface.name.c_str(), iface.ipv4.c_str());
        float usage = static_cast<float>(iface.rx.bytes + iface.tx.bytes) / (30.0f * 1024 * 1024 * 1024); // 30GB max
        std::string label = formatBytes(iface.rx.bytes + iface.tx.bytes);
        ImGui::ProgressBar(usage, ImVec2(-1, 0), label.c_str());
        ImGui::Spacing();
    }

    ImGui::Separator();
    ImGui::Spacing();

    // Tab bar for RX and TX
    if (ImGui::BeginTabBar("NetworkTabs")) {
        if (ImGui::BeginTabItem("RX")) {
            ImGui::BeginTable("RXTable", 9, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg);
            ImGui::TableSetupColumn("Interface");
            ImGui::TableSetupColumn("Bytes");
            ImGui::TableSetupColumn("Packets");
            ImGui::TableSetupColumn("Errs");
            ImGui::TableSetupColumn("Drop");
            ImGui::TableSetupColumn("Fifo");
            ImGui::TableSetupColumn("Frame");
            ImGui::TableSetupColumn("Compressed");
            ImGui::TableSetupColumn("Multicast");
            ImGui::TableHeadersRow();

            for (const auto& iface : interfaces) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", iface.name.c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%lld", iface.rx.bytes);
                ImGui::TableNextColumn();
                ImGui::Text("%lld", iface.rx.packets);
                ImGui::TableNextColumn();
                ImGui::Text("%lld", iface.rx.errs);
                ImGui::TableNextColumn();
                ImGui::Text("%lld", iface.rx.drop);
                ImGui::TableNextColumn();
                ImGui::Text("%lld", iface.rx.fifo);
                ImGui::TableNextColumn();
                ImGui::Text("%lld", iface.rx.frame);
                ImGui::TableNextColumn();
                ImGui::Text("%lld", iface.rx.compressed);
                ImGui::TableNextColumn();
                ImGui::Text("%lld", iface.rx.multicast);
            }
            ImGui::EndTable();

            ImGui::Text("RX Usage");
            for (const auto& iface : interfaces) {
                float usage = static_cast<float>(iface.rx.bytes) / (30.0f * 1024 * 1024 * 1024); // 30GB max
                std::string label = formatBytes(iface.rx.bytes);
                ImGui::Text("%s", iface.name.c_str());
                ImGui::ProgressBar(usage, ImVec2(-1, 0), label.c_str());
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("TX")) {
            ImGui::BeginTable("TXTable", 9, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg);
            ImGui::TableSetupColumn("Interface");
            ImGui::TableSetupColumn("Bytes");
            ImGui::TableSetupColumn("Packets");
            ImGui::TableSetupColumn("Errs");
            ImGui::TableSetupColumn("Drop");
            ImGui::TableSetupColumn("Fifo");
            ImGui::TableSetupColumn("Colls");
            ImGui::TableSetupColumn("Carrier");
            ImGui::TableSetupColumn("Compressed");
            ImGui::TableHeadersRow();

            for (const auto& iface : interfaces) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", iface.name.c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%lld", iface.tx.bytes);
                ImGui::TableNextColumn();
                ImGui::Text("%lld", iface.tx.packets);
                ImGui::TableNextColumn();
                ImGui::Text("%lld", iface.tx.errs);
                ImGui::TableNextColumn();
                ImGui::Text("%lld", iface.tx.drop);
                ImGui::TableNextColumn();
                ImGui::Text("%lld", iface.tx.fifo);
                ImGui::TableNextColumn();
                ImGui::Text("%lld", iface.tx.colls);
                ImGui::TableNextColumn();
                ImGui::Text("%lld", iface.tx.carrier);
                ImGui::TableNextColumn();
                ImGui::Text("%lld", iface.tx.compressed);
            }
            ImGui::EndTable();

            ImGui::Text("TX Usage");
            for (const auto& iface : interfaces) {
                float usage = static_cast<float>(iface.tx.bytes) / (30.0f * 1024 * 1024 * 1024); // 5GB max
                std::string label = formatBytes(iface.tx.bytes);
                ImGui::Text("%s", iface.name.c_str());
                ImGui::ProgressBar(usage, ImVec2(-1, 0), label.c_str());
            }

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}
