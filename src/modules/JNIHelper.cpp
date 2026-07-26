#include "JNIHelper.h"
#include <cmath>

#define FIND_CLASS(name) SafeFind(env, name)
#define FIND_METHOD(clazz, name, sig) SafeMethod(env, clazz, name, sig)
#define FIND_STATIC_METHOD(clazz, name, sig) SafeMethod(env, clazz, name, sig, true)
#define FIND_FIELD(clazz, name, sig) SafeField(env, clazz, name, sig)

jclass JNIHelper::SafeFind(JNIEnv* env, const char* name) {
    jclass local = env->FindClass(name);
    if (!local) {
        env->ExceptionClear();
        return nullptr;
    }
    jclass global = (jclass)env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    return global;
}

jfieldID JNIHelper::SafeField(JNIEnv* env, jclass clazz, const char* name, const char* sig) {
    if (!clazz) return nullptr;
    return env->GetFieldID(clazz, name, sig);
}

jmethodID JNIHelper::SafeMethod(JNIEnv* env, jclass clazz, const char* name, const char* sig, bool isStatic) {
    if (!clazz) return nullptr;
    return isStatic ? env->GetStaticMethodID(clazz, name, sig) : env->GetMethodID(clazz, name, sig);
}

void JNIHelper::SafeDelete(JNIEnv* env, jclass& clazz) {
    if (clazz) { env->DeleteGlobalRef(clazz); clazz = nullptr; }
}

