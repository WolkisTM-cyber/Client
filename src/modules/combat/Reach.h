#pragma once
#include "../Module.h"
#include "../JNIHelper.h"
#include <random>

class Reach : public Module {
public:
    Reach() : Module("Reach", "Reach", Category::Combat, 0) {
        AddSetting(Setting::FloatSetting("Range", "Range", 3.15f, 3.0f, 4.0f));
        AddSetting(Setting::BoolSetting("Randomize", "Randomize", true));
    }

    double GetReach() const {
        if (!IsEnabled()) return 3.0;
        float base = GetSetting("Range")->fVal;
        if (GetSetting("Randomize")->bVal) {
            static std::mt19937 rng(42);
            std::normal_distribution<double> dist(0.0, 0.05);
            base += (float)dist(rng);
        }
        return std::clamp((double)base, 3.0, 4.0);
    }
};

