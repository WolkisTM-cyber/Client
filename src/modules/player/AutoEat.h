#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class AutoEat : public Module {
public:
    AutoEat() : Module("AutoEat", "Auto Eat", Category::Player, 0) {
        AddSetting(Setting::FloatSetting("Hunger", "Hunger", 6.0f, 1.0f, 19.0f));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        jmethodID getFoodStats = env->GetMethodID(c.entityPlayer, "getFoodStats",
            "()Lnet/minecraft/util/FoodStats;");
        if (!getFoodStats) { env->DeleteLocalRef(player); return; }

        jobject foodStats = env->CallObjectMethod(player, getFoodStats);
        if (!foodStats || env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(player); return; }

        jmethodID getFoodLevel = env->GetMethodID(
            env->GetObjectClass(foodStats), "getFoodLevel", "()I");
        if (!getFoodLevel) { env->DeleteLocalRef(foodStats); env->DeleteLocalRef(player); return; }

        int foodLevel = env->CallIntMethod(foodStats, getFoodLevel);
        if (env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(foodStats); env->DeleteLocalRef(player); return; }

        if (foodLevel < (int)GetSetting("Hunger")->fVal) {
            // Search inventory for food
            jobject inv = env->CallObjectMethod(player,
                env->GetMethodID(c.entityPlayer, "getInventory",
                    "()Lnet/minecraft/entity/player/InventoryPlayer;"));
            if (!inv) { env->DeleteLocalRef(foodStats); env->DeleteLocalRef(player); return; }

            jmethodID getStack = env->GetMethodID(
                env->GetObjectClass(inv), "getStackInSlot", "(I)Lnet/minecraft/item/ItemStack;");

            for (int slot = 0; slot < 9; slot++) {
                jobject stack = env->CallObjectMethod(inv, getStack, slot);
                if (!stack) continue;

                jobject item = env->CallObjectMethod(stack, c.getItemFromStack);
                if (!item) { env->DeleteLocalRef(stack); continue; }

                jstring name = (jstring)env->CallObjectMethod(item, c.getUnlocalizedName);
                bool isFood = false;
                if (name) {
                    const char* str = env->GetStringUTFChars(name, nullptr);
                    if (str) {
                        std::string s(str);
                        isFood = s.find("food") != std::string::npos ||
                                 s.find("apple") != std::string::npos ||
                                 s.find("bread") != std::string::npos ||
                                 s.find("cook") != std::string::npos ||
                                 s.find("fish") != std::string::npos ||
                                 s.find("pork") != std::string::npos ||
                                 s.find("beef") != std::string::npos ||
                                 s.find("chicken") != std::string::npos ||
                                 s.find("rabbit") != std::string::npos ||
                                 s.find("mutton") != std::string::npos ||
                                 s.find("potato") != std::string::npos ||
                                 s.find("carrot") != std::string::npos;
                    }
                    env->ReleaseStringUTFChars(name, str);
                    env->DeleteLocalRef(name);
                }
                env->DeleteLocalRef(item);

                if (isFood) {
                    env->SetIntField(inv, env->GetFieldID(
                        env->GetObjectClass(inv), "currentItem", "I"), slot);
                    if (env->ExceptionCheck()) env->ExceptionClear();

                    // Right click (use item)
                    jobject controller = JNIHelper::GetPlayerController(env);
                    if (controller) {
                        jmethodID rightClick = env->GetMethodID(
                            env->GetObjectClass(controller), "sendUseItem",
                            "(Lnet/minecraft/entity/player/EntityPlayer;Lnet/minecraft/world/World;Lnet/minecraft/item/ItemStack;)V");
                        if (rightClick) {
                            env->CallVoidMethod(controller, rightClick, player, nullptr, stack);
                            if (env->ExceptionCheck()) env->ExceptionClear();
                        }
                        env->DeleteLocalRef(controller);
                    }
                    env->DeleteLocalRef(stack);
                    break;
                }
                env->DeleteLocalRef(stack);
            }
            env->DeleteLocalRef(inv);
        }

        env->DeleteLocalRef(foodStats);
        env->DeleteLocalRef(player);
    }
};