bool JNIHelper::Initialize(JNIEnv* env) {
    auto& c = Get();

    c.minecraft = FIND_CLASS("net/minecraft/client/Minecraft");
    c.entityPlayerSP = FIND_CLASS("net/minecraft/entity/player/EntityPlayerSP");
    c.entityPlayer = FIND_CLASS("net/minecraft/entity/player/EntityPlayer");
    c.entityLivingBase = FIND_CLASS("net/minecraft/entity/EntityLivingBase");
    c.entity = FIND_CLASS("net/minecraft/entity/Entity");
    c.worldClient = FIND_CLASS("net/minecraft/client/multiplayer/WorldClient");
    c.playerControllerMP = FIND_CLASS("net/minecraft/client/multiplayer/PlayerControllerMP");
    c.gameSettings = FIND_CLASS("net/minecraft/client/settings/GameSettings");
    c.playerCapabilities = FIND_CLASS("net/minecraft/entity/player/PlayerCapabilities");
    c.blockPos = FIND_CLASS("net/minecraft/util/BlockPos");
    c.movementInput = FIND_CLASS("net/minecraft/util/MovementInput");
    c.itemStack = FIND_CLASS("net/minecraft/item/ItemStack");
    c.item = FIND_CLASS("net/minecraft/item/Item");
    c.list = FIND_CLASS("java/util/List");

    if (!c.minecraft) return false;

    // Minecraft methods/fields
    c.getMinecraft = FIND_STATIC_METHOD(c.minecraft, "getMinecraft", "()Lnet/minecraft/client/Minecraft;");
    c.thePlayer = FIND_FIELD(c.minecraft, "thePlayer", "Lnet/minecraft/entity/player/EntityPlayer;");
    c.theWorld = FIND_FIELD(c.minecraft, "theWorld", "Lnet/minecraft/client/multiplayer/WorldClient;");
    c.playerController = FIND_FIELD(c.minecraft, "playerController", "Lnet/minecraft/client/multiplayer/PlayerControllerMP;");
    c.gameSettingsF = FIND_FIELD(c.minecraft, "gameSettings", "Lnet/minecraft/client/settings/GameSettings;");
    c.timer = FIND_FIELD(c.minecraft, "timer", "Lnet/minecraft/util/Timer;");
    c.leftClickCounter = FIND_FIELD(c.minecraft, "leftClickCounter", "I");

    // Entity fields
    c.motionX = FIND_FIELD(c.entity, "motionX", "D");
    c.motionY = FIND_FIELD(c.entity, "motionY", "D");
    c.motionZ = FIND_FIELD(c.entity, "motionZ", "D");
    c.rotationYaw = FIND_FIELD(c.entity, "rotationYaw", "F");
    c.rotationPitch = FIND_FIELD(c.entity, "rotationPitch", "F");
    c.posX = FIND_FIELD(c.entity, "posX", "D");
    c.posY = FIND_FIELD(c.entity, "posY", "D");
    c.posZ = FIND_FIELD(c.entity, "posZ", "D");
    c.onGround = FIND_FIELD(c.entity, "onGround", "Z");
    c.stepHeight = FIND_FIELD(c.entity, "stepHeight", "F");
    c.isInWeb = FIND_FIELD(c.entity, "isInWeb", "Z");

    // EntityLivingBase fields/methods
    c.hurtTime = FIND_FIELD(c.entityLivingBase, "hurtTime", "I");
    c.fallDistance = FIND_FIELD(c.entityLivingBase, "fallDistance", "F");
    c.hurtResistantTime = FIND_FIELD(c.entityLivingBase, "hurtResistantTime", "I");
    c.speedInAir = FIND_FIELD(c.entityLivingBase, "speedInAir", "F");
    c.jumpMovementFactor = FIND_FIELD(c.entityLivingBase, "jumpMovementFactor", "F");
    c.setSprinting = FIND_METHOD(c.entityLivingBase, "setSprinting", "(Z)V");
    c.isSprinting = FIND_METHOD(c.entityLivingBase, "isSprinting", "()Z");
    c.setSneaking = FIND_METHOD(c.entity, "setSneaking", "(Z)V");
    c.isSneaking = FIND_METHOD(c.entity, "isSneaking", "()Z");
    c.getHeldItem = FIND_METHOD(c.entityLivingBase, "getHeldItem", "()Lnet/minecraft/item/ItemStack;");
    c.isEntityAlive = FIND_METHOD(c.entityLivingBase, "isEntityAlive", "()Z");
    c.onUpdate = FIND_METHOD(c.entity, "onUpdate", "()V");

    // EntityPlayer fields
    c.capabilities = FIND_FIELD(c.entityPlayer, "capabilities", "Lnet/minecraft/entity/player/PlayerCapabilities;");
    c.movementInputF = FIND_FIELD(c.entityPlayer, "movementInput", "Lnet/minecraft/util/MovementInput;");

    // PlayerCapabilities fields
    c.isFlyingF = FIND_FIELD(c.playerCapabilities, "isFlying", "Z");

    // GameSettings fields
    c.gammaSetting = FIND_FIELD(c.gameSettings, "gammaSetting", "F");

    // MovementInput fields
    c.moveForward = FIND_FIELD(c.movementInput, "moveForward", "F");
    c.moveStrafe = FIND_FIELD(c.movementInput, "moveStrafe", "F");

    // PlayerControllerMP methods
    c.attackEntity = FIND_METHOD(c.playerControllerMP, "attackEntity", "(Lnet/minecraft/entity/player/EntityPlayer;Lnet/minecraft/entity/Entity;)V");
    c.clickBlock = FIND_METHOD(c.playerControllerMP, "clickBlock", "(Lnet/minecraft/util/BlockPos;Lnet/minecraft/util/EnumFacing;)Z");

    // Item methods
    c.getItemFromStack = FIND_METHOD(c.itemStack, "getItem", "()Lnet/minecraft/item/Item;");
    c.getUnlocalizedName = FIND_METHOD(c.item, "getUnlocalizedName", "()Ljava/lang/String;");

    // EntityPlayer methods
    c.sendChatMessage = FIND_METHOD(c.entityPlayer, "sendChatMessage", "(Ljava/lang/String;)V");

    // List methods
    c.listSize = FIND_METHOD(c.list, "size", "()I");
    c.listGet = FIND_METHOD(c.list, "get", "(I)Ljava/lang/Object;");

    // World methods
    c.getLoadedEntityList = FIND_METHOD(c.worldClient, "getLoadedEntityList", "()Ljava/util/List;");
    c.getPlayerEntitiesF = FIND_FIELD(c.worldClient, "playerEntities", "Ljava/util/List;");

    // Timer fields
    {
        jclass timerClass = FIND_CLASS("net/minecraft/util/Timer");
        c.timerSpeed = FIND_FIELD(timerClass, "timerSpeed", "F");
        if (timerClass) env->DeleteGlobalRef(timerClass);
    }

    return true;
}

