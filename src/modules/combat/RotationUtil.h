#pragma once
#include <cmath>
#include <algorithm>
#include <jni.h>
#include "../JNIHelper.h"

struct Vector3D {
    double x, y, z;
};

struct Rotations {
    float yaw;
    float pitch;
};

class RotationUtil {
public:
    static Rotations CalculateRotations(const Vector3D& src, const Vector3D& dst) {
        double diffX = dst.x - src.x;
        double diffY = dst.y - src.y;
        double diffZ = dst.z - src.z;

        double distXZ = std::sqrt(diffX * diffX + diffZ * diffZ);

        float yaw = (float)(std::atan2(diffZ, diffX) * 180.0 / 3.14159265358979323846) - 90.0f;
        float pitch = (float)(-(std::atan2(diffY, distXZ) * 180.0 / 3.14159265358979323846));

        return { WrapAngleTo180(yaw), std::clamp(pitch, -90.0f, 90.0f) };
    }

    static float WrapAngleTo180(float angle) {
        angle = fmodf(angle + 180.0f, 360.0f);
        if (angle < 0.0f) angle += 360.0f;
        return angle - 180.0f;
    }

    static Rotations ApplyGCD(const Rotations& current, const Rotations& target, float mouseSensitivity = 0.5f) {
        float f = mouseSensitivity * 0.6f + 0.2f;
        float gcd = f * f * f * 1.2f;

        float deltaYaw = target.yaw - current.yaw;
        float deltaPitch = target.pitch - current.pitch;

        deltaYaw = WrapAngleTo180(deltaYaw);

        float fixedYaw = current.yaw + std::round(deltaYaw / gcd) * gcd;
        float fixedPitch = std::clamp(current.pitch + std::round(deltaPitch / gcd) * gcd, -90.0f, 90.0f);

        return { WrapAngleTo180(fixedYaw), fixedPitch };
    }

    static void SetSilentRotation(JNIEnv* env, jobject player, float yaw, float pitch) {
        if (!env || !player) return;
        auto& c = JNIHelper::Get();
        jclass playerClass = env->GetObjectClass(player);
        if (!playerClass) return;

        jfieldID rotationYaw = env->GetFieldID(playerClass, "rotationYaw", "F");
        jfieldID rotationPitch = env->GetFieldID(playerClass, "rotationPitch", "F");

        if (rotationYaw && rotationPitch) {
            env->SetFloatField(player, rotationYaw, yaw);
            env->SetFloatField(player, rotationPitch, pitch);
        }

        env->DeleteLocalRef(playerClass);
    }
};
