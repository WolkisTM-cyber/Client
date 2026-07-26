#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class FastLadder : public Module {
public:
    FastLadder() : Module("FastLadder", "Fast Ladder", Category::Movement, 0) {
        AddSetting(Setting::FloatSetting("Speed", "Speed", 0.3f, 0.1f, 0.8f));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        bool isCollidedHorizontally = false;
        jclass eClass = env->GetObjectClass(player);
        jfieldID collidedH = env->GetFieldID(eClass, "isCollidedHorizontally", "Z");
        if (collidedH) isCollidedHorizontally = env->GetBooleanField(player, collidedH);
        env->DeleteLocalRef(eClass);

        if (isCollidedHorizontally && (GetAsyncKeyState(VK_UP) & 0x8000 || GetAsyncKeyState('W') & 0x8000)) {
            env->SetDoubleField(player, c.motionY, GetSetting("Speed")->fVal);
        }

        env->DeleteLocalRef(player);
    }
};
