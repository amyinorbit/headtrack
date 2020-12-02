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

extern "C" float settings_floop_cb(float, float, int, void* inRefcon);

class SettingsWindow : public ImgWindow {
public:
    SettingsWindow(int left, int top, int right, int bottom)
        : ImgWindow(left, top, right, bottom) {
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        // ImGuiStyle& style = ImGui::GetStyle();

        SetWindowTitle("HeadTrack Settings");
        SetWindowResizingLimits(400, 400, 512, 512);
    }

    virtual ~SettingsWindow() {
    }

    virtual void buildInterface() override {
        // float win_width = ImGui::GetWindowWidth();
        // float win_height = ImGui::GetWindowHeight();

        float limits[6];

        for(int i = 0; i < 6; ++i) limits[i] = htk_settings.axes_limits[i];

        ImGui::BeginGroup();
        ImGui::Text("Head Motion Range");

        if(ImGui::CollapsingHeader("Rotation Range")) {
            ImGui::Text("yaw");
            ImGui::SameLine(60.f); ImGui::SliderFloat("##yaw", &limits[3], 1, 90, "%.0f deg");
            ImGui::SameLine(); ImGui::Checkbox("Invert##yaw", &htk_settings.axes_invert[3]);

            ImGui::Text("pitch");
            ImGui::SameLine(60.f); ImGui::SliderFloat("##pitch", &limits[4], 1, 90, "%.0f deg");
            ImGui::SameLine(); ImGui::Checkbox("Invert##pitch", &htk_settings.axes_invert[4]);

            ImGui::Text("roll");
            ImGui::SameLine(60.f); ImGui::SliderFloat("##roll", &limits[5], 1, 180, "%.0f deg");
            ImGui::SameLine(); ImGui::Checkbox("Invert##roll", &htk_settings.axes_invert[5]);
        }

        if(ImGui::CollapsingHeader("Translation Range")) {
            ImGui::Text("x");
            ImGui::SameLine(60.f) ; ImGui::SliderFloat("##x", &limits[0], 1, 100, "max: %.0f cm");
            ImGui::SameLine(); ImGui::Checkbox("Invert##x", &htk_settings.axes_invert[0]);
            ImGui::Text("y");
            ImGui::SameLine(60.f); ImGui::SliderFloat("##y", &limits[1], 1, 100, "max: %.0f cm");
            ImGui::SameLine(); ImGui::Checkbox("Invert##y", &htk_settings.axes_invert[1]);
            ImGui::Text("z");
            ImGui::SameLine(60.f); ImGui::SliderFloat("##z", &limits[2], 1, 100, "max: %.0f cm");
            ImGui::SameLine(); ImGui::Checkbox("Invert##z", &htk_settings.axes_invert[2]);
        }
        ImGui::EndGroup();

        ImGui::BeginGroup();
        if(ImGui::CollapsingHeader("Stability Augmentation")) {
            ImGui::SliderFloat("rotation", &htk_settings.rotation_smooth, 0.f, 1.f, "%.2f");
            ImGui::SliderFloat("translation", &htk_settings.translation_smooth, 0.f, 1.f, "%.2f");
        }
        ImGui::EndGroup();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::BeginGroup();
        ImGui::Text("HeadTrack 2012.1 copyright (c) Amy Alex Parent.");
        ImGui::EndGroup();



        for(int i = 0; i < 6; ++i) htk_settings.axes_limits[i] = limits[i];
    }

};

SettingsWindow* window = nullptr;

extern "C" {

    void settings_show() {
        if(!window) {
            window = new SettingsWindow(0, 0, 0, 0);
        }
        window->SetVisible(!window->GetVisible());
    }

    bool settings_is_visible() {
        return window && window->GetVisible();
    }

    void settings_cleanup() {
        if(window) delete window;
    }
}
