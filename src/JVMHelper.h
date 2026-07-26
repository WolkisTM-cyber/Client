#pragma once
#include <windows.h>
#include <jni.h>

namespace JVMHelper {

    bool FindAndAttach(JavaVM** vm, JNIEnv** env);
    void Detach(JavaVM* vm);
    bool IsAttached(JavaVM* vm);

}
