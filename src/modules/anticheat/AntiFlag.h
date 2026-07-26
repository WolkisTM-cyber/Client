#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include "../ModuleManager.h"

class AntiFlag : public Module {
public:
    AntiFlag() : Module("AntiFlag", "Anti Flag", Category::Misc, 0) {
        AddSetting(Setting::BoolSetting("AutoDisable", "Auto Disable", true));
        AddSetting(Setting::BoolSetting("Alerts", "Alerts", true));
    }

    void OnTick(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto& c = JNIHelper::Get();

        // Detect anticheat checks:
        // 1. Server repeatedly setting position = force check
        // 2. Abrupt motion changes = flagged
        // 3. Invalid ground state

        double posX = env->GetDoubleField(player, c.posX);
        double posY = env->GetDoubleField(player, c.posY);
        double posZ = env->GetDoubleField(player, c.posZ);
        double motX = env->GetDoubleField(player, c.motionX);
        double motY = env->GetDoubleField(player, c.motionY);
        double motZ = env->GetDoubleField(player, c.motionZ);

        int check = 0;

        // Check 1: velocity reset (anticheat clearing motion)
        if (prevMotSet_) {
            if (fabs(motX) < 0.001 && fabs(prevMotX_) > 0.1) check++;
            if (fabs(motZ) < 0.001 && fabs(prevMotZ_) > 0.1) check++;
            if (motY < -0.1 && prevMotY_ > 0.1) check++; // Fall check
        }

        // Check 2: position rubberband (posY suddenly changes)
        if (prevPosSet_) {
            double dy = fabs(posY - prevPosY_);
            if (dy > 3.0) check += 2; // Rubberband
        }

        // Check 3: same position for too long (position freeze check)
        if (prevPosSet_) {
            double dx = fabs(posX - prevPosX_);
            double dz = fabs(posZ - prevPosZ_);
            if (dx < 0.001 && dz < 0.001) stuckTicks_++;
            else stuckTicks_ = 0;

            if (stuckTicks_ > 20) check += 3; // Suspicious freeze
        }

        prevMotX_ = motX; prevMotY_ = motY; prevMotZ_ = motZ;
        prevPosX_ = posX; prevPosY_ = posY; prevPosZ_ = posZ;
        prevMotSet_ = true;
        prevPosSet_ = true;

        if (check > 0) {
            auto* autoDisable = GetSetting("AutoDisable");
            if (autoDisable && autoDisable->bVal) {
                // Disable suspicious modules
                auto modules = g_moduleManager->GetAll();
                for (auto* mod : modules) {
                    if (mod->IsEnabled() && IsSuspicious(mod->GetCategory(), mod->GetName())) {
                        mod->Toggle(env);
                    }
                }
            }

            auto* alerts = GetSetting("Alerts");
            if (alerts && alerts->bVal) {
                jstring msg = env->NewStringUTF("[Client] AntiCheat flag detected!");
                if (msg) {
                    env->CallVoidMethod(player, c.sendChatMessage, msg);
                    if (env->ExceptionCheck()) env->ExceptionClear();
                    env->DeleteLocalRef(msg);
                }
            }
        }

        env->DeleteLocalRef(player);
    }

private:
    bool IsSuspicious(Category cat, const std::string& name) {
        if (cat == Category::Combat) return true;
        if (cat == Category::Movement) return true;
        if (name == "KillAura" || name == "Speed" || name == "Flight" ||
            name == "Velocity" || name == "NoFall") return true;
        return false;
    }

    double prevMotX_ = 0, prevMotY_ = 0, prevMotZ_ = 0;
    double prevPosX_ = 0, prevPosY_ = 0, prevPosZ_ = 0;
    bool prevMotSet_ = false, prevPosSet_ = false;
    int stuckTicks_ = 0;
};
