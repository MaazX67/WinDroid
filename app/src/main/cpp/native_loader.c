#include <jni.h>
#include <stdio.h>
#include <stdint.h>
JNIEXPORT jstring JNICALL Java_com_windroid_MainActivity_checkExe(JNIEnv *env, jobject thiz, jstring path) {
    const char *cpath = (*env)->GetStringUTFChars(env, path, 0);
    FILE *f = fopen(cpath, "rb");
    if(!f) return (*env)->NewStringUTF(env, "File nahi mila!");
    uint8_t m[2]; fread(m,1,2,f); fclose(f);
    if(m[0]=='M' && m[1]=='Z') return (*env)->NewStringUTF(env, "[Native] Valid.exe! PE OK - Ready to run with My_Kernel32");
    else return (*env)->NewStringUTF(env, "[Native] Ye.exe nahi hai!");
}
