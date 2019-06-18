/****************************************************************************
 * FileName: YUVToJpeg.h
 * Description: 
 * Author: Murphys
 * Date: 2016.06.07
 * 
 ****************************************************************************/
#ifndef _H_YUVTOJPEG_
#define _H_YUVTOJPEG_
 
#include "ErrorDefine.h"
#include <malloc.h>

eRetMsg NV21ToJpeg(
	const unsigned char * pDataNV21,
	int nNV21Width,
	int nNV21Height,
	unsigned char ** ppDataJpg,			// must be free by call: free(*ppDataJpg)
	int * nJpgLen
);






#endif