void JNIHelper::Cleanup(JNIEnv* env) {
    auto& c = Get();
    #define SAFE_DEL(x) SafeDelete(env, x)
    SAFE_DEL(c.minecraft);
    SAFE_DEL(c.entityPlayerSP);
    SAFE_DEL(c.entityPlayer);
    SAFE_DEL(c.entityLivingBase);
    SAFE_DEL(c.entity);
    SAFE_DEL(c.worldClient);
    SAFE_DEL(c.playerControllerMP);
    SAFE_DEL(c.gameSettings);
    SAFE_DEL(c.playerCapabilities);
    SAFE_DEL(c.blockPos);
    SAFE_DEL(c.movementInput);
    SAFE_DEL(c.itemStack);
    SAFE_DEL(c.item);
    SAFE_DEL(c.list);
    #undef SAFE_DEL
}

jobject JNIHelper::GetMinecraft(JNIEnv* env) {
    return env->CallStaticObjectMethod(Get().minecraft, Get().getMinecraft);
}

jobject JNIHelper::GetPlayer(JNIEnv* env) {
    jobject mc = GetMinecraft(env);
    if (!mc) return nullptr;
    jobject player = env->GetObjectField(mc, Get().thePlayer);
    env->DeleteLocalRef(mc);
    return player;
}

jobject JNIHelper::GetWorld(JNIEnv* env) {
    jobject mc = GetMinecraft(env);
    if (!mc) return nullptr;
    jobject world = env->GetObjectField(mc, Get().theWorld);
    env->DeleteLocalRef(mc);
    return world;
}

jobject JNIHelper::GetPlayerController(JNIEnv* env) {
    jobject mc = GetMinecraft(env);
    if (!mc) return nullptr;
    jobject pc = env->GetObjectField(mc, Get().playerController);
    env->DeleteLocalRef(mc);
    return pc;
}

jobject JNIHelper::GetGameSettings(JNIEnv* env) {
    jobject mc = GetMinecraft(env);
    if (!mc) return nullptr;
    jobject gs = env->GetObjectField(mc, Get().gameSettingsF);
    env->DeleteLocalRef(mc);
    return gs;
}

jobject JNIHelper::GetPlayerCapabilities(JNIEnv* env) {
    jobject player = GetPlayer(env);
    if (!player) return nullptr;
    jobject caps = env->GetObjectField(player, Get().capabilities);
    env->DeleteLocalRef(player);
    return caps;
}

void JNIHelper::SetSpeed(JNIEnv* env, jobject player, double speed) {
    float yaw = env->GetFloatField(player, Get().rotationYaw);
    double rad = yaw * 3.141592653589793 / 180.0;
    env->SetDoubleField(player, Get().motionX, -std::sin(rad) * speed);
    env->SetDoubleField(player, Get().motionZ, std::cos(rad) * speed);
}

double JNIHelper::GetMotionX(JNIEnv* env, jobject player) { return env->GetDoubleField(player, Get().motionX); }
double JNIHelper::GetMotionY(JNIEnv* env, jobject player) { return env->GetDoubleField(player, Get().motionY); }
double JNIHelper::GetMotionZ(JNIEnv* env, jobject player) { return env->GetDoubleField(player, Get().motionZ); }

void JNIHelper::SetMotion(JNIEnv* env, jobject player, double x, double y, double z) {
    env->SetDoubleField(player, Get().motionX, x);
    env->SetDoubleField(player, Get().motionY, y);
    env->SetDoubleField(player, Get().motionZ, z);
}
