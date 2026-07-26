#pragma once
#include <jni.h>
#include <string>
#include <unordered_map>
#include <vector>

class JNIHelper {
public:
    static bool Initialize(JNIEnv* env);
    static void Cleanup(JNIEnv* env);

    static jobject GetMinecraft(JNIEnv* env);
    static jobject GetPlayer(JNIEnv* env);
    static jobject GetWorld(JNIEnv* env);
    static jobject GetPlayerController(JNIEnv* env);
    static jobject GetGameSettings(JNIEnv* env);
    static jobject GetPlayerCapabilities(JNIEnv* env);

    static void SetSpeed(JNIEnv* env, jobject player, double speed);
    static double GetMotionX(JNIEnv* env, jobject player);
    static double GetMotionY(JNIEnv* env, jobject player);
    static double GetMotionZ(JNIEnv* env, jobject player);
    static void SetMotion(JNIEnv* env, jobject player, double x, double y, double z);

    struct Cache {
        // Classes (global refs)
        jclass minecraft = nullptr;
        jclass entityPlayerSP = nullptr;
        jclass entityPlayer = nullptr;
        jclass entityLivingBase = nullptr;
        jclass entity = nullptr;
        jclass worldClient = nullptr;
        jclass playerControllerMP = nullptr;
        jclass gameSettings = nullptr;
        jclass playerCapabilities = nullptr;
        jclass blockPos = nullptr;
        jclass movementInput = nullptr;
        jclass itemStack = nullptr;
        jclass item = nullptr;
        jclass list = nullptr;

        // Methods
        jmethodID getMinecraft = nullptr;

        // Fields (MCP 1.8.x)
        jfieldID thePlayer = nullptr;
        jfieldID theWorld = nullptr;
        jfieldID playerController = nullptr;
        jfieldID gameSettingsF = nullptr;
        jfieldID timer = nullptr;
        jfieldID leftClickCounter = nullptr;

        // Entity fields
        jfieldID motionX = nullptr;
        jfieldID motionY = nullptr;
        jfieldID motionZ = nullptr;
        jfieldID rotationYaw = nullptr;
        jfieldID rotationPitch = nullptr;
        jfieldID posX = nullptr;
        jfieldID posY = nullptr;
        jfieldID posZ = nullptr;
        jfieldID onGround = nullptr;
        jfieldID stepHeight = nullptr;
        jfieldID isInWeb = nullptr;
        jfieldID hurtTime = nullptr;
        jfieldID fallDistance = nullptr;
        jfieldID hurtResistantTime = nullptr;
        jfieldID speedInAir = nullptr;
        jfieldID jumpMovementFactor = nullptr;

        // EntityPlayer fields
        jfieldID capabilities = nullptr;
        jfieldID movementInputF = nullptr;

        // PlayerCapabilities fields
        jfieldID isFlyingF = nullptr;

        // GameSettings fields
        jfieldID gammaSetting = nullptr;

        // MovementInput fields
        jfieldID moveForward = nullptr;
        jfieldID moveStrafe = nullptr;

        // Methods
        jmethodID setSprinting = nullptr;
        jmethodID isSprinting = nullptr;
        jmethodID setSneaking = nullptr;
        jmethodID isSneaking = nullptr;
        jmethodID getHeldItem = nullptr;
        jmethodID getItemFromStack = nullptr;
        jmethodID getUnlocalizedName = nullptr;
        jmethodID isEntityAlive = nullptr;
        jmethodID attackEntity = nullptr;
        jmethodID clickBlock = nullptr;
        jmethodID sendChatMessage = nullptr;
        jmethodID listSize = nullptr;
        jmethodID listGet = nullptr;
        jmethodID getLoadedEntityList = nullptr;
        jfieldID getPlayerEntitiesF = nullptr;
        jmethodID onUpdate = nullptr;

        // Timer fields
        jfieldID timerSpeed = nullptr;
    };

    static Cache& Get() { static Cache c; return c; }

private:
    static void SafeDelete(JNIEnv* env, jclass& clazz);
    static jclass SafeFind(JNIEnv* env, const char* name);
    static jfieldID SafeField(JNIEnv* env, jclass clazz, const char* name, const char* sig);
    static jmethodID SafeMethod(JNIEnv* env, jclass clazz, const char* name, const char* sig, bool isStatic = false);
};
