#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class FastPlace : public Module {
public:
    FastPlace() : Module("FastPlace", "Fast Place", Category::Player, 0) {
        AddSetting(Setting::IntSetting("Delay", "Delay Ticks", 1, 0, 4));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto mc = JNIHelper::GetMinecraft(env);
        if (!mc) return;

        jfieldID rightClickDelay = env->GetFieldID(
            JNIHelper::Get().minecraft, "rightClickDelayTimer", "I");
        if (rightClickDelay) {
            int delay = GetSetting("Delay")->iVal;
            int cur = env->GetIntField(mc, rightClickDelay);
            if (cur > delay) {
                env->SetIntField(mc, rightClickDelay, delay);
            }
        }

        env->DeleteLocalRef(mc);
    }
};

