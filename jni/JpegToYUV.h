/****************************************************************************
 * FileName: JpegToYUV.h
 * Description: 
 * Author: Murphys
 * Date: 2016.06.07
 * 
 ****************************************************************************/
#ifndef _H_JPEGTOYUV_
#define _H_JPEGTOYUV_

#include "ErrorDefine.h"

// 
eRetMsg JpegToNV21(
	const unsigned char * pDataJpg,
	int nWidth,
	int nHeight,
	int nJpgLen,
	unsigned char * pDataNV21
);



#endif