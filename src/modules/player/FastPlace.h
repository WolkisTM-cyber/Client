#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class FastPlace : public Module {
public:
    FastPlace() : Module("FastPlace", "Fast Place", Category::Player, 0) {}

    void OnTick(JNIEnv* env) override {
        auto mc = JNIHelper::GetMinecraft(env);
        if (!mc) return;

        jfieldID rightClickDelay = env->GetFieldID(
            JNIHelper::Get().minecraft, "rightClickDelayTimer", "I");
        if (rightClickDelay) {
            env->SetIntField(mc, rightClickDelay, 0);
        }

        env->DeleteLocalRef(mc);
    }
};
