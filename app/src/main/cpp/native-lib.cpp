#include <jni.h>
#include <string>

extern "C" JNIEXPORT jstring JNICALL
Java_pl_edu_uksw_amap_ocl_1jni_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
extern "C"
JNIEXPORT void JNICALL
Java_pl_edu_uksw_amap_ocl_1jni_MainActivity_multiplyJni(JNIEnv *env, jobject thiz, jfloatArray a,
                                                        jfloatArray b, jfloatArray result) {
    // get Java array size
    jsize count = env->GetArrayLength(a);
    assert( env->GetArrayLength(a) == env->GetArrayLength(result));
    assert( env->GetArrayLength(b) == env->GetArrayLength(result));

    // get Java array data pointers
    jfloat* a_data = env->GetFloatArrayElements( a,0);
    jfloat* b_data = env->GetFloatArrayElements( b,0);
    jfloat* result_data = env->GetFloatArrayElements( result,0);

    // do the math!
    for (int i = 0; i < count; ++i) {
        result_data[i] = a_data[i] * b_data[i];
    }

    // release Java arrays
    env->ReleaseFloatArrayElements(a, a_data, 0);
    env->ReleaseFloatArrayElements(b, b_data, 0);
    env->ReleaseFloatArrayElements(result, result_data, 0);
}