#pragma once
#include "../Module.h"
#include "../JNIHelper.h"

class Fucker : public Module {
public:
    Fucker() : Module("Fucker", "Fucker", Category::World, 0) {
        AddSetting(Setting::ModeSetting("Target", "Target", {"Bed", "Cake", "Egg", "Chest"}, 0));
        AddSetting(Setting::IntSetting("Radius", "Radius", 5, 2, 10));
    }

    void OnTick(JNIEnv* env) override {
        if (!IsEnabled()) return;
        auto player = JNIHelper::GetPlayer(env);
        if (!player) return;
        auto world = JNIHelper::GetWorld(env);
        if (!world) { env->DeleteLocalRef(player); return; }

        auto& c = JNIHelper::Get();
        double px = env->GetDoubleField(player, c.posX);
        double py = env->GetDoubleField(player, c.posY);
        double pz = env->GetDoubleField(player, c.posZ);
        int radius = GetSetting("Radius")->iVal;

        const char* targetName = "";
        switch (GetSetting("Target")->modeVal) {
        case 0: targetName = "bed"; break;
        case 1: targetName = "cake"; break;
        case 2: targetName = "dragon_egg"; break;
        case 3: targetName = "chest"; break;
        }

        jmethodID getBlock = env->GetMethodID(
            env->GetObjectClass(world), "getBlock",
            "(III)Lnet/minecraft/block/Block;");

        auto controller = JNIHelper::GetPlayerController(env);
        jclass blockPosClass = env->FindClass("net/minecraft/util/BlockPos");
        jmethodID blockPosCtor = env->GetMethodID(blockPosClass, "<init>", "(DDD)V");

        for (int x = (int)px - radius; x <= (int)px + radius; x++) {
            for (int y = (int)py - radius; y <= (int)py + radius; y++) {
                for (int z = (int)pz - radius; z <= (int)pz + radius; z++) {
                    jobject block = env->CallObjectMethod(world, getBlock, x, y, z);
                    if (!block || env->ExceptionCheck()) { if (block) env->DeleteLocalRef(block); env->ExceptionClear(); continue; }

                    jstring name = (jstring)env->CallObjectMethod(block, env->GetMethodID(
                        env->GetObjectClass(block), "getLocalizedName", "()Ljava/lang/String;"));
                    bool isTarget = false;
                    if (name) {
                        const char* str = env->GetStringUTFChars(name, nullptr);
                        if (str) isTarget = strstr(str, targetName) != nullptr;
                        env->ReleaseStringUTFChars(name, str);
                        env->DeleteLocalRef(name);
                    }
                    env->DeleteLocalRef(block);

                    if (isTarget && blockPosCtor && controller) {
                        jobject pos = env->NewObject(blockPosClass, blockPosCtor, (double)x, (double)y, (double)z);
                        if (pos) {
                            jmethodID clickBlock = env->GetMethodID(
                                env->GetObjectClass(controller), "clickBlock",
                                "(Lnet/minecraft/util/BlockPos;Lnet/minecraft/util/EnumFacing;)Z");
                            if (clickBlock) {
                                env->CallBooleanMethod(controller, clickBlock, pos, nullptr);
                                if (env->ExceptionCheck()) env->ExceptionClear();
                            }
                            env->DeleteLocalRef(pos);
                        }
                    }
                }
            }
        }

        if (blockPosClass) env->DeleteLocalRef(blockPosClass);
        if (controller) env->DeleteLocalRef(controller);
        env->DeleteLocalRef(world);
        env->DeleteLocalRef(player);
    }
};
