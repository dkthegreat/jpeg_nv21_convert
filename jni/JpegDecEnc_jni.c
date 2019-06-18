
#include <stdio.h>
#include <string.h>

#include "JpegDecEnc_jni.h"
#include "JpegToYUV.h"
#include "YUVToJpeg.h"

//
JNIEXPORT jint JNICALL Java_com_le_camera_jpegdecenc_MainActivity_ntvJpegToNV21(
JNIEnv * env, jobject obj,
jbyteArray jpgData,
jint width,
jint height,
jbyteArray nv21Data)
{
	jint nRet = -1;
	int nDataLenJpeg = (*env)->GetArrayLength(env, jpgData);
	if(nDataLenJpeg > 0)
	{
		jbyte * pDataJpeg = (*env)->GetByteArrayElements(env, jpgData, NULL);
		if(pDataJpeg)
		{
			jbyte * pDataNV21 = (*env)->GetByteArrayElements(env, nv21Data, NULL);
			if(pDataNV21)
			{
				eRetMsg retMsg = JpegToNV21(pDataJpeg, width, height, nDataLenJpeg, pDataNV21);
				if(!strcmp(retMsg, ERROR_MSG_OK))
				{
					nRet = 0;
				}
				(*env)->ReleaseByteArrayElements(env, nv21Data, pDataNV21, JNI_ABORT);
			}
			(*env)->ReleaseByteArrayElements(env, jpgData, pDataJpeg, JNI_ABORT);
		}
	}
	return nRet;
}

//
JNIEXPORT jbyteArray JNICALL Java_com_le_camera_jpegdecenc_MainActivity_ntvNV21ToJpeg(
JNIEnv * env, jobject obj,
jbyteArray nv21Data,
jint width,
jint height)
{
	jbyteArray retArrary = NULL;
	jbyte * pDataNV21 = (*env)->GetByteArrayElements(env, nv21Data, NULL);
	if(pDataNV21)
	{
		unsigned char * pDataJpg = NULL;
		int nJpgLen = 0;
		eRetMsg retMsg = NV21ToJpeg(pDataNV21, width, height, &pDataJpg, &nJpgLen);
		if(!strcmp(retMsg, ERROR_MSG_OK))
		{
			retArrary = (*env)->NewByteArray(env, nJpgLen);
			jbyte * pDataJpeg = (*env)->GetByteArrayElements(env, retArrary, NULL);
			if(pDataJpeg)
			{
				memcpy(pDataJpeg, pDataJpg, nJpgLen);
				(*env)->ReleaseByteArrayElements(env, retArrary, pDataJpeg, JNI_ABORT);
				free(pDataJpg);
			}
		}
		(*env)->ReleaseByteArrayElements(env, nv21Data, pDataNV21, JNI_ABORT);
	}
	return retArrary;
}
