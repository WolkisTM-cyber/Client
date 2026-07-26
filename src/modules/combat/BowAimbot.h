#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <cmath>

class BowAimbot : public Module {
public:
    BowAimbot() : Module("BowAimbot", "Bow Aimbot", Category::Combat, 0) {
        AddSetting(Setting::FloatSetting("Range", "Range", 50.0f, 10.0f, 200.0f));
        AddSetting(Setting::BoolSetting("Predict", "Predict Motion", true));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto mc = JNIHelper::GetMinecraft(env);
        if (!mc) { env->DeleteLocalRef(player); return; }

        auto& c = JNIHelper::Get();
        jobject heldItem = env->CallObjectMethod(player, c.getHeldItem);
        if (!heldItem) { env->DeleteLocalRef(mc); env->DeleteLocalRef(player); return; }

        jobject item = env->CallObjectMethod(heldItem, c.getItemFromStack);
        if (!item) { env->DeleteLocalRef(heldItem); env->DeleteLocalRef(mc); env->DeleteLocalRef(player); return; }

        jstring bowName = env->NewStringUTF("bow");
        jmethodID getUnlocalized = c.getUnlocalizedName;
        jstring nameObj = (jstring)env->CallObjectMethod(item, getUnlocalized);
        if (!nameObj) { env->DeleteLocalRef(bowName); env->DeleteLocalRef(item); env->DeleteLocalRef(heldItem); env->DeleteLocalRef(mc); env->DeleteLocalRef(player); return; }

        const char* nameStr = env->GetStringUTFChars(nameObj, nullptr);
        bool isBow = nameStr && strstr(nameStr, "bow") != nullptr;
        env->ReleaseStringUTFChars(nameObj, nameStr);
        env->DeleteLocalRef(nameObj);
        env->DeleteLocalRef(bowName);
        env->DeleteLocalRef(item);
        env->DeleteLocalRef(heldItem);

        if (!isBow) { env->DeleteLocalRef(mc); env->DeleteLocalRef(player); return; }

        bool isCharging = false;
        jmethodID getItemInUseCount = env->GetMethodID(c.entityPlayer, "getItemInUseCount", "()I");
        if (getItemInUseCount) {
            int count = env->CallIntMethod(player, getItemInUseCount);
            if (env->ExceptionCheck()) env->ExceptionClear();
            if (count > 0) {
                auto world = JNIHelper::GetWorld(env);
                if (world) {
                    if (target_) { env->DeleteGlobalRef(target_); target_ = nullptr; }
                    target_ = FindBestTarget(env, player, world);
                    env->DeleteLocalRef(world);
                }
                if (target_) {
                    float yaw = GetAngleTo(env, player, target_);
                    float pitch = GetPitchTo(env, player, target_);
                    env->SetFloatField(player, c.rotationYaw, yaw);
                    env->SetFloatField(player, c.rotationPitch, pitch);
                }
            }
        }

        env->DeleteLocalRef(mc);
        env->DeleteLocalRef(player);
    }

    jobject FindBestTarget(JNIEnv* env, jobject player, jobject world) {
        auto& c = JNIHelper::Get();
        jobject entities = env->CallObjectMethod(world, c.getLoadedEntityList);
        if (!entities) return nullptr;

        double px = env->GetDoubleField(player, c.posX);
        double py = env->GetDoubleField(player, c.posY);
        double pz = env->GetDoubleField(player, c.posZ);

        jint size = env->CallIntMethod(entities, c.listSize);
        jobject best = nullptr;
        double bestDist = GetSetting("Range")->fVal;

        for (int i = 0; i < size && i < 100; i++) {
            jobject ent = env->CallObjectMethod(entities, c.listGet, i);
            if (!ent || env->ExceptionCheck()) { if (ent) env->DeleteLocalRef(ent); env->ExceptionClear(); continue; }
            if (env->IsSameObject(ent, player)) { env->DeleteLocalRef(ent); continue; }

            double ex = env->GetDoubleField(ent, c.posX);
            double ey = env->GetDoubleField(ent, c.posY);
            double ez = env->GetDoubleField(ent, c.posZ);
            double dist = std::sqrt((ex-px)*(ex-px) + (ey-py)*(ey-py) + (ez-pz)*(ez-pz));

            if (dist < bestDist) {
                if (best) env->DeleteLocalRef(best);
                best = env->NewGlobalRef(ent);
                bestDist = dist;
            }
            env->DeleteLocalRef(ent);
        }

        env->DeleteLocalRef(entities);
        return best;
    }

    float GetAngleTo(JNIEnv* env, jobject from, jobject to) {
        auto& c = JNIHelper::Get();
        double px = env->GetDoubleField(from, c.posX);
        double pz = env->GetDoubleField(from, c.posZ);
        double ex = env->GetDoubleField(to, c.posX);
        double ez = env->GetDoubleField(to, c.posZ);
        double dx = ex - px;
        double dz = ez - pz;
        float yaw = (float)(atan2(dz, dx) * 180.0 / 3.14159) - 90.0f;
        return yaw;
    }

    float GetPitchTo(JNIEnv* env, jobject from, jobject to) {
        auto& c = JNIHelper::Get();
        double px = env->GetDoubleField(from, c.posX);
        double py = env->GetDoubleField(from, c.posY);
        double pz = env->GetDoubleField(from, c.posZ);
        double ex = env->GetDoubleField(to, c.posX);
        double ey = env->GetDoubleField(to, c.posY) + 1.0;
        double ez = env->GetDoubleField(to, c.posZ);
        double dx = ex - px;
        double dy = ey - py;
        double dz = ez - pz;
        float pitch = (float)(atan2(dy, sqrt(dx*dx + dz*dz)) * 180.0 / 3.14159);
        return pitch;
    }

private:
    jobject target_ = nullptr;
};
