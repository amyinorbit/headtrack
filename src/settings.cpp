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
#include <tgmath.h>

static const double limits_out[6] = {100, 100, 100, 135, 90, 90};

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

        float sensitivity[6];

        for(int i = 0; i < 6; ++i) {
            // sensitivity[i] = pow(htk_settings.axes_sens[i], 0.5);
            sensitivity[i] = htk_settings.axes_sens[i];
        }

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
        if(ImGui::CollapsingHeader("Motion Sensitivity")) {
            ImGui::TextColored(nice_pink, "Rotation");
            ImGui::Separator();
            ImGui::Text("yaw");
            ImGui::SliderFloat("##yaw", &sensitivity[3], 0.1, 5, "%.2fx", 2.f);
            ImGui::SameLine(); ImGui::Checkbox("Reverse##yaw", &htk_settings.axes_invert[3]);

            ImGui::Text("pitch");
            ImGui::SliderFloat("##pitch", &sensitivity[4], 0.1, 5, "%.2fx", 2.f);
            ImGui::SameLine(); ImGui::Checkbox("Reverse##pitch", &htk_settings.axes_invert[4]);

            ImGui::Text("roll");
            ImGui::SliderFloat("##roll", &sensitivity[5], 0.1, 5, "%.2fx", 2.f);
            ImGui::SameLine(); ImGui::Checkbox("Reverse##roll", &htk_settings.axes_invert[5]);

            ImGui::Dummy(ImVec2(0, 10.f));
            ImGui::TextColored(nice_pink, "Translation");
            ImGui::Separator();
            ImGui::Text("X Axis");
            ImGui::SliderFloat("##x", &sensitivity[0], 0.1, 5, "%.2fx", 2.f);
            ImGui::SameLine(); ImGui::Checkbox("Reverse##x", &htk_settings.axes_invert[0]);
            ImGui::Text("Y Axis");
            ImGui::SliderFloat("##y", &sensitivity[1], 0.1, 5, "%.2fx", 2.f);
            ImGui::SameLine(); ImGui::Checkbox("Reverse##y", &htk_settings.axes_invert[1]);
            ImGui::Text("Z Axis");
            ImGui::SliderFloat("##z", &sensitivity[2], 0.1, 5, "%.2fx", 2.f);
            ImGui::SameLine(); ImGui::Checkbox("Reverse##z", &htk_settings.axes_invert[2]);
            ImGui::Dummy(ImVec2(0, 10.f));
        }

        if(ImGui::CollapsingHeader("Smoothing and Sensitivity")) {
            ImGui::Text("Input Smoothing");
            ImGui::SliderFloat("##input_smoothing", &htk_settings.input_smooth, 0.f, 1.f, "%.2f");
            ImGui::PushStyleColor(ImGuiCol_Text, light_grey);
            ImGui::TextWrapped("Input smoothing reduces jitter due to tracking, but increases input lag.");
            ImGui::PopStyleColor();
            ImGui::Dummy(ImVec2(0, 10.f));
            ImGui::Text("Rotation Response");
            ImGui::SliderFloat("##exp_rotation", &htk_settings.rotation_smooth, 0.f, 1.f, "%.2f");
            ImGui::Text("Translation Response");
            ImGui::SliderFloat("##exp_translation", &htk_settings.translation_smooth, 0.f, 1.f, "%.2f");
            ImGui::PushStyleColor(ImGuiCol_Text, light_grey);
            ImGui::TextWrapped("Higher response values give you finer-grained control around the neutral position, at the expense of coarser movement at large head deflections.");
            ImGui::PopStyleColor();

            ImGui::Dummy(ImVec2(0, 10.f));
        }

        if(ImGui::CollapsingHeader("Tracking State")) {
            ImGui::Dummy(ImVec2(0, 10.f));
            ImGui::TextColored(nice_pink, "Input (Head)");
            ImGui::PushStyleColor(ImGuiCol_PlotLines, yellow);

            float plot_limits[6];
            for(int i = 0; i < 6; ++i) {
                plot_limits[i] = limits_out[i] / htk_settings.axes_sens[i];
            }

            ImGui::PlotLines("##hyaw", head_hist + 3, num_hist, 0, "Yaw",
                -plot_limits[3], plot_limits[3],
                ImVec2(w/3.3, 30), 6 * sizeof(float));
            ImGui::SameLine();
            ImGui::PlotLines("##hpitch", head_hist + 4, num_hist, 0, "Pitch",
                -plot_limits[4], plot_limits[4],
                ImVec2(w/3.3, 30), 6 * sizeof(float));
            ImGui::SameLine();
            ImGui::PlotLines("##hroll", head_hist + 5, num_hist, 0, "Roll",
                -plot_limits[5], plot_limits[5],
                ImVec2(w/3.3, 30), 6 * sizeof(float));
            ImGui::PlotLines("##hx", head_hist + 0, num_hist, 0, "X",
                -plot_limits[0], plot_limits[0],
                ImVec2(w/3.3, 30), 6 * sizeof(float));
            ImGui::SameLine();
            ImGui::PlotLines("##hy", head_hist + 1, num_hist, 0, "Y",
                -plot_limits[1], plot_limits[1],
                ImVec2(w/3.3, 30), 6 * sizeof(float));
            ImGui::SameLine();
            ImGui::PlotLines("##hz", head_hist + 2, num_hist, 0, "Z",
                -plot_limits[2], plot_limits[2],
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


        for(int i = 0; i < 6; ++i)
            htk_settings.axes_sens[i] = sensitivity[i];

        htk_settings_did_update();
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
