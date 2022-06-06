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
                                                        jfloatArray b, jfloatArray result) {
    // get Java array size
    jsize count = env->GetArrayLength(a);
    assert( env->GetArrayLength(a) == env->GetArrayLength(result));
    assert( env->GetArrayLength(b) == env->GetArrayLength(result));

    // get Java array data pointers
    jfloat* aData = env->GetFloatArrayElements(a, 0);
    jfloat* bData = env->GetFloatArrayElements(b, 0);
    jfloat* resultData = env->GetFloatArrayElements(result, 0);


    size_t globalSize, localSize, bytes;
    cl_int err;

    // Number of work items in each local work group
    localSize = 64;

    // Number of total work items - localSize must be divisor
    globalSize = ceil(count / (float) localSize) * localSize;

    // Number of bytes to transfer
    bytes = count * sizeof(jfloat);

    // Create the input and output arrays in device memory for our calculation
    cl_mem aBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, NULL);
    cl_mem bBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, NULL);
    cl_mem resultBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, bytes, NULL, NULL);

    // Write our data set into the input array in device memory
    err = clEnqueueWriteBuffer(queue, aBuffer, CL_TRUE, 0, bytes, aData, 0, NULL, NULL);
    if (err) LOGE("Error @clEnqueueWriteBuffer:%d\n", err);

    err |= clEnqueueWriteBuffer(queue, bBuffer, CL_TRUE, 0, bytes, bData, 0, NULL, NULL);
    if (err) LOGE("Error @clEnqueueWriteBuffer:%d\n", err);

    // Set the arguments to our compute kernel
    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &aBuffer);
    if (err) LOGE("Error @clSetKernelArg:%d\n", err);

    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &bBuffer);
    if (err) LOGE("Error @clSetKernelArg:%d\n", err);

    err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &resultBuffer);
    if (err) LOGE("Error @clSetKernelArg:%d\n", err);

    // Execute the kernel over the entire range of the data set
    err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalSize, &localSize, 0, NULL, NULL);
    if (err) LOGE("Error @clEnqueueNDRangeKernel:%d\n", err);

    // Wait for the command queue to get serviced before reading back results
    clFinish(queue);

    // Read the results from the device
    clEnqueueReadBuffer(queue, resultBuffer, CL_TRUE, 0, bytes, resultData, 0, NULL, NULL);

    // release OpenCL resources
    clReleaseMemObject(aBuffer);
    clReleaseMemObject(bBuffer);
    clReleaseMemObject(resultBuffer);

    // release Java arrays
    env->ReleaseFloatArrayElements(a, aData, 0);
    env->ReleaseFloatArrayElements(b, bData, 0);
    env->ReleaseFloatArrayElements(result, resultData, 0);
}