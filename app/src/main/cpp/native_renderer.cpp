//
// Created by sanjeev on 02/05/25.
//

#include "arcore_manager.h"

#define LOG_TAG "ARCore Manager"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static ARCoreManager manager;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_my_1native_1app_ARNative_onCreate(JNIEnv *env, jobject thiz, jobject context, jobject activity) {
    manager.Initialize(env, context, activity);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_my_1native_1app_ARNative_onResume(JNIEnv *env, jobject thiz, jobject activity) {
    manager.Resume();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_my_1native_1app_ARNative_onPause(JNIEnv *env, jobject thiz) {
    manager.Pause();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_my_1native_1app_ARNative_onSurfaceCreated(JNIEnv *env, jobject thiz) {
    manager.OnSurfaceCreated();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_my_1native_1app_ARNative_onDrawFrame(JNIEnv *env, jobject thiz, jint width, jint height, jint displayRotation) {
    manager.OnDrawFrame(width, height, displayRotation);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_my_1native_1app_ARNative_onTouch(JNIEnv *env, jobject thiz, jfloat x, jfloat y) {
    manager.OnTouch(x, y);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_my_1native_1app_ARNative_onRotateCube(JNIEnv *env, jobject thiz, jfloat degrees) {
    manager.RotateCube(degrees);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_my_1native_1app_ARNative_onScaleCube(JNIEnv *env, jobject thiz, jfloat scale) {
    manager.ScaleCube(scale);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_my_1native_1app_ARNative_onTranslateCube(JNIEnv *env, jobject thiz, jfloat x, jfloat y, jfloat z) {
    manager.TranslateCube(x, y, z);
}