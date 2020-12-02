//===--------------------------------------------------------------------------------------------===
// settings.cpp - ImGui settings for HeadTrack
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include "htrack.h"
#include <ImgWindow/ImgWindow.h>
#include <algorithm>

extern "C" float settings_floop_cb(float, float, int, void* inRefcon);

class SettingsWindow : public ImgWindow {
public:
    static constexpr int num_hist = 50;
    SettingsWindow(int left, int top, int right, int bottom)
        : ImgWindow(left, top, right, bottom) {
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;

        SetWindowTitle("HeadTrack Settings");
        SetWindowResizingLimits(400, 400, 400, 800);

        std::fill(std::begin(head_hist), std::end(head_hist), 0.f);
        std::fill(std::begin(sim_hist), std::end(sim_hist), 0.f);
    }

    virtual ~SettingsWindow() {
    }

    void updateHistory() {
        static size_t offset = 6 * (num_hist - 1);

        memmove(head_hist, head_hist+6, (offset * sizeof(float)));
        memmove(sim_hist, sim_hist+6, (offset * sizeof(float)));

        float *head = head_hist + offset;
        float *sim = sim_hist + offset;
        for(int i = 0; i < 6; ++i) {
            head[i] = htk_settings.head[i];
            sim[i] = htk_settings.sim[i];
        }
    }

