#include <jni.h>
#include "wincore/wincore.h"
#include <android/log.h>
#include <memory>
#include <cstring>

#define LOG_TAG "WinCore-JNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Global WinCore engine instance
static std::unique_ptr<WinCoreEngine> gWinCoreEngine;

/**
 * JNI_OnLoad - Called when native library is first loaded
 * 
 * Initializes the WinCore engine singleton.
 * Version: JNI version 1.6
 */
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGI("JNI_OnLoad: Initializing WinCore native library");
    
    if (!gWinCoreEngine) {
        gWinCoreEngine = std::make_unique<WinCoreEngine>();
        LOGI("WinCoreEngine singleton created");
    }
    
    return JNI_VERSION_1_6;
}

/**
 * JNI_OnUnload - Called when native library is unloaded
 * 
 * Cleans up the WinCore engine singleton.
 */
void JNI_OnUnload(JavaVM* vm, void* reserved) {
    LOGI("JNI_OnUnload: Cleaning up WinCore native library");
    
    if (gWinCoreEngine) {
        gWinCoreEngine->unload();
        gWinCoreEngine.reset();
        LOGI("WinCoreEngine singleton destroyed");
    }
}

/**
 * Java_com_windroid_MainActivity_runExe
 * 
 * JNI function called from Kotlin MainActivity
 * Loads and executes a Windows PE executable on Android
 * 
 * @param env - JNI environment pointer
 * @param obj - Java object reference (MainActivity)
 * @param exePath - Java String containing path to .exe file
 * @return true (jboolean) if execution successful, false on error
 * 
 * Process:
 * 1. Converts Java String to C++ string
 * 2. Loads executable via WinCoreEngine::loadExe()
 * 3. Executes binary via WinCoreEngine::run()
 * 4. Returns execution status to Java/Kotlin
 */
JNIEXPORT jboolean JNICALL Java_com_windroid_MainActivity_runExe(
    JNIEnv* env, jobject obj, jstring exePath) {
    
    if (!env || !exePath) {
        LOGE("runExe: Invalid JNI parameters");
        return JNI_FALSE;
    }

    if (!gWinCoreEngine) {
        LOGE("runExe: WinCoreEngine not initialized");
        return JNI_FALSE;
    }

    // Convert Java String to C++ std::string
    const char* nativeExePath = env->GetStringUTFChars(exePath, nullptr);
    if (!nativeExePath) {
        LOGE("runExe: Failed to get native string from Java String");
        return JNI_FALSE;
    }

    std::string exePathStr(nativeExePath);
    env->ReleaseStringUTFChars(exePath, nativeExePath);

    LOGI("runExe: Starting execution of '%s'", exePathStr.c_str());

    try {
        // Step 1: Load the Windows executable
        if (!gWinCoreEngine->loadExe(exePathStr)) {
            LOGE("runExe: Failed to load executable");
            LOGE("Error: %s", gWinCoreEngine->getLastError().c_str());
            return JNI_FALSE;
        }

        LOGI("runExe: Executable loaded successfully");

        // Step 2: Execute the binary
        if (!gWinCoreEngine->run()) {
            LOGE("runExe: Execution failed");
            LOGE("Error: %s", gWinCoreEngine->getLastError().c_str());
            LOGE("Exit Code: %d", gWinCoreEngine->getExitCode());
            return JNI_FALSE;
        }

        LOGI("runExe: Execution completed successfully");
        LOGI("runExe: Exit Code: %d", gWinCoreEngine->getExitCode());

        // Step 3: Unload the binary
        gWinCoreEngine->unload();

        return JNI_TRUE;

    } catch (const std::exception& e) {
        LOGE("runExe: Exception caught: %s", e.what());
        return JNI_FALSE;
    } catch (...) {
        LOGE("runExe: Unknown exception caught");
        return JNI_FALSE;
    }
}

/**
 * Java_com_windroid_MainActivity_getVersion
 * 
 * JNI function to get WinCore version information
 * Called from native code to verify library is working
 * 
 * @param env - JNI environment pointer
 * @param obj - Java object reference
 * @return Java String containing version information
 */
JNIEXPORT jstring JNICALL Java_com_windroid_MainActivity_getVersion(
    JNIEnv* env, jobject obj) {
    
    LOGI("getVersion: Called");
    
    const char* version = "WinDroid V4 - WinCore NDK26 C++17 ARM64";
    jstring result = env->NewStringUTF(version);
    
    LOGI("getVersion: Returning '%s'", version);
    return result;
}

/**
 * Java_com_windroid_MainActivity_getEngineStatus
 * 
 * JNI function to query current WinCore engine status
 * 
 * @param env - JNI environment pointer
 * @param obj - Java object reference
 * @return Java String describing current engine state
 */
JNIEXPORT jstring JNICALL Java_com_windroid_MainActivity_getEngineStatus(
    JNIEnv* env, jobject obj) {
    
    if (!gWinCoreEngine) {
        return env->NewStringUTF("Engine not initialized");
    }

    // Get current execution state
    WinCoreEngine::ExecutionState state = gWinCoreEngine->getExecutionState();
    
    std::string stateStr;
    switch (state) {
        case WinCoreEngine::ExecutionState::IDLE:
            stateStr = "IDLE - Ready to load binary";
            break;
        case WinCoreEngine::ExecutionState::LOADING:
            stateStr = "LOADING - Binary being loaded";
            break;
        case WinCoreEngine::ExecutionState::RUNNING:
            stateStr = "RUNNING - Binary executing";
            break;
        case WinCoreEngine::ExecutionState::SUSPENDED:
            stateStr = "SUSPENDED - Execution paused";
            break;
        case WinCoreEngine::ExecutionState::COMPLETED:
            stateStr = "COMPLETED - Execution finished";
            break;
        case WinCoreEngine::ExecutionState::ERROR:
            stateStr = "ERROR - " + gWinCoreEngine->getLastError();
            break;
    }

    LOGI("getEngineStatus: %s", stateStr.c_str());
    return env->NewStringUTF(stateStr.c_str());
}

/**
 * Java_com_windroid_MainActivity_getLastError
 * 
 * JNI function to retrieve last error message from WinCore
 * 
 * @param env - JNI environment pointer
 * @param obj - Java object reference
 * @return Java String containing last error message
 */
JNIEXPORT jstring JNICALL Java_com_windroid_MainActivity_getLastError(
    JNIEnv* env, jobject obj) {
    
    if (!gWinCoreEngine) {
        return env->NewStringUTF("Engine not initialized");
    }

    std::string error = gWinCoreEngine->getLastError();
    LOGI("getLastError: %s", error.c_str());
    
    return env->NewStringUTF(error.c_str());
}
