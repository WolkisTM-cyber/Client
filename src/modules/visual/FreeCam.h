#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class FreeCam : public Module {
public:
    FreeCam() : Module("FreeCam", "Free Cam", Category::Visual, 0) {
        AddSetting(Setting::FloatSetting("Speed", "Speed", 0.5f, 0.1f, 2.0f));
    }

    void OnEnable(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto& c = JNIHelper::Get();

        oldX_ = env->GetDoubleField(player, c.posX);
        oldY_ = env->GetDoubleField(player, c.posY);
        oldZ_ = env->GetDoubleField(player, c.posZ);
        oldYaw_ = env->GetFloatField(player, c.rotationYaw);
        oldPitch_ = env->GetFloatField(player, c.rotationPitch);
        oldOnGround_ = env->GetBooleanField(player, c.onGround);
        wasFlying_ = false;

        // Save capabilities
        jobject caps = env->GetObjectField(player, c.capabilities);
        if (caps) {
            wasFlying_ = env->GetBooleanField(caps, c.isFlyingF);
            env->DeleteLocalRef(caps);
        }

        env->DeleteLocalRef(player);
    }

    void OnDisable(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto& c = JNIHelper::Get();

        // Teleport back
        env->SetDoubleField(player, c.posX, oldX_);
        env->SetDoubleField(player, c.posY, oldY_);
        env->SetDoubleField(player, c.posZ, oldZ_);
        env->SetFloatField(player, c.rotationYaw, oldYaw_);
        env->SetFloatField(player, c.rotationPitch, oldPitch_);

        // Restore motion
        env->SetDoubleField(player, c.motionX, 0);
        env->SetDoubleField(player, c.motionY, 0);
        env->SetDoubleField(player, c.motionZ, 0);

        // NoClip off
        jfieldID noClipField = env->GetFieldID(c.entityPlayer, "noClip", "Z");
        if (noClipField) env->SetBooleanField(player, noClipField, false);

        env->DeleteLocalRef(player);
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto& c = JNIHelper::Get();

        // NoClip
        jfieldID noClipField = env->GetFieldID(c.entityPlayer, "noClip", "Z");
        if (noClipField) env->SetBooleanField(player, noClipField, true);

        // Flying motion
        float speed = GetSetting("Speed")->fVal;
        float yaw = oldYaw_;
        float pitch = oldPitch_;

        double mx = 0, my = 0, mz = 0;
        if (GetAsyncKeyState('W') & 0x8000) {
            mx += -sin(yaw * 3.14159 / 180.0) * speed;
            mz += cos(yaw * 3.14159 / 180.0) * speed;
        }
        if (GetAsyncKeyState('S') & 0x8000) {
            mx += sin(yaw * 3.14159 / 180.0) * speed;
            mz += -cos(yaw * 3.14159 / 180.0) * speed;
        }
        if (GetAsyncKeyState('A') & 0x8000) {
            mx += -sin((yaw - 90) * 3.14159 / 180.0) * speed;
            mz += cos((yaw - 90) * 3.14159 / 180.0) * speed;
        }
        if (GetAsyncKeyState('D') & 0x8000) {
            mx += -sin((yaw + 90) * 3.14159 / 180.0) * speed;
            mz += cos((yaw + 90) * 3.14159 / 180.0) * speed;
        }
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) my += speed;
        if (GetAsyncKeyState(VK_SHIFT) & 0x8000) my -= speed;

        // Update camera position (not player)
        oldX_ += mx;
        oldY_ += my;
        oldZ_ += mz;

        // Keep player frozen
        env->SetDoubleField(player, c.motionX, 0);
        env->SetDoubleField(player, c.motionY, 0);
        env->SetDoubleField(player, c.motionZ, 0);
        env->SetBooleanField(player, c.onGround, false);

        // Fake position for camera (we'd use RenderManager.viewerPosX etc.)
        // For simplicity: set noClip and fly
        env->SetDoubleField(player, c.posX, oldX_);
        env->SetDoubleField(player, c.posY, oldY_);
        env->SetDoubleField(player, c.posZ, oldZ_);

        env->DeleteLocalRef(player);
    }

    void OnUpdate(JNIEnv* env) {
        // Block movement input
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto& c = JNIHelper::Get();

        jobject movementInput = env->GetObjectField(player, c.movementInputF);
        if (movementInput) {
            env->SetFloatField(movementInput, c.moveForward, 0);
            env->SetFloatField(movementInput, c.moveStrafe, 0);
            jfieldID jumpField = env->GetFieldID(env->GetObjectClass(movementInput), "jump", "Z");
            jfieldID sneakField = env->GetFieldID(env->GetObjectClass(movementInput), "sneak", "Z");
            if (jumpField) env->SetBooleanField(movementInput, jumpField, false);
            if (sneakField) env->SetBooleanField(movementInput, sneakField, false);
            env->DeleteLocalRef(movementInput);
        }

        env->DeleteLocalRef(player);
    }

private:
    double oldX_ = 0, oldY_ = 0, oldZ_ = 0;
    float oldYaw_ = 0, oldPitch_ = 0;
    bool oldOnGround_ = false;
    bool wasFlying_ = false;
};
