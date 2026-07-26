#pragma once
#include <string>
#include <vector>

struct Setting {
    std::string name;
    std::string display;

    enum Type : int { Bool = 0, Int, Float, Mode } type;

    bool bVal, bDef;
    int iVal, iDef, iMin, iMax;
    float fVal, fDef, fMin, fMax;
    std::vector<std::string> modeOptions;
    int modeVal, modeDef;

    Setting() = default;

    static Setting BoolSetting(const std::string& name, const std::string& display, bool val) {
        Setting s;
        s.type = Bool; s.name = name; s.display = display;
        s.bVal = s.bDef = val;
        return s;
    }

    static Setting IntSetting(const std::string& name, const std::string& display, int val, int min, int max) {
        Setting s;
        s.type = Int; s.name = name; s.display = display;
        s.iVal = s.iDef = val; s.iMin = min; s.iMax = max;
        return s;
    }

    static Setting FloatSetting(const std::string& name, const std::string& display, float val, float min, float max) {
        Setting s;
        s.type = Float; s.name = name; s.display = display;
        s.fVal = s.fDef = val; s.fMin = min; s.fMax = max;
        return s;
    }

    static Setting ModeSetting(const std::string& name, const std::string& display, const std::vector<std::string>& modes, int def) {
        Setting s;
        s.type = Mode; s.name = name; s.display = display;
        s.modeOptions = modes; s.modeVal = s.modeDef = def;
        return s;
    }
};
