/*
****************************************************
Copyright (C), 1988-1999, Reach Tech. Co., Ltd.

File name:     	write.c

Description:    收取编码之后的数据进行网络发送

Date:     		2010-11-17

Author:	  		yangshh

version:  		V1.0

*****************************************************
*/
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <xdc/std.h>
#include <ti/sdo/dmai/Fifo.h>
#include <ti/sdo/dmai/BufTab.h>
#include <ti/sdo/dmai/BufferGfx.h>
#include <ti/sdo/dmai/Rendezvous.h>
#include <ti/sdo/ce/video1/videnc1.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>

#include "demo.h"
#include "common.h"
#include "writer.h"
#include "tcpcom.h"
#include "stream_output_struct.h"
#include "app_stream_output.h"

#include "log_common.h"
unsigned char g_h264Header[H264_HEADER_LEN];
unsigned char g_h264LowHeader[H264_HEADER_LEN];
static int g_idr_header_length = 0;
static int g_idr_low_header_length = 0;

/* Number of buffers in writer pipe */
#define NUM_WRITER_BUFS         2

#ifndef AV_RB16
#   define AV_RB16(x)                           \
	((((const uint8_t*)(x))[0] << 8) |          \
	 ((const uint8_t*)(x))[1])
#endif

#ifndef AV_RB24
#define AV_RB24(x)  ((((uint8_t*)(x))[0] << 16) | \
                     (((uint8_t*)(x))[1] <<  8) | \
                     ((uint8_t*)(x))[2])
#endif

#ifndef AV_RB32
#   define AV_RB32(x)                           \
	((((const uint8_t*)(x))[0] << 24) |         \
	 (((const uint8_t*)(x))[1] << 16) |         \
	 (((const uint8_t*)(x))[2] <<  8) |         \
	 ((const uint8_t*)(x))[3])
#endif

/*查找H264起始NAL*/
int FindH264StartNAL(unsigned char *pp)
{
#if 0

	/*is for 00 00 00 01 Nal header*/
	if(pp[0] != 0 || pp[1] != 0 || pp[2] != 0 || pp[3] != 1) {
		return 0;
	} else {
		return 1;
	}

#endif

	if(AV_RB32(pp) == 0x00000001 || AV_RB24(pp) == 0x000001) {
		return 1;
	}

	return 0;
}

/*解析IDR 帧头长度*/
int ParseIDRHeader(unsigned char *pdata)
{
	int len = 0x17;
	int ret = 0, I_frame_header_length = 0;
	unsigned char *find = pdata;

	while(find) {
		ret = FindH264StartNAL(find);

		if(ret) {
			I_frame_header_length++ ;

			if(I_frame_header_length >= 3) {
				break;
			} else {
				find += 3;    //find next NAL header
			}
		}

		find++;
		len = (int)(find - pdata);

		if(len > H264_HEADER_LEN) {
			len = 0x18;
			break;
		}
	}

	DEBUG(DL_DEBUG, "length = %d\n", len);
	return len;
}

/*create temp buffer*/
Buffer_Handle CreateBuffer(int width, int height, ColorSpace_Type csType)
{
	BufferGfx_Attrs         gfxAttrs         = BufferGfx_Attrs_DEFAULT;
	int bufSize;
	Buffer_Handle hBuf;
	gfxAttrs.dim.x = 0;
	gfxAttrs.dim.y = 0;
	gfxAttrs.dim.width      = width;
	gfxAttrs.dim.height     = height;
	gfxAttrs.dim.lineLength = BufferGfx_calcLineLength(width, csType);

	if(csType == ColorSpace_YUV422PSEMI) {
		bufSize = gfxAttrs.dim.lineLength * gfxAttrs.dim.height * 2;
	} else {
		bufSize = gfxAttrs.dim.lineLength * gfxAttrs.dim.height * 1.5;
	}

	hBuf = Buffer_create(bufSize, (Buffer_Attrs *) &gfxAttrs);

	if(hBuf) {
		BufferGfx_setColorSpace(hBuf, csType);
		BufferGfx_setDimensions(hBuf, &gfxAttrs.dim);
		Buffer_setNumBytesUsed(hBuf, 0);
	}

	return hBuf;
}

int WriteToFile(char *buf, int len)
{
	FILE *fp;
	fp = fopen("idr.264", "w+");

	if(fp == NULL) {
		DEBUG(DL_ERROR, "Open File Failed!!!\n");
		return -1;
	}

	fwrite(buf, len, 1, fp);
	fclose(fp);
	return 0;
}

