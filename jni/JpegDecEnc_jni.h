#ifndef _JpegDecEnc_jni_H_
#define _JpegDecEnc_jni_H_


#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

//
JNIEXPORT jint JNICALL Java_com_le_camera_jpegdecenc_MainActivity_ntvJpegToNV21(
JNIEnv * env, jobject obj,
jbyteArray jpgData,
jint width,
jint height,
jbyteArray nv21Data);

//
JNIEXPORT jbyteArray JNICALL Java_com_le_camera_jpegdecenc_MainActivity_ntvNV21ToJpeg(
JNIEnv * env, jobject obj,
jbyteArray nv21Data,
jint width,
jint height);

#ifdef __cplusplus
}
#endif



#endif
