//
// Created by sanjeev on 02/05/25.
//

#include "arcore_manager.h"

#define LOG_TAG "ARCore Manager"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

/* Now only the pointer variable is in the .data(initialized global static) process memory segment */
ARCoreManager *manager = nullptr;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_my_1native_1app_ARNative_onCreate(JNIEnv *env, jobject thiz, jobject context) {
    jclass context_class = env->GetObjectClass(context);
    jmethodID get_assets_method = env->GetMethodID(context_class, "getAssets", "()Landroid/content/res/AssetManager;");
    jobject asset_manager_jobj = env->CallObjectMethod(context, get_assets_method);

    AAssetManager* asset_manager = AAssetManager_fromJava(env, asset_manager_jobj);

    manager = new ARCoreManager;
    manager->Initialize(env, context, asset_manager);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_my_1native_1app_ARNative_onResume(JNIEnv *env, jobject thiz, jobject activity) {
    manager->Resume();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_my_1native_1app_ARNative_onPause(JNIEnv *env, jobject thiz) {
    manager->Pause();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_my_1native_1app_ARNative_onSurfaceCreated(JNIEnv *env, jobject thiz) {
    manager->OnSurfaceCreated();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_my_1native_1app_ARNative_onDrawFrame(JNIEnv *env, jobject thiz, jint width, jint height, jint displayRotation) {
    manager->OnDrawFrame(width, height, displayRotation);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_my_1native_1app_ARNative_onTouch(JNIEnv *env, jobject thiz, jfloat x, jfloat y) {
    manager->OnTouch(x, y);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_my_1native_1app_ARNative_onRotateCube(JNIEnv *env, jobject thiz, jfloat degrees) {
    manager->RotateCube(degrees);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_my_1native_1app_ARNative_onScaleCube(JNIEnv *env, jobject thiz, jfloat scale) {
    manager->ScaleCube(scale);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_my_1native_1app_ARNative_onTranslateCube(JNIEnv *env, jobject thiz, jfloat x, jfloat y, jfloat z) {
    manager->TranslateCube(x, y, z);
}