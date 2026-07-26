#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <cmath>

class Nuker : public Module {
public:
    Nuker() : Module("Nuker", "Nuker", Category::Player, 0) {
        AddSetting(Setting::IntSetting("Radius", "Radius", 4, 1, 10));
        AddSetting(Setting::ModeSetting("Mode", "Mode", {"All", "Ores", "Wood"}, 0));
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

        jmethodID getBlock = env->GetMethodID(
            env->GetObjectClass(world), "getBlock",
            "(III)Lnet/minecraft/block/Block;");

        jmethodID getBlockState = env->GetMethodID(
            env->GetObjectClass(world), "getBlockState",
            "(Lnet/minecraft/util/BlockPos;)Lnet/minecraft/block/state/IBlockState;");

        jclass blockPosClass = env->FindClass("net/minecraft/util/BlockPos");
        jmethodID blockPosCtor = env->GetMethodID(blockPosClass, "<init>", "(DDD)V");

        auto controller = JNIHelper::GetPlayerController(env);

        for (int x = (int)px - radius; x <= (int)px + radius; x++) {
            for (int y = (int)py - radius; y <= (int)py + radius; y++) {
                for (int z = (int)pz - radius; z <= (int)pz + radius; z++) {
                    double dx = x - px, dy = y - py, dz = z - pz;
                    if (sqrt(dx*dx + dy*dy + dz*dz) > radius) continue;

                    if (getBlock && getBlockState && blockPosCtor && controller) {
                        jobject pos = env->NewObject(blockPosClass, blockPosCtor, (double)x, (double)y, (double)z);
                        if (!pos) continue;

                        jobject block = env->CallObjectMethod(world, getBlock, x, y, z);
                        if (block && !env->ExceptionCheck()) {
                            jstring name = (jstring)env->CallObjectMethod(block, env->GetMethodID(
                                env->GetObjectClass(block), "getLocalizedName", "()Ljava/lang/String;"));
                            bool shouldBreak = false;

                            if (name) {
                                const char* str = env->GetStringUTFChars(name, nullptr);
                                if (str) {
                                    std::string s(str);
                                    switch (GetSetting("Mode")->modeVal) {
                                    case 0: shouldBreak = true; break;
                                    case 1: // Ores
                                        shouldBreak = s.find("ore") != std::string::npos;
                                        break;
                                    case 2: // Wood
                                        shouldBreak = s.find("log") != std::string::npos ||
                                                      s.find("wood") != std::string::npos;
                                        break;
                                    }
                                }
                                env->ReleaseStringUTFChars(name, str);
                                env->DeleteLocalRef(name);
                            }

                            if (shouldBreak) {
                                jmethodID clickBlock = env->GetMethodID(
                                    env->GetObjectClass(controller), "clickBlock",
                                    "(Lnet/minecraft/util/BlockPos;Lnet/minecraft/util/EnumFacing;)Z");
                                if (clickBlock) {
                                    env->CallBooleanMethod(controller, clickBlock, pos, nullptr);
                                    if (env->ExceptionCheck()) env->ExceptionClear();
                                }
                            }
                            env->DeleteLocalRef(block);
                        }
                        env->DeleteLocalRef(pos);
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
