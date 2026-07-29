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

    static Setting NumberSetting(const std::string& name, const std::string& display, float val, float min, float max, float step = 0.1f) {
        return FloatSetting(name, display, val, min, max);
    }

    static Setting ModeSetting(const std::string& name, const std::string& display, const std::vector<std::string>& modes, int def) {
        Setting s;
        s.type = Mode; s.name = name; s.display = display;
        s.modeOptions = modes; s.modeVal = s.modeDef = def;
        return s;
    }

    bool IsBool() const { return type == Bool; }
    bool IsFloat() const { return type == Float; }
    bool IsInt() const { return type == Int; }
    bool IsNumber() const { return type == Float || type == Int; }
    bool IsMode() const { return type == Mode; }

    bool GetBool() const { return bVal; }
    void SetBool(bool v) { bVal = v; }

    const std::string& GetName() const { return display.empty() ? name : display; }

    float GetNumber() const { return type == Float ? fVal : (float)iVal; }
    void SetNumber(float v) { if (type == Float) fVal = v; else iVal = (int)v; }

    float GetMin() const { return type == Float ? fMin : (float)iMin; }
    float GetMax() const { return type == Float ? fMax : (float)iMax; }
};

