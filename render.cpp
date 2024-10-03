#include "header.h"


void RenderSystemInfo() {


    ImGui::Text("OS: %s", getOsName());

    ImGui::Text("Logged-in-User: %s",getLoggedInUser().c_str());

    ImGui::Text("Hostname: %s", getHostName().c_str());

    ImGui::Text("CPU: %s", CPUinfo().c_str());

    ImGui::Text("Total Processes: %d", getTotalProcesses());

}
void RenderGraph(const char* label, float* data, int data_size, float y_scale, bool animate) {
    ImGui::Text(label);
    
    // Create a graph (you can use ImGui::PlotLines or any custom plotting function)
    ImGui::PlotLines("Graph", data, data_size, 0, NULL, 0.0f, y_scale, ImVec2(0, 80));
}
void RenderSystemMonitor() {
    // static int currentTab = 0;
    static bool animate = true;
    static float yScale = 50.0f;
    static float fps = 30.0f;

    // Tab Bar
    ImGui::BeginTabBar("System Info");

    // CPU Tab
    if (ImGui::BeginTabItem("CPU")) {
        static float cpuData[100]; // Buffer for CPU data
        static int cpuDataSize = 0;
        float cpuLoad;

        // Control the graph animation
        static float frameCountCPU = 0.0f;
        frameCountCPU += ImGui::GetIO().DeltaTime; // Increment frame count with time elapsed
        if (frameCountCPU >= 4.0f / fps && animate) {
            // Get current CPU load
            cpuLoad = GetCPULoad();

            // Store CPU load Data for Graph
            cpuData[cpuDataSize++ % 100] = cpuLoad; // Store the CPU load in a circular buffer

            frameCountCPU = 0.0f; // Reset frame count
            cpuData[cpuDataSize % 100] = cpuLoad; // Update CPU load if animating
           
        }
        ImGui::Dummy(ImVec2(0.0f, 20.0f));
        RenderGraph("CPU Load", cpuData, cpuDataSize, yScale, animate);

        ImGui::Text("Current CPU Load: %.1f%%", cpuLoad);
        ImGui::Checkbox("Animate", &animate);

        ImGui::EndTabItem();
    }

    // Fan Tab
    if (ImGui::BeginTabItem("Fan")) {
        static float fanData[100]; // Buffer for Fan data
        static int fanDataSize = 0;

        // Get current fan speed
        float fanSpeed = GetFanSPeed(); 

        // Store Fan Speed data for graph
        fanData[fanDataSize++ % 100] = fanSpeed; // Store the Fan speed in a circular buffer

        // Control the graph animation
        static float frameCountFan = 0.0f;
        frameCountFan += ImGui::GetIO().DeltaTime; // Increment frame count with time elapsed
        if (frameCountFan >= 1.0f / fps && animate) {
            frameCountFan = 0.0f; // Reset frame count
            fanData[fanDataSize % 100] = fanSpeed; // Update Fan speed if animating
        }

        RenderGraph("Fan Speed", fanData, fanDataSize, yScale, animate);

        ImGui::Text("Fan Status: Active");
        ImGui::Text("Current Fan Speed: %.1f RPM", fanSpeed);

        ImGui::EndTabItem();
    }

    // Thermal Tab
    if (ImGui::BeginTabItem("Thermal")) {
        static float thermalData[100]; // Buffer for Thermal data
        static int thermalDataSize = 0;

        // Get current temperature
        float temperature =GetTemprature(); 

        // Store Temperature data for graph
        thermalData[thermalDataSize++ % 100] = temperature; // Store the Temperature in a circular buffer

        // Control the graph animation
        static float frameCountThermal = 0.0f;
        frameCountThermal += ImGui::GetIO().DeltaTime; // Increment frame count with time elapsed
        if (frameCountThermal >= 1.0f / fps && animate) {
            frameCountThermal = 0.0f; // Reset frame count
            thermalData[thermalDataSize % 100] = temperature; // Update Temperature if animating
        }

        RenderGraph("Temperature", thermalData, thermalDataSize, yScale, animate);

        ImGui::Text("Current Temperature: %.1f °C", temperature);

        ImGui::EndTabItem();
    }
    
    ImGui::EndTabBar();

    // Sliders for controlling FPS and Y scale
    ImGui::SliderFloat("Y Scale", &yScale, 0.0f, 200.0f);
    ImGui::SliderFloat("FPS", &fps, 1.0f, 60.0f);
}
