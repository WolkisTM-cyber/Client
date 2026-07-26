#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class Spider : public Module {
public:
    Spider() : Module("Spider", "Spider", Category::Movement, 0) {
        AddSetting(Setting::FloatSetting("Motion", "Motion", 0.2f, 0.1f, 0.5f));
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

        if (isCollidedHorizontally) {
            env->SetDoubleField(player, c.motionY, GetSetting("Motion")->fVal);
        }

        env->DeleteLocalRef(eClass);
        env->DeleteLocalRef(player);
    }
};
