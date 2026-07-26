#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <cmath>

class TargetHUD : public Module {
public:
    TargetHUD() : Module("TargetHUD", "Target HUD", Category::Visual, 0) {
        AddSetting(Setting::ModeSetting("Style", "Style", {"Default", "Compact", "Hypixel"}, 0));
    }

    void OnTick(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto world = JNIHelper::GetWorld(env);
        if (!world) { env->DeleteLocalRef(player); return; }

        auto& c = JNIHelper::Get();
        jobject entityList = env->CallObjectMethod(world, c.getLoadedEntityList);
        if (!entityList || env->ExceptionCheck()) {
            env->ExceptionClear();
            env->DeleteLocalRef(world);
            env->DeleteLocalRef(player);
            return;
        }

        double px = env->GetDoubleField(player, c.posX);
        double py = env->GetDoubleField(player, c.posY);
        double pz = env->GetDoubleField(player, c.posZ);

        jint size = env->CallIntMethod(entityList, c.listSize);
        if (env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(entityList); env->DeleteLocalRef(world); env->DeleteLocalRef(player); return; }

        jobject closest = nullptr;
        double closestDist = 1000.0;

        for (int i = 0; i < size && i < 100; i++) {
            jobject entity = env->CallObjectMethod(entityList, c.listGet, i);
            if (!entity || env->ExceptionCheck()) {
                if (entity) env->DeleteLocalRef(entity);
                env->ExceptionClear();
                continue;
            }
            if (env->IsSameObject(entity, player)) { env->DeleteLocalRef(entity); continue; }

            double ex = env->GetDoubleField(entity, c.posX);
            double ey = env->GetDoubleField(entity, c.posY);
            double ez = env->GetDoubleField(entity, c.posZ);
            double dist = std::sqrt((ex-px)*(ex-px) + (ey-py)*(ey-py) + (ez-pz)*(ez-pz));

            // Check if in crosshair area
            float yaw = env->GetFloatField(player, c.rotationYaw);
            float pitch = env->GetFloatField(player, c.rotationPitch);
            double dx = ex - px;
            double dz = ez - pz;
            double angleToTarget = atan2(dz, dx) * 180.0 / 3.14159 - yaw;
            while (angleToTarget > 180) angleToTarget -= 360;
            while (angleToTarget < -180) angleToTarget += 360;
            double angleDiff = sqrt(angleToTarget * angleToTarget);

            if (dist < closestDist && angleDiff < 30.0) {
                if (closest) env->DeleteLocalRef(closest);
                closest = env->NewGlobalRef(entity);
                closestDist = dist;
            }

            env->DeleteLocalRef(entity);
        }

        env->DeleteLocalRef(entityList);

        // Update target
        if (target_) env->DeleteGlobalRef(target_);
        target_ = closest;
        targetDist_ = closestDist;

        if (target_) {
            targetHP_ = 20.0;
            jmethodID getHealth = env->GetMethodID(
                JNIHelper::Get().entityLivingBase, "getHealth", "()F");
            if (getHealth) {
                targetHP_ = env->CallFloatMethod(target_, getHealth);
                if (env->ExceptionCheck()) env->ExceptionClear();
            }

            jmethodID getName = env->GetMethodID(
                env->GetObjectClass(target_), "getName", "()Ljava/lang/String;");
            if (getName) {
                jstring nameObj = (jstring)env->CallObjectMethod(target_, getName);
                if (nameObj) {
                    const char* nameStr = env->GetStringUTFChars(nameObj, nullptr);
                    if (nameStr) targetName_ = nameStr;
                    env->ReleaseStringUTFChars(nameObj, nameStr);
                    env->DeleteLocalRef(nameObj);
                }
            }
        }

        env->DeleteLocalRef(world);
        env->DeleteLocalRef(player);
    }

    void Render(JNIEnv* env, jobject fr, jmethodID drawStr) {
        if (!target_) return;

        // Animate HP bar
        animatedHP_ += (targetHP_ - animatedHP_) * 0.15f;

        char buf[128];
        snprintf(buf, sizeof(buf), "%s HP: %.1f [%.1fm]",
                 targetName_.c_str(), targetHP_, targetDist_);

        jstring text = env->NewStringUTF(buf);
        if (text && drawStr) {
            int color = (animatedHP_ > 15.0f) ? 0x55FF55 : ((animatedHP_ > 7.0f) ? 0xFFAA00 : 0xFF5555);
            env->CallIntMethod(fr, drawStr, text, 4, 100, color);
            if (env->ExceptionCheck()) env->ExceptionClear();
            env->DeleteLocalRef(text);
        }
    }

    void OnDisable(JNIEnv* env) override {
        if (target_) { env->DeleteGlobalRef(target_); target_ = nullptr; }
    }

private:
    jobject target_ = nullptr;
    double targetDist_ = 0;
    float targetHP_ = 20.0f;
    float animatedHP_ = 20.0f;
    std::string targetName_;
};
