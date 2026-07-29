#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include "../../render/Renderer.h"
#include <shlobj.h>
#include <GL/gl.h>
#include <vector>
#include <fstream>
#include <string>
#include <cmath>

struct Waypoint {
    std::string name;
    double x, y, z;
    int r, g, b;
};

class Waypoints : public Module {
public:
    Waypoints() : Module("Waypoints", "Waypoints", Category::World, 0) {}

    void OnEnable(JNIEnv* env) override {
        Load();
    }

    void AddWaypoint(const std::string& name, double x, double y, double z,
                     int r = 255, int g = 255, int b = 255) {
        waypoints_.push_back({name, x, y, z, r, g, b});
        Save();
    }

    void RemoveWaypoint(const std::string& name) {
        for (size_t i = 0; i < waypoints_.size(); i++) {
            if (waypoints_[i].name == name) {
                waypoints_.erase(waypoints_.begin() + i);
                Save();
                return;
            }
        }
    }

    void Render3D(JNIEnv* env) {
        if (!IsEnabled() || waypoints_.empty()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto& c = JNIHelper::Get();

        double px = env->GetDoubleField(player, c.posX);
        double py = env->GetDoubleField(player, c.posY);
        double pz = env->GetDoubleField(player, c.posZ);

        auto* renderer = Renderer::GetInstance();
        renderer->Setup3DProjection();

        glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glLineWidth(1.0f);

        for (auto& wp : waypoints_) {
            double dx = wp.x - px;
            double dy = wp.y - py;
            double dz = wp.z - pz;
            double dist = sqrt(dx*dx + dy*dy + dz*dz);
            if (dist > 500) continue;

            // Beam
            glBegin(GL_LINES);
            glColor4f(wp.r/255.0f, wp.g/255.0f, wp.b/255.0f, 0.5f);
            glVertex3d(wp.x, wp.y, wp.z);
            glColor4f(wp.r/255.0f, wp.g/255.0f, wp.b/255.0f, 0.0f);
            glVertex3d(wp.x, wp.y - 100, wp.z);
            glEnd();

            // Box
            glColor4f(wp.r/255.0f, wp.g/255.0f, wp.b/255.0f, 0.8f);
            glBegin(GL_LINE_LOOP);
            glVertex3d(wp.x - 0.5, wp.y, wp.z - 0.5);
            glVertex3d(wp.x + 0.5, wp.y, wp.z - 0.5);
            glVertex3d(wp.x + 0.5, wp.y + 1, wp.z - 0.5);
            glVertex3d(wp.x - 0.5, wp.y + 1, wp.z - 0.5);
            glEnd();
            glBegin(GL_LINE_LOOP);
            glVertex3d(wp.x - 0.5, wp.y, wp.z + 0.5);
            glVertex3d(wp.x + 0.5, wp.y, wp.z + 0.5);
            glVertex3d(wp.x + 0.5, wp.y + 1, wp.z + 0.5);
            glVertex3d(wp.x - 0.5, wp.y + 1, wp.z + 0.5);
            glEnd();
        }

        glPopAttrib();
        renderer->RestoreProjection();
        env->DeleteLocalRef(player);
    }

    std::vector<Waypoint>& GetWaypoints() { return waypoints_; }

private:
    void Load() {
        waypoints_.clear();
        wchar_t appData[MAX_PATH];
        if (FAILED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, appData))) return;

        std::string path = std::string(appData, appData + wcslen(appData)) + "\\Client\\waypoints.txt";
        std::ifstream file(path);
        if (!file.is_open()) return;

        std::string line;
        while (std::getline(file, line)) {
            char name[64]; double x,y,z; int r,g,b;
            if (sscanf_s(line.c_str(), "%63[^,],%lf,%lf,%lf,%d,%d,%d",
                         name, (unsigned)sizeof(name), &x, &y, &z, &r, &g, &b) >= 7) {
                waypoints_.push_back({name, x, y, z, r, g, b});
            }
        }
    }

    void Save() {
        wchar_t appData[MAX_PATH];
        if (FAILED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, appData))) return;

        std::string path = std::string(appData, appData + wcslen(appData)) + "\\Client\\waypoints.txt";
        SHCreateDirectoryExW(nullptr, std::wstring(
            path.begin(), path.end()).c_str(), nullptr);

        std::ofstream file(path);
        if (!file.is_open()) return;

        for (auto& wp : waypoints_) {
            file << wp.name << "," << wp.x << "," << wp.y << "," << wp.z
                 << "," << wp.r << "," << wp.g << "," << wp.b << std::endl;
        }
    }

    std::vector<Waypoint> waypoints_;
};
