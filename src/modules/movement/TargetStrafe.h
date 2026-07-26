#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <cmath>

class TargetStrafe : public Module {
public:
    TargetStrafe() : Module("TargetStrafe", "Target Strafe", Category::Movement, 0) {
        AddSetting(Setting::FloatSetting("Radius", "Radius", 2.5f, 1.0f, 6.0f));
        AddSetting(Setting::FloatSetting("Speed", "Speed", 0.3f, 0.1f, 1.0f));
        AddSetting(Setting::BoolSetting("Jump", "Auto Jump", true));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto world = JNIHelper::GetWorld(env);
        if (!world) { env->DeleteLocalRef(player); return; }

        auto& c = JNIHelper::Get();
        jobject entities = env->CallObjectMethod(world, c.getLoadedEntityList);
        if (!entities || env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(world); env->DeleteLocalRef(player); return; }

        double px = env->GetDoubleField(player, c.posX);
        double pz = env->GetDoubleField(player, c.posZ);
        bool onGround = env->GetBooleanField(player, c.onGround);

        jint size = env->CallIntMethod(entities, c.listSize);
        jobject target = nullptr;
        double bestDist = 6.0;

        for (int i = 0; i < size && i < 50; i++) {
            jobject ent = env->CallObjectMethod(entities, c.listGet, i);
            if (!ent || env->ExceptionCheck()) { if (ent) env->DeleteLocalRef(ent); env->ExceptionClear(); continue; }
            if (env->IsSameObject(ent, player)) { env->DeleteLocalRef(ent); continue; }

            double ex = env->GetDoubleField(ent, c.posX);
            double ez = env->GetDoubleField(ent, c.posZ);
            double dist = sqrt((ex-px)*(ex-px) + (ez-pz)*(ez-pz));

            if (dist < bestDist) {
                if (target) env->DeleteLocalRef(target);
                target = env->NewGlobalRef(ent);
                bestDist = dist;
            }
            env->DeleteLocalRef(ent);
        }
        env->DeleteLocalRef(entities);

        if (target) {
            double tx = env->GetDoubleField(target, c.posX);
            double tz = env->GetDoubleField(target, c.posZ);
            env->DeleteLocalRef(target);

            // Circle strafe
            double dx = tx - px;
            double dz = tz - pz;
            double dist = sqrt(dx*dx + dz*dz);
            if (dist < 0.1) { env->DeleteLocalRef(world); env->DeleteLocalRef(player); return; }

            float radius = GetSetting("Radius")->fVal;
            float speed = GetSetting("Speed")->fVal;
            bool autoJump = GetSetting("Jump")->bVal;

            // Tangent direction
            double nx = -dz / dist;
            double nz = dx / dist;
            angle_ += speed;
            double circleX = tx + nx * radius * cos(angle_);
            double circleZ = tz + nz * radius * sin(angle_);

            double moveX = circleX - px;
            double moveZ = circleZ - pz;
            double moveDist = sqrt(moveX*moveX + moveZ*moveZ);
            if (moveDist > 0.1) {
                float yaw = (float)(atan2(moveZ, moveX) * 180.0 / 3.14159) - 90.0f;
                env->SetFloatField(player, c.rotationYaw, yaw);
                env->SetDoubleField(player, c.motionX, moveX / moveDist * speed);
                env->SetDoubleField(player, c.motionZ, moveZ / moveDist * speed);
            }

            if (autoJump && onGround) {
                env->SetDoubleField(player, c.motionY, 0.42);
            }
        }

        env->DeleteLocalRef(world);
        env->DeleteLocalRef(player);
    }

private:
    double angle_ = 0.0;
};
