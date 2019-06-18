/****************************************************************************
 * FileName: YUVToJpeg.c
 * Description: 
 * Author: Murphys
 * Date: 2016.06.07
 * 
 ****************************************************************************/
 
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include "YUVToJpeg.h"
#include "turbojpeg.h"
#include "jpeglib.h"
#include "jpegint.h"

 // 
extern void jpeg_mem_dest_tj(j_compress_ptr, unsigned char **, unsigned long *, boolean);

//----------------------------------------------------------------
static void deinterleave(
	unsigned char* vuPlanar,
	unsigned char* uRows,
	unsigned char* vRows,
	int rowIndex,
	int width,
	int fStrides[2])
{
	for (int row = 0; row < 8; ++row)
	{
		int offset = ((rowIndex >> 1) + row) * fStrides[1];
		unsigned char* vu = vuPlanar + offset;
		for (int i = 0; i < (width >> 1); ++i)
		{
			int index = row * (width >> 1) + i;
			uRows[index] = vu[0];
			vRows[index] = vu[1];
			vu += 2;
		}
	}
}

 //-----------------------------------------------------------------------------
eRetMsg NV21ToJpeg(
	const unsigned char * pDataNV21,
	int nNV21Width,
	int nNV21Height,
	unsigned char ** ppDataJpg,			// must be free by call: free(*ppDataJpg)
	int * nJpgLen
)
{
	eRetMsg retMsg = ERROR_MSG_OK;
	int outDataLen = 0;
	JSAMPROW y[16], cb[16], cr[16]; // y[2][5] = color sample of row 2 and pixel column 5; (one plane) 
	JSAMPARRAY data[3]; // t[0][2][5] = color sample 0 of row 2 and column 5 

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	data[0] = y;
	data[1] = cr;
	data[2] = cb;

	cinfo.err = jpeg_std_error(&jerr);  // Errors get written to stderr 
	jpeg_create_compress(&cinfo);
	cinfo.image_width = nNV21Width;
	cinfo.image_height = nNV21Height;
	cinfo.input_components = 3;
	jpeg_set_defaults(&cinfo);
	jpeg_set_colorspace(&cinfo, JCS_YCbCr);
	cinfo.raw_data_in = TRUE; // Supply downsampled data 
#if JPEG_LIB_VERSION >= 70 
							  //#warning using JPEG_LIB_VERSION >= 70 
	cinfo.do_fancy_downsampling = FALSE;  // Fix segfault with v7 
#endif 
	cinfo.comp_info[0].h_samp_factor = 2;
	cinfo.comp_info[0].v_samp_factor = 2;
	cinfo.comp_info[1].h_samp_factor = 1;
	cinfo.comp_info[1].v_samp_factor = 1;
	cinfo.comp_info[2].h_samp_factor = 1;
	cinfo.comp_info[2].v_samp_factor = 1;

	jpeg_set_quality(&cinfo, 80, TRUE);
	cinfo.dct_method = JDCT_FASTEST;
	unsigned char * pOutData = NULL;
	jpeg_mem_dest_tj(&cinfo, &pOutData, (unsigned long*)(&outDataLen), TRUE);        // Data written to file   jpeg_mem_dest_tj(&srcinfo,pData,inSize);

	jpeg_start_compress(&cinfo, TRUE);

	int fStrides[2] = { nNV21Width, nNV21Width };
	unsigned char* yPlanar = (unsigned char*)pDataNV21;
	unsigned char* vuPlanar = (unsigned char*)pDataNV21 + nNV21Width * nNV21Height; //width * height;
	unsigned char* uRows = (unsigned char*)malloc(8 * (nNV21Width >> 1));
	unsigned char* vRows = (unsigned char*)malloc(8 * (nNV21Width >> 1));

	// process 16 lines of Y and 8 lines of U/V each time.
	while (cinfo.next_scanline < cinfo.image_height)
	{

		if (cinfo.next_scanline + 16 <= cinfo.image_height)
		{
			//deitnerleave u and v
			deinterleave(vuPlanar, uRows, vRows, cinfo.next_scanline, nNV21Width, fStrides);

			for (int i = 0; i < 16; i++) {
				// y row
				y[i] = yPlanar + (cinfo.next_scanline + i) * fStrides[0];

				// construct u row and v row
				if ((i & 1) == 0) {
					// height and width are both halved because of downsampling
					int offset = (i >> 1) * (nNV21Width >> 1);
					cb[i / 2] = uRows + offset;
					cr[i / 2] = vRows + offset;
				}
			}
			jpeg_write_raw_data(&cinfo, data, 16);
		}
		else
		{
			//cinfo.next_scanline += cinfo.image_height - cinfo.next_scanline;
			int nRestRow = cinfo.image_height - cinfo.next_scanline;
			unsigned char * pTempVUPlanar = (unsigned char*)malloc(8 * nNV21Width);
			if (pTempVUPlanar)
			{
				unsigned char * pUVLine = pTempVUPlanar;
				for (int i = 0; i < 4; i++)
				{
					int offset = ((cinfo.next_scanline >> 1) + i) * fStrides[1];
					unsigned char* vu = vuPlanar + offset;
					memcpy(pUVLine, vu, fStrides[1]);
					pUVLine += fStrides[1];
					memcpy(pUVLine, vu, fStrides[1]);
				}

				//deitnerleave u and v
				deinterleave(pTempVUPlanar, uRows, vRows, 0, nNV21Width, fStrides);

				for (int i = 0; i < 16; i++) {
					// y row
					y[i] = yPlanar + (cinfo.next_scanline + i) * fStrides[0];

					// construct u row and v row
					if ((i & 1) == 0) {
						// height and width are both halved because of downsampling
						int offset = (i >> 1) * (nNV21Width >> 1);
						cb[i / 2] = uRows + offset;
						cr[i / 2] = vRows + offset;
					}
				}
				jpeg_write_raw_data(&cinfo, data, 16);
				free(pTempVUPlanar);
			}
		}

	}


	jpeg_finish_compress(&cinfo);
	free(uRows);
	uRows = NULL;
	free(vRows);
	vRows = NULL;

	jpeg_destroy_compress(&cinfo);
	if (pOutData != NULL)
	{
		*ppDataJpg = pOutData;
		*nJpgLen = outDataLen;
		retMsg = ERROR_MSG_OK;
	}

	return retMsg;
}

