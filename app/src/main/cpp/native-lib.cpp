#include <jni.h>
#include <string>

#include <android/log.h>
#define LOG    "ocl-jni"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG,__VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG,__VA_ARGS__)

#define CL_TARGET_OPENCL_VERSION 120
#include <CL/cl.h>

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

const static char *kernelSource =
        "__kernel void vecAdd(__global float *inputA, __global float *inputB, __global float *output)\n"\
            "{\n"\
            "  size_t id = get_global_id(0);\n"\
            "  output[id] = inputA[id] * inputB[id];\n"\
            "}\n";

cl_context context;               // context
cl_command_queue queue;           // command queue
cl_kernel kernel;                 // kernel

extern "C"
JNIEXPORT void JNICALL
Java_pl_edu_uksw_amap_ocl_1jni_MainActivity_initOCL(JNIEnv *env, jobject thiz) {
    cl_platform_id cpPlatform;
    cl_device_id device_id;
    cl_program program;

    cl_int err;

    // Bind to platform
    err = clGetPlatformIDs(1, &cpPlatform, NULL);
    if(err) LOGE("Error @clGetPlatformIDs:%d\n",err);

    // Get ID for the device
    err = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
    if(err) LOGE("Error @clGetDeviceIDs:%d\n",err);

    // Create a context
    context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
    if(err) LOGE("Error @clCreateContext:%d\n",err);

    // Create a command queue
    queue = clCreateCommandQueue(context, device_id, 0, &err);
    if(err) LOGE("Error @clCreateCommandQueue:%d\n",err);

    // Create the compute program from the source buffer
    program = clCreateProgramWithSource(context, 1,  (const char **) & kernelSource, NULL, &err);
    if(err) LOGE("Error @clCreateProgramWithSource:%d\n",err);

    // Build the program executable
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if(err) LOGE("Error @clBuildProgram:%d\n",err);

    // Create the compute kernel in the program we wish to run
    kernel = clCreateKernel(program, "vecAdd", &err);
    if(err) LOGE("Error @clCreateKernel:%d\n",err);

    clReleaseProgram(program);
}

extern "C"
JNIEXPORT void JNICALL
Java_pl_edu_uksw_amap_ocl_1jni_MainActivity_shutdownOCL(JNIEnv *env, jobject thiz) {
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
}
extern "C"
JNIEXPORT void JNICALL
Java_pl_edu_uksw_amap_ocl_1jni_MainActivity_multiplyOcl(JNIEnv *env, jobject thiz, jfloatArray a,
                                                        jfloatArray b, jfloatArray result,
                                                        jint rounds) {
    // TODO: implement multiplyOcl()
}