    virtual void buildInterface() override {
        float w = ImGui::GetWindowWidth();
        // float win_height = ImGui::GetWindowHeight();
        updateHistory();

        ImVec4 nice_pink = ImColor(255, 150, 200);
        ImVec4 light_grey = ImColor(0xffb4a0aa);
        ImVec4 yellow = ImColor(247, 170, 61);
        ImVec4 red = ImColor(0xff, 0x33, 0x33);

        float limits[6];

        for(int i = 0; i < 6; ++i) limits[i] = htk_settings.axes_limits[i];

        ImGui::Text("HeadTrack %s by Amy Alex Parent.", HTK_VERSION);
        ImGui::Dummy(ImVec2(0, 10.f));
        ImGui::PushStyleColor(ImGuiCol_Text, nice_pink);
        ImGui::TextWrapped("HeadTrack will always be free, but you can support my X-Plane work by buying me a coffee <3\nko-fi.com/amyinorbit");
        ImGui::PopStyleColor();
        ImGui::Dummy(ImVec2(0, 10.f));
        ImGui::Separator();

        if(htk_settings.last_error) {
            ImGui::PushStyleColor(ImGuiCol_Text, red);
            ImGui::TextWrapped("error: %s", htk_settings.last_error);
            ImGui::PopStyleColor();
        } else {
            ImGui::Text("Server is listening on 0.0.0.0:4242");
        }
        ImGui::Separator();

        if(ImGui::Button("Save Settings (Global)")) {
            settings_save(true);
        }
        ImGui::SameLine();
        if(ImGui::Button("Save Settings (Aircraft)")) {
            settings_save(false);
        }

        ImGui::Dummy(ImVec2(0, 10.f));
        if(ImGui::CollapsingHeader("Head Motion Limits")) {
            ImGui::TextColored(nice_pink, "Rotation");
            ImGui::Text("Head deflection for full in-sim rotation");
            ImGui::Separator();
            ImGui::Text("yaw");
            ImGui::SliderFloat("##yaw", &limits[3], 1, 90, "%.0f deg");
            ImGui::SameLine(); ImGui::Checkbox("Reverse##yaw", &htk_settings.axes_invert[3]);

            ImGui::Text("pitch");
            ImGui::SliderFloat("##pitch", &limits[4], 1, 90, "%.0f deg");
            ImGui::SameLine(); ImGui::Checkbox("Reverse##pitch", &htk_settings.axes_invert[4]);

            ImGui::Text("roll");
            ImGui::SliderFloat("##roll", &limits[5], 1, 180, "%.0f deg");
            ImGui::SameLine(); ImGui::Checkbox("Reverse##roll", &htk_settings.axes_invert[5]);

            ImGui::Dummy(ImVec2(0, 10.f));
            ImGui::TextColored(nice_pink, "Translation");
            ImGui::Text("Head displacement for full in-sim rotation");
            ImGui::Separator();
            ImGui::Text("X Axis");
            ImGui::SliderFloat("##x", &limits[0], 1, 100, "max: %.0f cm");
            ImGui::SameLine(); ImGui::Checkbox("Reverse##x", &htk_settings.axes_invert[0]);
            ImGui::Text("Y Axis");
            ImGui::SliderFloat("##y", &limits[1], 1, 100, "max: %.0f cm");
            ImGui::SameLine(); ImGui::Checkbox("Reverse##y", &htk_settings.axes_invert[1]);
            ImGui::Text("Z Axis");
            ImGui::SliderFloat("##z", &limits[2], 1, 100, "max: %.0f cm");
            ImGui::SameLine(); ImGui::Checkbox("Reverse##z", &htk_settings.axes_invert[2]);
            ImGui::Dummy(ImVec2(0, 10.f));
        }

        if(ImGui::CollapsingHeader("Smoothing and Sensitivity")) {
            ImGui::SliderFloat("Input Smoothing", &htk_settings.input_smooth, 0.f, 1.f, "%.2f");
            ImGui::PushStyleColor(ImGuiCol_Text, light_grey);
            ImGui::TextWrapped("Smoothing reduces jitter due to tracking, but increases input lag.");
            ImGui::PopStyleColor();
            ImGui::Dummy(ImVec2(0, 10.f));
            ImGui::SliderFloat("Rotation Expo", &htk_settings.rotation_smooth, 0.f, 1.f, "%.2f");
            ImGui::SliderFloat("Translation Expo", &htk_settings.translation_smooth, 0.f, 1.f, "%.2f");
            ImGui::PushStyleColor(ImGuiCol_Text, light_grey);
            ImGui::TextWrapped("Exponential gives you finer-grained control around the neutral position, at the expense of coarser movement at large head deflections.");
            ImGui::PopStyleColor();

            ImGui::Dummy(ImVec2(0, 10.f));
        }

        if(ImGui::CollapsingHeader("Tracking State")) {
            ImGui::Dummy(ImVec2(0, 10.f));
            ImGui::TextColored(nice_pink, "Input (Head)");
            ImGui::PushStyleColor(ImGuiCol_PlotLines, yellow);

            ImGui::PlotLines("##hyaw", head_hist + 3, num_hist, 0, "Yaw",
                -htk_settings.axes_limits[3], htk_settings.axes_limits[3],
                ImVec2(w/3.3, 30), 6 * sizeof(float));
            ImGui::SameLine();
            ImGui::PlotLines("##hpitch", head_hist + 4, num_hist, 0, "Pitch",
                -htk_settings.axes_limits[4], htk_settings.axes_limits[4],
                ImVec2(w/3.3, 30), 6 * sizeof(float));
            ImGui::SameLine();
            ImGui::PlotLines("##hroll", head_hist + 5, num_hist, 0, "Roll",
                -htk_settings.axes_limits[5], htk_settings.axes_limits[5],
                ImVec2(w/3.3, 30), 6 * sizeof(float));
            ImGui::PlotLines("##hx", head_hist + 0, num_hist, 0, "X",
                -htk_settings.axes_limits[0], htk_settings.axes_limits[0],
                ImVec2(w/3.3, 30), 6 * sizeof(float));
            ImGui::SameLine();
            ImGui::PlotLines("##hy", head_hist + 1, num_hist, 0, "Y",
                -htk_settings.axes_limits[1], htk_settings.axes_limits[1],
                ImVec2(w/3.3, 30), 6 * sizeof(float));
            ImGui::SameLine();
            ImGui::PlotLines("##hz", head_hist + 2, num_hist, 0, "Z",
                -htk_settings.axes_limits[2], htk_settings.axes_limits[2],
                ImVec2(w/3.3, 30), 6 * sizeof(float));

            ImGui::Dummy(ImVec2(0, 10.f));
            ImGui::TextColored(nice_pink, "Output (Sim)");
            ImGui::PlotLines("##syaw", sim_hist + 3, num_hist, 0, "Yaw",
                -135, 135, ImVec2(w/3.3, 30), 6 * sizeof(float));
            ImGui::SameLine();
            ImGui::PlotLines("##spitch", sim_hist + 4, num_hist, 0, "Pitch",
                -90, 90, ImVec2(w/3.3, 30), 6 * sizeof(float));
            ImGui::SameLine();
            ImGui::PlotLines("##sroll", sim_hist + 5, num_hist, 0, "Roll",
                -90, 90, ImVec2(w/3.3, 30), 6 * sizeof(float));

            ImGui::PlotLines("##sx", sim_hist + 0, num_hist, 0, "X",
                -100, 100, ImVec2(w/3.3, 30), 6 * sizeof(float));
            ImGui::SameLine();
            ImGui::PlotLines("##sy", sim_hist + 1, num_hist, 0, "Y",
                -100, 100, ImVec2(w/3.3, 30), 6 * sizeof(float));
            ImGui::SameLine();
            ImGui::PlotLines("##sz", sim_hist + 2, num_hist, 0, "Z",
                -100, 100, ImVec2(w/3.3, 30), 6 * sizeof(float));

            ImGui::Dummy(ImVec2(0, 10.f));
            ImGui::PopStyleColor();
        }


        ImGui::Dummy(ImVec2(0, 10.f));
        ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Text, nice_pink);
        ImGui::TextWrapped("Made in Scotland with <3\nTrans Rights = Human Rights");
        ImGui::PopStyleColor();


        for(int i = 0; i < 6; ++i) htk_settings.axes_limits[i] = limits[i];
    }
private:
    float sim_hist[6 * num_hist];
    float head_hist[6 * num_hist];
};

SettingsWindow* window = nullptr;

extern "C" {

    void settings_show() {
        if(!window) {
            window = new SettingsWindow(20, 20, 0, 0);
        }
        window->SetVisible(!window->GetVisible());
    }

    bool settings_is_visible() {
        return window && window->GetVisible() && window->IsWindowInFront();
    }

    void settings_cleanup() {
        if(window) delete window;
    }
}
