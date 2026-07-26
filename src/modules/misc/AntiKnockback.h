#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class AntiKnockback : public Module {
public:
    AntiKnockback() : Module("AntiKnockback", "Anti Knockback", Category::Combat, 0) {
        AddSetting(Setting::FloatSetting("Horizontal", "Horizontal", 0.0f, 0.0f, 1.0f));
        AddSetting(Setting::FloatSetting("Vertical", "Vertical", 0.0f, 0.0f, 1.0f));
    }

    void OnTick(JNIEnv* env) override {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto& c = JNIHelper::Get();

        int hurtTime = env->GetIntField(player, c.hurtTime);
        if (hurtTime > 0) {
            auto* h = GetSetting("Horizontal");
            auto* v = GetSetting("Vertical");
            double hVal = h ? h->fVal : 0.0;
            double vVal = v ? v->fVal : 0.0;

            double mx = env->GetDoubleField(player, c.motionX);
            double my = env->GetDoubleField(player, c.motionY);
            double mz = env->GetDoubleField(player, c.motionZ);

            env->SetDoubleField(player, c.motionX, mx * hVal);
            env->SetDoubleField(player, c.motionY, my * vVal);
            env->SetDoubleField(player, c.motionZ, mz * hVal);
        }

        env->DeleteLocalRef(player);
    }
};
