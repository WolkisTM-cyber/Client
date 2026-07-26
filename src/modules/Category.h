#pragma once

enum class Category : int {
    Combat = 0,
    Movement,
    Visual,
    Player,
    Misc,
    COUNT
};

inline const char* CategoryName(Category c) {
    switch (c) {
    case Category::Combat: return "Combat";
    case Category::Movement: return "Movement";
    case Category::Visual: return "Visual";
    case Category::Player: return "Player";
    case Category::Misc: return "Misc";
    default: return "Unknown";
    }
}
