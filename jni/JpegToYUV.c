/****************************************************************************
 * FileName: JpegToYUV.c
 * Description: 
 * Author: Murphys
 * Date: 2016.06.07
 * 
 ****************************************************************************/
 
#include "JpegToYUV.h"

#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include "turbojpeg.h"
#include "jpeglib.h"
#include "jpegint.h"

#include "ErrorDefine.h"

#define PAD(v, p) ((v+(p)-1)&(~((p)-1)))

 // 
extern void jpeg_mem_src_tj(j_decompress_ptr, unsigned char *, unsigned long);

 //-----------------------------------------------------------------------------
eRetMsg JpegToNV21(
	const unsigned char * pDataJpg,
	int nWidth,
	int nHeight,
	int nJpgLen,
	unsigned char * pDataNV21
)
{
	eRetMsg retMsg = ERROR_MSG_OK;
	{
		if (nWidth > 0 && nHeight > 0)
		{
			unsigned char * pOutputY = pDataNV21;
			unsigned char * pOutputV = pOutputY + nWidth * nHeight;
			unsigned char * pOutputU = pOutputV + 1;

			unsigned char * pYLine = pOutputY;
			int cw[MAX_COMPONENTS] = { 0 };
			int ch[MAX_COMPONENTS] = { 0 };
			int iw[MAX_COMPONENTS] = { 0 };
			int tmpbufsize = 0;
			int th[MAX_COMPONENTS] = { 0 };
			JSAMPLE * _tmpbuf = NULL, *ptr = pOutputY;
			JSAMPROW * tmpbuf[MAX_COMPONENTS] = { NULL };
			int nLinesPerCycleY = 0;
			int row = 0;
			int nLinesFor = 0;
			int nRestRow = 0;
			boolean bNeedJumpLines = FALSE;

			struct jpeg_decompress_struct dinfo;
			struct jpeg_error_mgr jerr;
			dinfo.err = jpeg_std_error(&jerr);
			jpeg_create_decompress(&dinfo);
			jpeg_mem_src_tj(&dinfo, (unsigned char*)pDataJpg, nJpgLen);
			jpeg_read_header(&dinfo, TRUE);

			if (dinfo.max_v_samp_factor == 2 && dinfo.max_h_samp_factor == 2)
			{
				dinfo.comp_info[0].v_samp_factor = 2;
				dinfo.comp_info[0].h_samp_factor = 2;
				dinfo.comp_info[1].v_samp_factor = 1;
				dinfo.comp_info[1].h_samp_factor = 1;
				dinfo.comp_info[2].v_samp_factor = 1;
				dinfo.comp_info[2].h_samp_factor = 1;
			}
			else if (dinfo.max_h_samp_factor == 2 && dinfo.max_v_samp_factor == 1)
			{
				dinfo.comp_info[0].v_samp_factor = 1;
				dinfo.comp_info[0].h_samp_factor = 2;
				dinfo.comp_info[1].v_samp_factor = 1;
				dinfo.comp_info[1].h_samp_factor = 1;
				dinfo.comp_info[2].v_samp_factor = 1;
				dinfo.comp_info[2].h_samp_factor = 1;
				bNeedJumpLines = TRUE;
			}
			dinfo.raw_data_out = TRUE;
			jpeg_calc_output_dimensions(&dinfo);
			{
				jpeg_component_info *compptr = &dinfo.comp_info[0];
				int ih;
				iw[0] = compptr->width_in_blocks*DCTSIZE;
				ih = compptr->height_in_blocks*DCTSIZE;
				cw[0] = PAD(dinfo.image_width, dinfo.max_h_samp_factor)
					*compptr->h_samp_factor / dinfo.max_h_samp_factor;
				ch[0] = PAD(dinfo.image_height, dinfo.max_v_samp_factor)
					*compptr->v_samp_factor / dinfo.max_v_samp_factor;
				th[0] = compptr->v_samp_factor*DCTSIZE;
			}
			for (int i = 1; i < dinfo.num_components; i++)
			{
				jpeg_component_info *compptr = &dinfo.comp_info[i];
				int ih;
				iw[i] = compptr->width_in_blocks*DCTSIZE;
				ih = compptr->height_in_blocks*DCTSIZE;

				int pw[MAX_COMPONENTS], ph[MAX_COMPONENTS];
				pw[i] = PAD(dinfo.output_width, dinfo.max_h_samp_factor)
					*compptr->h_samp_factor / dinfo.max_h_samp_factor;
				ph[i] = PAD(dinfo.output_height, dinfo.max_v_samp_factor)
					*compptr->v_samp_factor / dinfo.max_v_samp_factor;

				cw[i] = PAD(dinfo.image_width, dinfo.max_h_samp_factor)
					*compptr->h_samp_factor / dinfo.max_h_samp_factor;
				ch[i] = PAD(dinfo.image_height, dinfo.max_v_samp_factor)
					*compptr->v_samp_factor / dinfo.max_v_samp_factor;
				th[i] = compptr->v_samp_factor*DCTSIZE;
				tmpbufsize += iw[i] * th[i];
			}
			if (cw[1] != cw[2])
			{
				retMsg = ERROR_MSG_INVALID_PARAM;
				goto YVU420spFailedOut;
			}
			if ((_tmpbuf = (JSAMPLE *)malloc(sizeof(JSAMPLE)*tmpbufsize)) == NULL)
			{
				retMsg = ERROR_MSG_OUT_OF_MEM;
				goto YVU420spFailedOut;
			}
			ptr = _tmpbuf;
			if ((tmpbuf[0] = (JSAMPROW *)malloc(sizeof(JSAMPROW)*th[0])) == NULL)
			{
				retMsg = ERROR_MSG_OUT_OF_MEM;
				goto YVU420spFailedOut;
			}

			for (int i = 1; i < dinfo.num_components; i++)
			{
				if ((tmpbuf[i] = (JSAMPROW *)malloc(sizeof(JSAMPROW)*th[i])) == NULL)
				{
					retMsg = ERROR_MSG_OUT_OF_MEM;
					goto YVU420spFailedOut;
				}
				
				for (int row = 0; row < th[i]; row++)
				{
					tmpbuf[i][row] = ptr;
					ptr += iw[i];
				}
			}
			dinfo.scale_num = 1;
			dinfo.scale_denom = 1;
			jpeg_start_decompress(&dinfo);
			nLinesPerCycleY = th[0];
			nLinesFor = (int)dinfo.output_height / nLinesPerCycleY * nLinesPerCycleY;
			for (row = 0; row < nLinesFor; /*row+=dinfo.max_v_samp_factor*DCTSIZE*/)
			{
				JSAMPARRAY yuvptr[MAX_COMPONENTS];
				int crow[MAX_COMPONENTS];
				for (int i = 0; i < nLinesPerCycleY; i++)
				{
					tmpbuf[0][i] = pYLine;
					pYLine += cw[0];
				}
				for (int i = 0; i < dinfo.num_components; i++)
				{
					jpeg_component_info *compptr = &dinfo.comp_info[i];
					crow[i] = row*compptr->v_samp_factor / dinfo.max_v_samp_factor;
					yuvptr[i] = tmpbuf[i];
				}
				row += jpeg_read_raw_data(&dinfo, yuvptr, dinfo.max_v_samp_factor*DCTSIZE);

				if (bNeedJumpLines)
				{
					if (row % (dinfo.max_v_samp_factor*dinfo.min_DCT_scaled_size * 2) == 0)
					{
						continue;
					}
				}

				// VU
				if (bNeedJumpLines)
				{
					int nLinesPerCycleUV = th[1];
					int nBytePerLineC = cw[1];
					for (int j = 0; j < nLinesPerCycleUV; j++)
					{
						for (int i = 0; i < nBytePerLineC; i++)
						{
							*pOutputU = *(tmpbuf[1][j] + i);
							pOutputU += 2;
							*pOutputV = *(tmpbuf[2][j] + i);
							pOutputV += 2;
						}
					}
				}
				else
				{
					int nLinesPerCycleUV = th[1];
					int nBytePerLineC = cw[1];
					for (int j = 0; j < nLinesPerCycleUV; j++)
					{
						for (int i = 0; i < nBytePerLineC; i++)
						{
							*pOutputU = *(tmpbuf[1][j] + i);
							pOutputU += 2;
							*pOutputV = *(tmpbuf[2][j] + i);
							pOutputV += 2;
						}
					}
				}
			}

			nRestRow = (int)dinfo.output_height - row;
			if (nRestRow > 0)
			{
				JSAMPARRAY yuvptr[MAX_COMPONENTS];
				int crow[MAX_COMPONENTS];
				for (int i = 0; i < nRestRow; i++)
				{
					tmpbuf[0][i] = pYLine;
					pYLine += cw[0];
				}
				for (int i = 0; i < dinfo.num_components; i++)
				{
					jpeg_component_info *compptr = &dinfo.comp_info[i];
					crow[i] = row*compptr->v_samp_factor / dinfo.max_v_samp_factor;
					yuvptr[i] = tmpbuf[i];
				}
				row += jpeg_read_raw_data(&dinfo, yuvptr, dinfo.max_v_samp_factor*dinfo.min_DCT_scaled_size);

				// VU
				if (bNeedJumpLines)
				{
					int nLinesPerCycleUV = nRestRow / 2;//剩下的行数nRestRow，决定uv的行数为nRestRow/2
					int nBytePerLineC = cw[1];
					for (int j = 0; j < nLinesPerCycleUV; j++)
					{
						for (int i = 0; i < nBytePerLineC; i++)
						{
							*pOutputU = *(tmpbuf[1][j] + i);
							pOutputU += 2;
							*pOutputV = *(tmpbuf[2][j] + i);
							pOutputV += 2;
						}
					}
				}
				else
				{
					int nLinesPerCycleUV = nRestRow / 2;//剩下的行数nRestRow，决定uv的行数为nRestRow/2
					int nBytePerLineC = cw[1];
					for (int j = 0; j < nLinesPerCycleUV; j++)
					{
						for (int i = 0; i < nBytePerLineC; i++)
						{
							*pOutputU = *(tmpbuf[1][j*2] + i);
							pOutputU += 2;
							*pOutputV = *(tmpbuf[2][j*2] + i);
							pOutputV += 2;
						}
					}
				}
			}

			jpeg_finish_decompress(&dinfo);
			jpeg_destroy_decompress(&dinfo);
			retMsg = ERROR_MSG_OK;

YVU420spFailedOut:
			if (dinfo.global_state > DSTATE_START)
			{
				jpeg_abort_decompress(&dinfo);
			}
			for (int i = 0; i < MAX_COMPONENTS; i++)
			{
				if (tmpbuf[i])
					free(tmpbuf[i]);
			}
			if (_tmpbuf)
			{
				free(_tmpbuf);
			}
		}
		else
		{
			retMsg = "Jpeg decode head error";
		}
	}
	
	return retMsg;
}

