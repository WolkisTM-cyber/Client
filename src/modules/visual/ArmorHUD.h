#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class ArmorHUD : public Module {
public:
    ArmorHUD() : Module("ArmorHUD", "Armor HUD", Category::Visual, 0) {
        AddSetting(Setting::BoolSetting("Durability", "Show Durability", true));
        AddSetting(Setting::BoolSetting("Horizontal", "Horizontal", true));
    }

    void Render(JNIEnv* env, jobject fr, jmethodID drawStr) {
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;

        auto& c = JNIHelper::Get();
        jobject inventory = env->CallObjectMethod(player,
            env->GetMethodID(c.entityPlayer, "getInventory",
                "()Lnet/minecraft/entity/player/InventoryPlayer;"));
        if (!inventory) { env->DeleteLocalRef(player); return; }

        jmethodID getStackInSlot = env->GetMethodID(
            env->GetObjectClass(inventory), "getStackInSlot", "(I)Lnet/minecraft/item/ItemStack;");

        int x = 4;
        int yBase = 40;

        auto* horiz = GetSetting("Horizontal");
        auto* dur = GetSetting("Durability");

        // Armor slots: 3=head, 2=chest, 1=legs, 0=boots
        for (int i = 3; i >= 0; i--) {
            jobject stack = env->CallObjectMethod(inventory, getStackInSlot, i);
            if (stack) {
                jobject item = env->CallObjectMethod(stack, c.getItemFromStack);
                if (item) {
                    jstring name = (jstring)env->CallObjectMethod(item, c.getUnlocalizedName);
                    if (name) {
                        const char* str = env->GetStringUTFChars(name, nullptr);
                        if (str) {
                            std::string s(str);
                            // Get slot name
                            const char* slotName = "";
                            if (s.find("helmet") != std::string::npos) slotName = "Helm";
                            else if (s.find("chestplate") != std::string::npos) slotName = "Chest";
                            else if (s.find("leggings") != std::string::npos) slotName = "Legs";
                            else if (s.find("boots") != std::string::npos) slotName = "Boots";
                            else slotName = str;

                            if (dur && dur->bVal) {
                                jmethodID getMaxDamage = env->GetMethodID(
                                    c.itemStack, "getMaxDamage", "()I");
                                jmethodID getItemDamage = env->GetMethodID(
                                    c.itemStack, "getItemDamage", "()I");
                                if (getMaxDamage && getItemDamage) {
                                    int maxDmg = env->CallIntMethod(stack, getMaxDamage);
                                    int curDmg = env->CallIntMethod(stack, getItemDamage);
                                    char buf[64];
                                    snprintf(buf, sizeof(buf), "%s: %d/%d",
                                             slotName, maxDmg - curDmg, maxDmg);
                                    jstring text = env->NewStringUTF(buf);
                                    if (text && drawStr) {
                                        env->CallIntMethod(fr, drawStr, text, x,
                                            horiz && horiz->bVal ? yBase : yBase + 12*i, 0xAAAAAA);
                                        if (env->ExceptionCheck()) env->ExceptionClear();
                                        env->DeleteLocalRef(text);
                                    }
                                }
                            } else {
                                jstring text = env->NewStringUTF(slotName);
                                if (text && drawStr) {
                                    env->CallIntMethod(fr, drawStr, text, x,
                                        horiz && horiz->bVal ? yBase : yBase + (3 - i) * 12, 0xAAAAAA);
                                    if (env->ExceptionCheck()) env->ExceptionClear();
                                    env->DeleteLocalRef(text);
                                }
                            }
                            env->ReleaseStringUTFChars(name, str);
                        }
                        env->DeleteLocalRef(name);
                    }
                    env->DeleteLocalRef(item);
                }
                env->DeleteLocalRef(stack);
            }
        }

        env->DeleteLocalRef(inventory);
        env->DeleteLocalRef(player);
    }
};
