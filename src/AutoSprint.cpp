#include "AutoSprint.h"
#include "GUI.h"
#include "JVMHelper.h"

AutoSprint::AutoSprint()
    : gui_(nullptr)
    , running_(false)
    , mode_(SprintMode::Forward)
    , keyDown_(false)
    , thread_(nullptr)
{
    InitializeCriticalSection(&cs_);
}

AutoSprint::~AutoSprint() {
    Stop();
    DeleteCriticalSection(&cs_);
}

bool AutoSprint::Start(GUI* gui) {
    EnterCriticalSection(&cs_);
    if (running_) {
        LeaveCriticalSection(&cs_);
        return false;
    }
    gui_ = gui;
    if (gui_) gui_->SetSprintMode(static_cast<int>(mode_));
    running_ = true;
    thread_ = CreateThread(nullptr, 0, ThreadProc, this, 0, nullptr);
    LeaveCriticalSection(&cs_);
    return thread_ != nullptr;
}

void AutoSprint::Stop() {
    EnterCriticalSection(&cs_);
    running_ = false;
    LeaveCriticalSection(&cs_);

    if (thread_) {
        WaitForSingleObject(thread_, 1000);
        CloseHandle(thread_);
        thread_ = nullptr;
    }
}

bool AutoSprint::IsRunning() const {
    return running_;
}

SprintMode AutoSprint::GetMode() const {
    EnterCriticalSection(&cs_);
    SprintMode m = mode_;
    LeaveCriticalSection(&cs_);
    return m;
}

void AutoSprint::SetMode(SprintMode mode) {
    EnterCriticalSection(&cs_);
    mode_ = mode;
    LeaveCriticalSection(&cs_);
}

void AutoSprint::CycleMode() {
    EnterCriticalSection(&cs_);
    int next = (static_cast<int>(mode_) + 1) % static_cast<int>(SprintMode::COUNT);
    mode_ = static_cast<SprintMode>(next);
    SprintMode m = mode_;
    LeaveCriticalSection(&cs_);

    if (gui_) {
        gui_->SetSprintMode(static_cast<int>(m));
        gui_->SetVisible(true);
    }
}

DWORD WINAPI AutoSprint::ThreadProc(LPVOID param) {
    AutoSprint* self = static_cast<AutoSprint*>(param);
    self->Run();
    return 0;
}

void AutoSprint::Run() {
    JavaVM* vm = nullptr;
    JNIEnv* env = nullptr;

    for (int attempts = 0; attempts < 100 && running_; attempts++) {
        if (JVMHelper::FindAndAttach(&vm, &env)) break;
        Sleep(100);
    }

    if (!vm || !env) {
        if (gui_) gui_->SetVisible(true);
        return;
    }

    jclass mcClass = env->FindClass("net/minecraft/client/Minecraft");
    if (!mcClass) { env->ExceptionClear(); JVMHelper::Detach(vm); return; }

    jmethodID getMinecraft = env->GetStaticMethodID(mcClass, "getMinecraft",
        "()Lnet/minecraft/client/Minecraft;");
    if (!getMinecraft) { env->ExceptionClear(); JVMHelper::Detach(vm); return; }

    jfieldID thePlayerField = env->GetFieldID(mcClass, "thePlayer",
        "Lnet/minecraft/entity/player/EntityPlayerSP;");
    if (!thePlayerField) {
        thePlayerField = env->GetFieldID(mcClass, "thePlayer",
            "Lnet/minecraft/entity/player/EntityPlayer;");
    }
    if (!thePlayerField) { env->ExceptionClear(); JVMHelper::Detach(vm); return; }

    jclass entityPlayerClass = env->FindClass("net/minecraft/entity/player/EntityPlayer");
    if (!entityPlayerClass) { env->ExceptionClear(); JVMHelper::Detach(vm); return; }

    jmethodID setSprinting = env->GetMethodID(entityPlayerClass, "setSprinting", "(Z)V");
    if (!setSprinting) { env->ExceptionClear(); JVMHelper::Detach(vm); return; }

    jmethodID isSprinting = env->GetMethodID(entityPlayerClass, "isSprinting", "()Z");
    if (!isSprinting) { env->ExceptionClear(); JVMHelper::Detach(vm); return; }

    jfieldID movementInputField = env->GetFieldID(entityPlayerClass, "movementInput",
        "Lnet/minecraft/util/MovementInput;");
    if (!movementInputField) { env->ExceptionClear(); JVMHelper::Detach(vm); return; }

    jclass movementInputClass = env->FindClass("net/minecraft/util/MovementInput");
    if (!movementInputClass) { env->ExceptionClear(); JVMHelper::Detach(vm); return; }

    jfieldID moveForwardField = env->GetFieldID(movementInputClass, "moveForward", "F");
    if (!moveForwardField) { env->ExceptionClear(); JVMHelper::Detach(vm); return; }

    jobject mc = env->CallStaticObjectMethod(mcClass, getMinecraft);
    if (env->ExceptionCheck()) { env->ExceptionClear(); JVMHelper::Detach(vm); return; }

    while (running_) {
        if (GetAsyncKeyState(CYCLE_KEY) & 0x8000) {
            if (!keyDown_) {
                keyDown_ = true;
                CycleMode();
            }
        } else {
            keyDown_ = false;
        }

        SprintMode currentMode;
        EnterCriticalSection(&cs_);
        currentMode = mode_;
        LeaveCriticalSection(&cs_);

        jobject player = env->GetObjectField(mc, thePlayerField);
        if (!player) {
            env->ExceptionClear();
            Sleep(30);
            continue;
        }

        jboolean alreadySprinting = env->CallBooleanMethod(player, isSprinting);

        switch (currentMode) {
        case SprintMode::Off:
            break;

        case SprintMode::Forward: {
            if (!alreadySprinting) {
                jobject movement = env->GetObjectField(player, movementInputField);
                if (movement) {
                    jfloat forward = env->GetFloatField(movement, moveForwardField);
                    if (forward > 0.0f) {
                        env->CallVoidMethod(player, setSprinting, JNI_TRUE);
                    }
                    env->DeleteLocalRef(movement);
                }
            }
            break;
        }

        case SprintMode::Always: {
            if (!alreadySprinting) {
                env->CallVoidMethod(player, setSprinting, JNI_TRUE);
            }
            break;
        }
        }

        env->DeleteLocalRef(player);
        Sleep(30);
    }

    env->DeleteLocalRef(mc);
    JVMHelper::Detach(vm);
}
