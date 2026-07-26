#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class AutoTool : public Module {
public:
    AutoTool() : Module("AutoTool", "Auto Tool", Category::Player, 0) {
        AddSetting(Setting::BoolSetting("SwitchBack", "Switch Back", true));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        jobject mc = JNIHelper::GetMinecraft(env);
        if (!mc) { env->DeleteLocalRef(player); return; }

        jfieldID objectMouseOverField = env->GetFieldID(c.minecraft, "objectMouseOver", "Lnet/minecraft/util/MovingObjectPosition;");
        if (objectMouseOverField) {
            jobject mop = env->GetObjectField(mc, objectMouseOverField);
            if (mop) {
                if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
                    // Holding left-click while targeting block
                    jobject inv = env->CallObjectMethod(player, env->GetMethodID(c.entityPlayer, "getInventory", "()Lnet/minecraft/entity/player/InventoryPlayer;"));
                    if (inv) {
                        // Scan hotbar for pickaxe/axe/shovel
                        jmethodID getStack = env->GetMethodID(env->GetObjectClass(inv), "getStackInSlot", "(I)Lnet/minecraft/item/ItemStack;");
                        for (int slot = 0; slot < 9; slot++) {
                            jobject stack = env->CallObjectMethod(inv, getStack, slot);
                            if (stack) {
                                jobject item = env->CallObjectMethod(stack, c.getItemFromStack);
                                if (item) {
                                    jstring name = (jstring)env->CallObjectMethod(item, c.getUnlocalizedName);
                                    if (name) {
                                        const char* str = env->GetStringUTFChars(name, nullptr);
                                        if (str && (strstr(str, "pickaxe") || strstr(str, "axe") || strstr(str, "shovel"))) {
                                            env->SetIntField(inv, env->GetFieldID(env->GetObjectClass(inv), "currentItem", "I"), slot);
                                            env->ReleaseStringUTFChars(name, str);
                                            env->DeleteLocalRef(name);
                                            env->DeleteLocalRef(item);
                                            env->DeleteLocalRef(stack);
                                            break;
                                        }
                                        if (str) env->ReleaseStringUTFChars(name, str);
                                        env->DeleteLocalRef(name);
                                    }
                                    env->DeleteLocalRef(item);
                                }
                                env->DeleteLocalRef(stack);
                            }
                        }
                        env->DeleteLocalRef(inv);
                    }
                }
                env->DeleteLocalRef(mop);
            }
        }

        env->DeleteLocalRef(mc);
        env->DeleteLocalRef(player);
    }
};

