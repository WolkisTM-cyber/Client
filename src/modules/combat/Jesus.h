#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class Jesus : public Module {
public:
    Jesus() : Module("Jesus", "Jesus", Category::Combat, 0) {
        AddSetting(Setting::ModeSetting("Mode", "Mode", {"Solid", "NCP", "Matrix"}, 0));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto& c = JNIHelper::Get();

        bool inWater = false;
        jmethodID isInWater = env->GetMethodID(env->GetObjectClass(player), "isInWater", "()Z");
        if (isInWater) {
            inWater = env->CallBooleanMethod(player, isInWater);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        if (!inWater) { env->DeleteLocalRef(player); return; }

        bool inLava = false;
        jmethodID isInLava = env->GetMethodID(env->GetObjectClass(player), "isInLava", "()Z");
        if (isInLava) {
            inLava = env->CallBooleanMethod(player, isInLava);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }

        double motionY = env->GetDoubleField(player, c.motionY);

        switch (GetSetting("Mode")->modeVal) {
        case 0: { // Solid - force on top
            env->SetDoubleField(player, c.motionY, 0.0);
            jmethodID setPosition = env->GetMethodID(env->GetObjectClass(player),
                "setPosition", "(DDD)V");
            if (setPosition) {
                double x = env->GetDoubleField(player, c.posX);
                double z = env->GetDoubleField(player, c.posZ);
                env->CallVoidMethod(player, setPosition, x, (int)env->GetDoubleField(player, c.posY) + 1.0, z);
            }
            env->SetBooleanField(player, c.onGround, true);
            break;
        }
        case 1: // NCP - bounce on water
            if (motionY < -0.05) {
                env->SetDoubleField(player, c.motionY, 0.1);
            }
            break;
        case 2: // Matrix - spoof position
            env->SetDoubleField(player, c.motionY, 0.0);
            break;
        }

        env->DeleteLocalRef(player);
    }
};