extern unsigned long long app_get_start_time();
/*收取编码之后的数据进行网络发送*/
Void *writerThrFxn(Void *arg)
{
	PRINTF("get pid= %d\n", getpid());
	DEBUG(DL_DEBUG, "[writerThrFxn] start ...\n");
	WriterEnv          *envp            = (WriterEnv *) arg;
	Void               *status          = THREAD_SUCCESS;

	Buffer_Attrs        bAttrs          = Buffer_Attrs_DEFAULT;
	BufTab_Handle       hBufTab         = NULL;
	Buffer_Handle       hBufTabResize	= NULL;
	int 				frametype 		= 0;
	Buffer_Handle       hOutBuf;
	Int                 fifoRet;
	Int                 bufIdx;
	VideoParam videoPara;
	APP_VIDEO_DATA_INFO info;
	unsigned int capture_time = 0;
	unsigned long long rtp_time = 0;
	unsigned long long start_time = 0;
	unsigned long long last_time = 0;
	
	hBufTabResize = CreateBuffer(H264_MAX_ENCODE_WIDTH, H264_MAX_ENCODE_HIGHT + 8, ColorSpace_YUV420PSEMI);

	if(hBufTabResize == NULL) {
		ERR("Failed to allocate contiguous buffers\n");
		cleanup(THREAD_FAILURE);
	}

	DEBUG(DL_DEBUG, "[High] envp->outBufSize = %d \n", (int)envp->outBufSize);
	/*
	 * Create a table of buffers for communicating buffers to
	 * and from the video thread.
	 */
	hBufTab = BufTab_create(NUM_WRITER_BUFS, envp->outBufSize, &bAttrs);



	if(hBufTab == NULL) {
		ERR("Failed to allocate contiguous buffers\n");
		cleanup(THREAD_FAILURE);
	}

	/* Send all buffers to the video thread to be filled with encoded data */
	for(bufIdx = 0; bufIdx < NUM_WRITER_BUFS; bufIdx++) {
		if(Fifo_put(envp->to_video_c, BufTab_getBuf(hBufTab, bufIdx)) < 0) {
			ERR("Failed to send buffer to display thread\n");
			cleanup(THREAD_FAILURE);
		}
	}

	/* Signal that initialization is done and wait for other threads */
	Rendezvous_meet(envp->hRendezvousInit);
	unsigned int t1, t2;

	while(!gblGetQuit()) {
		/* Get an encoded buffer from the video thread */
		fifoRet = Fifo_get(envp->from_video_c, &hOutBuf);

		if(fifoRet < 0) {
			ERR("Failed to get buffer from video thread\n");
			cleanup(THREAD_FAILURE);
		}

		/* Did the video thread flush the fifo? */
		if(fifoRet == Dmai_EFLUSH) {
			cleanup(THREAD_SUCCESS);
		}
		
		capture_time = get_run_time();
		start_time =app_get_start_time();
		rtp_time = getCurrentTime();
		if(rtp_time < start_time)
		{
			PRINTF("warnning,the rtp_time=%lld,the start time = %lld\n",rtp_time,start_time);
			app_set_start_time();
			start_time =app_get_start_time();
			rtp_time = 0;
		}
		else
		{
			rtp_time = rtp_time -start_time;
		}

		if(last_time > rtp_time)
		{
			rtp_time = last_time + 1;
			//last_time = last_time; 
		}
		
		
		t1 = get_run_time();
#if 1

		/* Store the encoded frame to disk */
		if(Buffer_getNumBytesUsed(hOutBuf)) {
			frametype  = Buffer_getTimeStamp(hOutBuf);
			videoPara.nWidth = Buffer_getWidth(hOutBuf);		//视频宽
			videoPara.nHight = Buffer_getHeight(hOutBuf); 	//视频高

			memset(&info, 0, sizeof(info));
			info.width = videoPara.nWidth;
			info.height = videoPara.nHight;
			info.timestamp = capture_time;
			info.rtp_time = rtp_time&0xffffffff;
			last_time = rtp_time;
		//	printf("info.rtp_time  = %u\n",info.rtp_time );
			app_set_media_info(info.width, info.height, -1 , 0);

			if(frametype  == IVIDEO_I_FRAME || frametype == IVIDEO_IDR_FRAME) {
				if(frametype == IVIDEO_IDR_FRAME) {  //IDR frame
					g_idr_header_length = ParseIDRHeader((unsigned char *)Buffer_getUserPtr(hOutBuf));
					memcpy((void *)g_h264Header, (const void *)Buffer_getUserPtr(hOutBuf), g_idr_header_length);
					SendDataToClient(Buffer_getNumBytesUsed(hOutBuf), (unsigned char *)Buffer_getUserPtr(hOutBuf), 1, 0, videoPara.nWidth, videoPara.nHight);

					info.IsIframe = 1;
					app_stream_video_output((unsigned char *)Buffer_getUserPtr(hOutBuf), Buffer_getNumBytesUsed(hOutBuf), &info);

					DEBUG(DL_DEBUG, "IDR frame send data to client:%d  IDR header Length = %d\n", g_h264Header[3], g_idr_header_length);
				} else {
					memcpy(Buffer_getUserPtr(hBufTabResize), g_h264Header, g_idr_header_length);
					memcpy(Buffer_getUserPtr(hBufTabResize) + g_idr_header_length, Buffer_getUserPtr(hOutBuf), Buffer_getNumBytesUsed(hOutBuf));
					SendDataToClient(Buffer_getNumBytesUsed(hOutBuf) + g_idr_header_length, (unsigned char *)Buffer_getUserPtr(hBufTabResize), 1, 0, videoPara.nWidth, videoPara.nHight);

					info.IsIframe = 1;
					app_stream_video_output((unsigned char *)Buffer_getUserPtr(hBufTabResize), Buffer_getNumBytesUsed(hOutBuf) + g_idr_header_length, &info);

				}
			}	else {
				SendDataToClient(Buffer_getNumBytesUsed(hOutBuf), (unsigned char *)Buffer_getUserPtr(hOutBuf), 0, 0, videoPara.nWidth, videoPara.nHight);


				info.IsIframe = 0;
				app_stream_video_output((unsigned char *)Buffer_getUserPtr(hOutBuf), Buffer_getNumBytesUsed(hOutBuf), &info);

			}
		} else {
			DEBUG(DL_FLOW, "[writerThrFxn] Warning, writer received 0 byte encoded frame\n");
		}

#endif
		t2 = get_run_time();

		if(t2 - t1 > 30) {
			DEBUG(DL_DEBUG, "[High Rate]  t2 - t1 = %d \n", t2 - t1);
		}

		/* Return buffer to capture thread */
		if(Fifo_put(envp->to_video_c, hOutBuf) < 0) {
			ERR("Failed to send buffer to display thread\n");
			cleanup(THREAD_FAILURE);
		}

	}

cleanup:
	DEBUG(DL_DEBUG, "[writerThrFxn] clean XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
	/* Make sure the other threads aren't waiting for us */
	Rendezvous_force(envp->hRendezvousInit);
	Fifo_flush(envp->from_video_c);
	/* Meet up with other threads before cleaning up */
	Rendezvous_meet(envp->hRendezvousCleanup);

	if(hBufTab) {
		BufTab_delete(hBufTab);
	}

	if(hBufTabResize) {
		Buffer_delete(hBufTabResize);
	}

	DEBUG(DL_DEBUG, "[writerThrFxn] clean XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX end\n");
	return status;
}

/*LOW RATE Send thread*/
Void *writerLowThrFxn(Void *arg)
{
	PRINTF("get pid= %d\n", getpid());
	DEBUG(DL_DEBUG, "[writerLowThrFxn] start ...\n");
	WriterEnv          *envp            = (WriterEnv *) arg;
	Void               *status          = THREAD_SUCCESS;
	Buffer_Attrs        bAttrs          = Buffer_Attrs_DEFAULT;

	BufTab_Handle 	    hBufTab		= NULL;//dd
	Buffer_Handle		hOutBufResize;

	Int                 fifoRet;
	Int                 bufIdx;
	Buffer_Handle           hBufTabResize	= NULL;
	int frametype = 0;
	// FILE *fp ;
	//fp = fopen("testlowbit.264","w+");


	hBufTabResize = CreateBuffer(704, 576, ColorSpace_YUV420PSEMI); //--buffersize=3133440

	if(hBufTabResize == NULL) 	{
		ERR("Failed to allocate contiguous buffers\n");
		cleanup(THREAD_FAILURE);
	}

	DEBUG(DL_DEBUG, "[Low] envp->outBufSize = %d \n", (int)envp->outBufSize);
	hBufTab = BufTab_create(NUM_WRITER_BUFS, envp->outBufSize, &bAttrs);

	if(hBufTab == NULL) 	{
		ERR("[writerThrFxn] Failed to allocate contiguous buffers\n");
		cleanup(THREAD_FAILURE);
	}

	for(bufIdx = 0; bufIdx < NUM_WRITER_BUFS; bufIdx++) {
		fifoRet = Fifo_put(envp->to_writelow_c, BufTab_getBuf(hBufTab, bufIdx));

		if(fifoRet < 0)	{
			ERR("[writerThrFxn] Failed to send buffer to display thread\n");
			cleanup(THREAD_FAILURE);
		}
	}

	unsigned int t1, t2 = 0;
	Rendezvous_meet(envp->hRendezvousInit);

	while(!gblGetQuit()) {
		fifoRet = Fifo_get(envp->from_writelow_c, &hOutBufResize);

		if(fifoRet < 0) 	{
			ERR("Failed to get buffer from video thread\n");
			cleanup(THREAD_FAILURE);
		}

		t1 = get_run_time();

		if(Buffer_getNumBytesUsed(hOutBufResize)) {
			frametype  = Buffer_getTimeStamp(hOutBufResize);

			if(frametype  == IVIDEO_I_FRAME || frametype == IVIDEO_IDR_FRAME) {
				if(frametype == IVIDEO_IDR_FRAME) {
					g_idr_low_header_length = ParseIDRHeader((unsigned char *)Buffer_getUserPtr(hOutBufResize));
					memcpy(g_h264LowHeader, (unsigned char *)Buffer_getUserPtr(hOutBufResize), g_idr_low_header_length);
					SendLowRateToClient(Buffer_getNumBytesUsed(hOutBufResize), (unsigned char *)Buffer_getUserPtr(hOutBufResize), 1, 0);
					DEBUG(DL_DEBUG, "Resize IDR frame send data to client:%d\n", g_h264LowHeader[3]);
				} else {
					memcpy(Buffer_getUserPtr(hBufTabResize), g_h264LowHeader, g_idr_low_header_length);
					memcpy(Buffer_getUserPtr(hBufTabResize) + g_idr_low_header_length, Buffer_getUserPtr(hOutBufResize), Buffer_getNumBytesUsed(hOutBufResize));
					SendLowRateToClient(Buffer_getNumBytesUsed(hOutBufResize) + g_idr_low_header_length, (unsigned char *)Buffer_getUserPtr(hBufTabResize), 1, 0);
				}
			} else {
				SendLowRateToClient(Buffer_getNumBytesUsed(hOutBufResize), (unsigned char *)Buffer_getUserPtr(hOutBufResize), 0, 0);
			}
		} else {
			DEBUG(DL_FLOW, "[writerLowThrFxn] Warning, writer received 0 byte encoded frame\n");
		}

		t2 = get_run_time();

		if(t2 - t1 > 30) {
			DEBUG(DL_DEBUG, "[Low Rate]  t2 - t1 = %d \n", t2 - t1);
		}

		fifoRet = Fifo_put(envp->to_writelow_c, hOutBufResize);

		if(fifoRet < 0) {
			ERR("Failed to send buffer to display thread\n");
			cleanup(THREAD_FAILURE);
		}
	}

cleanup:
	DEBUG(DL_FLOW, "[writerLowThrFxn] cleanup -----------------------------!!!\n");
	/* Make sure the other threads aren't waiting for us */
	Rendezvous_force(envp->hRendezvousInit);
	Fifo_flush(envp->to_writelow_c);

	/* Meet up with other threads before cleaning up */
	Rendezvous_meet(envp->hRendezvousCleanup);

	if(hBufTab) {
		BufTab_delete(hBufTab);
	}

	if(hBufTabResize) {
		Buffer_delete(hBufTabResize);
	}

	DEBUG(DL_FLOW, "[writerLowThrFxn] cleanup ----------------------------- end !!!\n");
	return status;
}




static APP_MEDIA_INFO g_media_info[2] ;

int app_set_media_info(int width, int height, int sample , int num)
{
	if(num < 0 || num >= 2) {
		PRINTF("NUM is error\n");
		return -1;
	}

	if(352 <= width &&  width <= 1980) {
		g_media_info[num].width = width;
	}

	if(288 <= height &&  height <= 1200) {
		g_media_info[num].height = height;
	}

	//now just support sample 44100/48000
	if(sample == 44100 || sample == 48000) {
		g_media_info[num].sample = sample;
	}

	return 0;

}

int app_get_media_info(int *width, int *height, int *sample, int num)
{
	int w, h, s;
	w = h = s = 0;
	w = g_media_info[num].width;
	h = g_media_info[num].height;
	s = g_media_info[num].sample;

	if(352 <= w &&  w <= 1980) {
		*width = w;
	} else {
		PRINTF("error,width is error,=%d.", w);
		return -1;
	}

	if(288 <= h &&  h <= 1200) {
		*height = h;
	} else {
		PRINTF("error,height is error,=%d.", h);
		return -1;
	}

	//now just support sample 44100/48000
	if(s  == 44100 || s  == 48000) {
		*sample = s;
	} else {
		PRINTF("error,sample is error,=%d.", s);
		return -1;
	}

	return 0;
}
