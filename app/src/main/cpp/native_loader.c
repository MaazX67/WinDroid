#include <jni.h>
JNIEXPORT jstring JNICALL Java_com_windroid_MainActivity_getVersion(JNIEnv *env, jobject thiz) {
    return (*env)->NewStringUTF(env, "WinDroid V4 Box64+Wine");
}
