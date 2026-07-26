#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <vector>
#include <deque>

struct EntityPosition {
    double x, y, z;
    int tick;
};

class BackTrack : public Module {
public:
    BackTrack() : Module("BackTrack", "Back Track", Category::Combat, 0) {
        AddSetting(Setting::IntSetting("Ticks", "Ticks", 10, 2, 40));
        AddSetting(Setting::IntSetting("Range", "Range", 4, 2, 6));
    }

    void OnTick(JNIEnv* env) override {
        auto world = JNIHelper::GetWorld(env);
        if (!world) return;

        auto& c = JNIHelper::Get();
        jobject entityList = env->CallObjectMethod(world, c.getLoadedEntityList);
        if (!entityList || env->ExceptionCheck()) {
            env->ExceptionClear();
            env->DeleteLocalRef(world);
            return;
        }

        jint size = env->CallIntMethod(entityList, c.listSize);
        if (env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(entityList); env->DeleteLocalRef(world); return; }

        currentTick_++;

        for (int i = 0; i < size && i < 100; i++) {
            jobject entity = env->CallObjectMethod(entityList, c.listGet, i);
            if (!entity || env->ExceptionCheck()) {
                if (entity) env->DeleteLocalRef(entity);
                env->ExceptionClear();
                continue;
            }

            jint id = env->CallIntMethod(entity, env->GetMethodID(
                env->GetObjectClass(entity), "getEntityId", "()I"));

            double ex = env->GetDoubleField(entity, c.posX);
            double ey = env->GetDoubleField(entity, c.posY);
            double ez = env->GetDoubleField(entity, c.posZ);

            // Store position in history
            history_[id].push_back({ex, ey, ez, currentTick_});

            auto* ticks = GetSetting("Ticks");
            int maxTicks = ticks ? ticks->iVal : 10;
            while (history_[id].size() > (size_t)maxTicks) {
                history_[id].pop_front();
            }

            env->DeleteLocalRef(entity);
        }

        env->DeleteLocalRef(entityList);
        env->DeleteLocalRef(world);
    }

    // Get closest backtrack position for an entity
    bool GetBackTrackPosition(JNIEnv* env, int entityId, double& x, double& y, double& z) {
        if (history_.count(entityId) == 0) return false;

        auto* ticks = GetSetting("Ticks");
        int maxTicks = ticks ? ticks->iVal : 10;
        auto& positions = history_[entityId];

        // Return oldest position in range
        if (!positions.empty()) {
            auto& pos = positions.front();
            if (currentTick_ - pos.tick <= maxTicks) {
                x = pos.x;
                y = pos.y;
                z = pos.z;
                return true;
            }
        }
        return false;
    }

    void OnDisable(JNIEnv* env) override {
        history_.clear();
        currentTick_ = 0;
    }

private:
    std::unordered_map<int, std::deque<EntityPosition>> history_;
    int currentTick_ = 0;
};
