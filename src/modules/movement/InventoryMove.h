#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class InventoryMove : public Module {
public:
    InventoryMove() : Module("InventoryMove", "Inventory Move", Category::Movement, 0) {
        AddSetting(Setting::BoolSetting("Sneak", "Allow Sneak", true));
        AddSetting(Setting::BoolSetting("Jump", "Allow Jump", true));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto mc = JNIHelper::GetMinecraft(env);
        if (!mc) { env->DeleteLocalRef(player); return; }

        auto& c = JNIHelper::Get();
        jobject screen = env->GetObjectField(mc,
            env->GetFieldID(c.minecraft, "currentScreen",
                "Lnet/minecraft/client/gui/GuiScreen;"));
        if (!screen) { env->DeleteLocalRef(mc); env->DeleteLocalRef(player); return; }

        bool onGround = env->GetBooleanField(player, c.onGround);

        if (GetAsyncKeyState('W') & 0x8000) {
            float yaw = env->GetFloatField(player, c.rotationYaw);
            double rad = yaw * 3.14159 / 180.0;
            env->SetDoubleField(player, c.motionX, -sin(rad) * 0.3);
            env->SetDoubleField(player, c.motionZ, cos(rad) * 0.3);
        }
        if (GetAsyncKeyState('S') & 0x8000) {
            float yaw = env->GetFloatField(player, c.rotationYaw);
            double rad = yaw * 3.14159 / 180.0;
            env->SetDoubleField(player, c.motionX, sin(rad) * 0.3);
            env->SetDoubleField(player, c.motionZ, -cos(rad) * 0.3);
        }
        if (GetAsyncKeyState('A') & 0x8000) {
            float yaw = env->GetFloatField(player, c.rotationYaw);
            double rad = (yaw - 90) * 3.14159 / 180.0;
            env->SetDoubleField(player, c.motionX, -sin(rad) * 0.3);
            env->SetDoubleField(player, c.motionZ, cos(rad) * 0.3);
        }
        if (GetAsyncKeyState('D') & 0x8000) {
            float yaw = env->GetFloatField(player, c.rotationYaw);
            double rad = (yaw + 90) * 3.14159 / 180.0;
            env->SetDoubleField(player, c.motionX, -sin(rad) * 0.3);
            env->SetDoubleField(player, c.motionZ, cos(rad) * 0.3);
        }
        if (GetSetting("Jump")->bVal && (GetAsyncKeyState(VK_SPACE) & 0x8000) && onGround) {
            env->SetDoubleField(player, c.motionY, 0.42);
        }

        env->DeleteLocalRef(screen);
        env->DeleteLocalRef(mc);
        env->DeleteLocalRef(player);
    }
};
