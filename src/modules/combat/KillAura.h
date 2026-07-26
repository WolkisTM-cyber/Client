#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <cmath>
#include <vector>

class KillAura : public Module {
public:
    KillAura() : Module("KillAura", "Kill Aura", Category::Combat, 0) {
        AddSetting(Setting::FloatSetting("Range", "Range", 4.2f, 1.0f, 6.0f));
        AddSetting(Setting::IntSetting("CPS", "CPS", 10, 1, 20));
        AddSetting(Setting::ModeSetting("Mode", "Mode", {"Single", "Switch", "Multi"}, 0));
        AddSetting(Setting::BoolSetting("Autoblock", "Auto Block", true));
    }

    void OnTick(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto world = JNIHelper::GetWorld(env);
        if (!world) { env->DeleteLocalRef(player); return; }

        auto& c = JNIHelper::Get();
        auto* rangeSetting = GetSetting("Range");
        auto* cpsSetting = GetSetting("CPS");
        auto* modeSetting = GetSetting("Mode");
        if (!rangeSetting || !cpsSetting || !modeSetting) {
            env->DeleteLocalRef(world); env->DeleteLocalRef(player); return;
        }

        double range = rangeSetting->fVal;
        int cps = cpsSetting->iVal;
        tick_++;
        if (tick_ < (20 / cps)) { env->DeleteLocalRef(world); env->DeleteLocalRef(player); return; }
        tick_ = 0;

        jobject entityList = env->CallObjectMethod(world, c.getLoadedEntityList);
        if (!entityList) { env->DeleteLocalRef(world); env->DeleteLocalRef(player); return; }

        double px = env->GetDoubleField(player, c.posX);
        double py = env->GetDoubleField(player, c.posY);
        double pz = env->GetDoubleField(player, c.posZ);

        jint size = env->CallIntMethod(entityList, c.listSize);
        std::vector<jobject> targets;

        for (int i = 0; i < size; i++) {
            jobject entity = env->CallObjectMethod(entityList, c.listGet, i);
            if (!entity || env->IsSameObject(entity, player)) {
                if (entity) env->DeleteLocalRef(entity);
                continue;
            }

            if (!env->CallBooleanMethod(entity, c.isEntityAlive)) {
                env->DeleteLocalRef(entity);
                continue;
            }

            double ex = env->GetDoubleField(entity, c.posX);
            double ey = env->GetDoubleField(entity, c.posY);
            double ez = env->GetDoubleField(entity, c.posZ);
            double dist = std::sqrt((ex-px)*(ex-px) + (ey-py)*(ey-py) + (ez-pz)*(ez-pz));

            if (dist <= range) {
                if (modeSetting->modeVal == 2) {
                    targets.push_back(env->NewGlobalRef(entity));
                } else if (modeSetting->modeVal == 1) {
                    targets.push_back(env->NewGlobalRef(entity));
                } else {
                    if (targets.empty()) targets.push_back(env->NewGlobalRef(entity));
                    else if (dist < range) {
                        env->DeleteGlobalRef(targets[0]);
                        targets[0] = env->NewGlobalRef(entity);
                    }
                }
            }
            env->DeleteLocalRef(entity);
        }

        env->DeleteLocalRef(entityList);

        for (auto& target : targets) {
            auto pc = JNIHelper::GetPlayerController(env);
            if (pc) {
                env->CallVoidMethod(pc, c.attackEntity, player, target);
                env->DeleteLocalRef(pc);
            }
            env->DeleteGlobalRef(target);
        }

        env->DeleteLocalRef(world);
        env->DeleteLocalRef(player);
    }

private:
    int tick_ = 0;
};
