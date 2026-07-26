#pragma once
#include <windows.h>
#include <vector>

class Detour {
public:
    Detour() : target_(nullptr), hook_(nullptr), applied_(false) {}

    bool Install(void* target, void* hook) {
        if (!target || !hook) return false;
        target_ = (uintptr_t)target;
        hook_ = (uintptr_t)hook;

        DWORD old;
        VirtualProtect((void*)target_, 14, PAGE_EXECUTE_READWRITE, &old);

        // Save original bytes
        memcpy(backup_, (void*)target_, 14);
        applied_ = true;

        // Write JMP [RIP+0] to hook
        // FF 25 00 00 00 00 <8 byte addr>
        BYTE jmp[] = { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00 };
        memcpy((void*)target_, jmp, 6);
        *(uintptr_t*)(target_ + 6) = hook_;

        VirtualProtect((void*)target_, 14, old, &old);

        return true;
    }

    void Uninstall() {
        if (!applied_) return;
        DWORD old;
        VirtualProtect((void*)target_, 14, PAGE_EXECUTE_READWRITE, &old);
        memcpy((void*)target_, backup_, 14);
        VirtualProtect((void*)target_, 14, old, &old);
        applied_ = false;
    }

    template<typename T>
    T GetOriginal() { return (T)backup_trampoline_; }

private:
    uintptr_t target_;
    uintptr_t hook_;
    BYTE backup_[14];
    BYTE backup_trampoline_[14 + 14];
    bool applied_;
};
