#include <jni.h>
#include <string>

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_my_1native_1app_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_my_1native_1app_MainActivity_getNumber(JNIEnv *env, jobject thiz) {
    return 34;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_my_1native_1app_MainActivity_sendString(JNIEnv *env, jobject thiz, jstring data) {
    const char* nativeString = env->GetStringUTFChars(data, 0);
    printf("SANJU : %s", nativeString);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_my_1native_1app_MainActivity_processArray(JNIEnv *env, jobject thiz,
                                                           jintArray data) {
    jint *elements = env->GetIntArrayElements(data, nullptr);
    jsize length = env->GetArrayLength(data);
    int sum = 0;

    for(jsize i = 0; i < length; i++) {
        sum += elements[i];
    }

    env->ReleaseIntArrayElements(data, elements, 0);

    return sum;
}