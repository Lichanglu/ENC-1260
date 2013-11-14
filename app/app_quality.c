/****************************************************************************
* add by zhangmin
*    平移ENC1200的RESIZE相关逻辑，可以配套华为智真测试
* 													2011-05-08
*****************************************************************************/
#ifdef HAVE_QUALITY_MODULE
#include <xdc/std.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <ti/sdo/ce/Engine.h>
#include <ti/sdo/dmai/Fifo.h>
#include <ti/sdo/dmai/Pause.h>
#include <ti/sdo/dmai/BufTab.h>
#include <ti/sdo/dmai/VideoStd.h>
#include <ti/sdo/dmai/BufferGfx.h>
#include <ti/sdo/dmai/Rendezvous.h>
#include <ti/sdo/dmai/ce/Venc1.h>
#include <ti/sdo/codecs/h264fhdvenc/ih264fhdvenc.h>
#include <ti/sdo/ce/video1/videnc1.h>
#include <ti/sdo/dmai/Resize.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "sysparam.h"



#include "app_quality.h"
#include "log_common.h"

/*set quality*/
int g_QUA_LEVEL = NOT_QUA_LEVEL;
#define SET_QUA_LEVEL(qua)          (g_QUA_LEVEL = qua)
#define IIS_QUA_LEVEL()             (QUA_LEVEL == g_QUA_LEVEL)

CurrentResolutionStatus lock_status = LOCK_OUT;

/*get resize param*/

#define INIT_RESIZE_PARAM	{0,RESIZE_INVALID,RECREATE_INVALID,6,1280,1024,1280,576,0,0};
ResizeParam gResizeP = INIT_RESIZE_PARAM;
/*get resize param*/
//#define ISRESIZE()		(RESIZE_OK == gResizeP.flag)
static void ResizeLock();
static void ResizeUnlock();
extern int capture_height;
extern int capture_width;
extern SavePTable 	gSysParaT;

int AppIsResizeFlag()
{
	return (!NORESIZE);
}
int AppGetResizeReset()
{
	int flag = 0;
	ResizeLock();
	flag = gResizeP.re_enc;
	ResizeUnlock();
	return flag;
}
void AppSetResizeReset(int flag)
{
	ResizeLock();
	gResizeP.re_enc = flag;
	ResizeUnlock();
	return ;
}
static void ResizeLock()
{

}
static void ResizeUnlock()
{

}
int AppGetResizeHeight()
{
	int height = 0;
	ResizeLock();
	height = gResizeP.enc_h;
	ResizeUnlock();
	return height;
}

void AppSetResizeHeight(int height)
{
	ResizeLock();
	gResizeP.enc_h = height;
	ResizeUnlock();
	return ;
}

int AppGetResizeWidth()
{
	int width = 0;
	ResizeLock();
	width = gResizeP.enc_w;
	ResizeUnlock();
	return width;
}

void AppSetResizeWidth(int width)
{
	ResizeLock();
	gResizeP.enc_w = width;
	ResizeUnlock();
	return ;
}

void GetZoomInOutWH(int nSW, int nSH, int *nDW, int *nDH)
{
	float x = (float)nSW / (float)nSH;//宽高比
	float y = (float) * nDW / x;

	PRINTF("nSW=%d=%d,nDw = %d,nDh=%d\n", nSW, nSH, *nDW, *nDH);

	//int x = nSW / nSH;
	//int y = nDW / x;
	if(y - *nDH > 0) {
		x = (float)nSH / (float)nSW;
		y = (float) * nDH / x;

		if(*nDW - y > 0) {
			*nDW = (int) y;
			//*nDW = (*nDW / 8) * 8;
		}

		//	nDW = y;
	} else {
		*nDH = (int) y;
	}

	PRINTF("nSW=%d=%d,nDw = %d,nDh=%d\n", nSW, nSH, *nDW, *nDH);
	//	nDH = y;
	return;
}

int calcDstWH(int *width, int *height, int capture_width, int capture_height, int resizemode)
{
	PRINTF("width:[%d],height:[%d],capture_width:[%d],capture_height:[%d],resizemode=%d,\n",
	       *width, *height, capture_width, capture_height, resizemode);
	int dstWidth = *width;
	int dstHeight  = *height;

	if((*height >= capture_height) && (*width >= capture_width)) {
		if(resizemode) {
			gResizeP.dst_h = *height;
			gResizeP.dst_w = *width;
			gResizeP.x = 0;
			gResizeP.y = 0;
		} else {
			GetZoomInOutWH(capture_width, capture_height, &dstWidth, &dstHeight);
			gResizeP.dst_h = dstHeight;
			gResizeP.dst_w = dstWidth ;

			if(*height == dstHeight) {
				gResizeP.x = (*width - gResizeP.dst_w) / 2;
				gResizeP.y = 0;
			}

			if(*width == dstWidth) {
				gResizeP.x = 0;
				gResizeP.y = (*height - gResizeP.dst_h) / 2;
			}
		}

		gResizeP.enc_w = *width;
		gResizeP.enc_h = *height;
		gResizeP.black = 6;
		gResizeP.re_enc = RECREATE_OK;
		gResizeP.flag = ZOOMIN;//放大
		PRINTF("mode=%d,x =%d,y=%d,dst_h:[%d],dst_w:[%d]\n", resizemode, gResizeP.x, gResizeP.y, gResizeP.dst_h, gResizeP.dst_w);

	} else if((*height < capture_height) && (*width < capture_width)) {
		if(resizemode) {
			gResizeP.dst_h = *height;
			gResizeP.dst_w = *width;
			gResizeP.x = 0;
			gResizeP.y = 0;
			gResizeP.flag = ZOOMOUT;
		} else {
			GetZoomInOutWH(capture_width, capture_height, &dstWidth, &dstHeight);
			gResizeP.dst_h = dstHeight;
			gResizeP.dst_w = dstWidth;

			if(*height == dstHeight) {
				gResizeP.x = (*width - gResizeP.dst_w) / 2;
			}

			if(*width / 8 == dstWidth / 8) {
				gResizeP.y = (*height - gResizeP.dst_h) / 2;
			}

			gResizeP.flag = ZOOMIN;
		}
	//	PRINTF("x=%d,y=%d,w=%d,h=%d,flag=%d\n",gResizeP.x ,gResizeP.y,gResizeP.dst_w,gResizeP.dst_h,gResizeP.flag);
		gResizeP.enc_w = *width;
		gResizeP.enc_h = *height;
		gResizeP.black = 6;
		gResizeP.re_enc = RECREATE_OK;
		PRINTF("mode=%d,x =%d,y=%d,dst_h:[%d],dst_w:[%d]\n", resizemode, gResizeP.x, gResizeP.y, gResizeP.dst_h, gResizeP.dst_w);
	} else {
		if(*height <= capture_height) {
			if(resizemode) {
				gResizeP.dst_h = *height;
				gResizeP.dst_w = ((*height) * capture_width) / capture_height;
			} else {
				GetZoomInOutWH(capture_width, capture_height, &dstWidth, &dstHeight);
				gResizeP.dst_h = dstHeight;
				gResizeP.dst_w = dstWidth;
			}

			gResizeP.enc_w = *width;
			gResizeP.enc_h = *height;
			gResizeP.x = (*width - gResizeP.dst_w) / 2;
			gResizeP.y = 0;
			gResizeP.black = 6;
			gResizeP.flag = ZOOMIN;//放大
			gResizeP.re_enc = RECREATE_OK;
			PRINTF("mode=%d,x =%d,y=%d,dst_h:[%d],dst_w:[%d]\n", resizemode, gResizeP.x, gResizeP.y, gResizeP.dst_h, gResizeP.dst_w);

		} else if(*width <= capture_width) {
			if(resizemode) {
				gResizeP.dst_h = ((*width) * capture_height) / capture_width;
				gResizeP.dst_w = *width;
			} else {
				GetZoomInOutWH(capture_width, capture_height, &dstWidth, &dstHeight);
				gResizeP.dst_h = dstHeight;
				gResizeP.dst_w = dstWidth;
			}

			gResizeP.enc_w = *width;
			gResizeP.enc_h = *height;
			gResizeP.x = 0;
			gResizeP.y = (*height - gResizeP.dst_h) / 2;
			gResizeP.black = 6;
			gResizeP.flag = ZOOMIN;//放大
			gResizeP.re_enc = RECREATE_OK;
			PRINTF("mode=%d,x =%d,y=%d,dst_h:[%d],dst_w:[%d]\n", resizemode, gResizeP.x, gResizeP.y, gResizeP.dst_h, gResizeP.dst_w);
		}
	}

}
unsigned int CalcReaolution(int width, int height, int capture_width, int capture_height, int flag, int resizemode)
{
	if(1 == flag) {
		if((height == capture_height) && (width == capture_width)) {
			gResizeP.dst_h = capture_height;
			gResizeP.dst_w = capture_width;
			gResizeP.enc_w = width;
			gResizeP.enc_h = height;
			gResizeP.x = 0;
			gResizeP.y = 0;
			gResizeP.flag = RESIZE_INVALID;
			gResizeP.re_enc = RECREATE_OK;
			return;
		}

		if(width <= 1920 && height <= 1080) {
			calcDstWH(&width, &height, capture_width, capture_height, resizemode);
		}
	} else {
		gResizeP.dst_h = capture_height;
		gResizeP.dst_w = capture_width;
		gResizeP.x = 0;
		gResizeP.y = 0;
		gResizeP.enc_w = capture_width;
		gResizeP.enc_h = capture_height;
		gResizeP.flag = RESIZE_INVALID;
	}

	return gResizeP.flag;

}


/*set Quality Resize*/
int AppSetQualityResize(unsigned char data[], int len)
{
	int width = 0 , height = 0 , bitrate = 0, quality = 0;
	VideoParam param;
	int resizemode =  0, tmp = 0;

	if(len < 13) {
		DEBUG(DL_WARNING, "SetQualityResize() len < %d\n", len);
		return -1;
	}

	SET_QUA_LEVEL(NOT_QUA_LEVEL);
	quality = data[0];
	width = data[1] | (data[2] << 8) | data[3] << 16 | data[4] << 24;
	height = data[5] | (data[6] << 8) | data[7] << 16 | data[8] << 24;
	bitrate = data[9] | (data[10] << 8) | data[11] << 16 | data[12] << 24;
	DEBUG(DL_DEBUG, "[AppSetQualityResize] Recv width = %d  height = %d  bitrate = %d,quality=%d\n", width , height, bitrate, quality);

	if(bitrate < 128) {
		bitrate = 128;
	}

	ResizeLock();
	gResizeP.dbstatus = 1;
	getResizeMode(tmp, &resizemode);
	CalcReaolution(width, height, capture_width, capture_height, 1, resizemode);
	DEBUG(DL_DEBUG, "[CalcReaolution]  gResizeP.dst_w = %d  gResizeP.dst_h = %d  gResizeP.enc_w = %d,gResizeP.enc_h=%d,gResizeP.x=%d,gResizeP.y=%d,gResizeP.flag=%X,gResizeP.re_enc=%X\n",
	      gResizeP.dst_w , gResizeP.dst_h, gResizeP.enc_w, gResizeP.enc_h, gResizeP.x, gResizeP.y, gResizeP.flag, gResizeP.re_enc);
	memcpy(&param, &gSysParaT.videoPara, sizeof(VideoParam));
	/*bitrate*/
	param.sCbr = 1;
	param.sBitrate = bitrate;
	SetVideoParam(0, (unsigned char *)&param, sizeof(VideoParam));
	ResizeUnlock();
	return 0;
}

/*is resize*/
int AppISresize(void)
{
	ResizeLock();
	ResizeParam *resize = &gResizeP;

	int width = H264Width();
	int height = H264Height();
	int ret = 1;

	//DEBUG(DL_DEBUG,"[AppISresize]  ++++++++++++++ rwidth = %d  rheight = %d  width = %d  height = %d\n",resize->dst_w ,resize->dst_h, width ,height);
	if(height < resize->enc_h) {
		ret = 0;
	}

	if(width  < resize->enc_w) {
		ret = 0;
	}

	ResizeUnlock();
	ret = 1;
	return (ret);
}



/*resize 420 data*/
int AppQuality420YUV(Resize_Handle hRsz, Buffer_Handle hSrcBuf, Buffer_Handle hDstResize)
{
	BufferGfx_Dimensions    srcDim, dstDim, oldDim;
	//ResizeParam *resize = &gResizeP;
	int width  = 0 , height = 0 ;

	width =	gResizeP.dst_w;
	height =	gResizeP.dst_h;
	BufferGfx_getDimensions(hSrcBuf, &dstDim);
	dstDim.width = H264Width();
	dstDim.height = H264Height();
	BufferGfx_setDimensions(hSrcBuf, &dstDim);

	BufferGfx_getDimensions(hDstResize, &srcDim);
	BufferGfx_getDimensions(hDstResize, &oldDim);

	/*width and height */
	if(width >= dstDim.width) {
		width = dstDim.width;
		height = dstDim.height;
	}

	if(height >= dstDim.height) {
		width = dstDim.width;
		height = dstDim.height;
	}

	srcDim.width = width;
	srcDim.height = height;
	BufferGfx_setDimensions(hDstResize, &srcDim);
	//PRINTF("source width=%d,he=%d,ds w=%d,h=%d\n",dstDim.width ,dstDim.height, srcDim.width,srcDim.height );
	/*resize the buffer to width height*/
	if(Resize_config(hRsz, hSrcBuf, hDstResize) < 0)  {
		DEBUG(DL_ERROR, "[captureThrFxn] Failed to configure resize job\n");
		return -1;
	}

	/* Resize the captured frame to the resolution of the display frame */
	if(Resize_execute(hRsz, hSrcBuf, hDstResize) < 0) {
		DEBUG(DL_ERROR, "[captureThrFxn] Failed to execute resize job\n");
		return -1;
	}

	BufferGfx_setDimensions(hDstResize, &oldDim);
	return 0;
}


/*resize 420 data*/
int AppQuality422YUV(Resize_Handle hRsz, Buffer_Handle hSrcBuf, Buffer_Handle hDstResize)
{
	BufferGfx_Dimensions    srcDim, dstDim, oldDim;
	//ResizeParam *resize = &gResizeP;
	int width  = 0 , height = 0 ;
	width =	gResizeP.dst_w;
	height =	gResizeP.dst_h;
	BufferGfx_getDimensions(hSrcBuf, &srcDim);
	srcDim.width = H264Width();
	srcDim.height = H264Height();
	BufferGfx_setDimensions(hSrcBuf, &srcDim);

	BufferGfx_getDimensions(hDstResize, &dstDim);
	BufferGfx_getDimensions(hDstResize, &oldDim);

	/*width and height */
	if(width >= srcDim.width) {
		width = srcDim.width;
		height = srcDim.height;
	}

	if(height >= srcDim.height) {
		width = srcDim.width;
		height = srcDim.height;
	}

	dstDim.width = (width / 8) * 8;

	//if(width != dstDim.width) {
	//	PRINTF("++++++++%d++dst+%d+++++++++++++++++++\n", width, dstDim.width);
	//}

	dstDim.height = height;

	if(width == 1400 && height == 1050) {
		;//dstDim.height = 1080;
	}

	dstDim.x = gResizeP.x;
	dstDim.y = gResizeP.y;
	//printf("AppQuality422YUV x=%d,y=%d,width=%d,height=%d\n", dstDim.x, dstDim.y, dstDim.width, dstDim.height);
	BufferGfx_setDimensions(hDstResize, &dstDim);
#if 0

	/*resize the buffer to width height*/
	if(Resize_config(hRsz, hSrcBuf, hDstResize) < 0)  {
		DEBUG(DL_ERROR, "[captureThrFxn] Failed to configure resize job\n");
		return -1;
	}

	/* Resize the captured frame to the resolution of the display frame */
	if(Resize_execute(hRsz, hSrcBuf, hDstResize) < 0) {
		DEBUG(DL_ERROR, "[captureThrFxn] Failed to execute resize job\n");
		return -1;
	}

#else

	//fprintf(stderr, "src: x = %d, y = %d, width = %d, height = %d, length = %d\n",
	//        (int)srcDim.x, (int)srcDim.y, (int)srcDim.width, (int)srcDim.height, (int)srcDim.lineLength);
	//fprintf(stderr, "dst: x = %d, y = %d, width = %d, height = %d, length = %d\n\n",
	//        (int)dstDim.x, (int)dstDim.y, (int)dstDim.width, (int)dstDim.height, (int)dstDim.lineLength);
	if(gResizeP.black > 0) {
		PRINTF("gResizeP.black=%d,\n", gResizeP.black);
		__BlackFill_422(hDstResize, 1920, 1080);
		gResizeP.black--;
	}

	if(Resize_config_dstDim(hRsz, hSrcBuf, srcDim, hDstResize, dstDim) < 0)  {
		DEBUG(DL_ERROR, "[captureThrFxn] Failed to configure resize job\n");
		return -1;
	}

	/* Resize the captured frame to the resolution of the display frame */
	if(Resize_execute_dstDim(hRsz, hSrcBuf, srcDim, hDstResize, dstDim) < 0) {
		DEBUG(DL_ERROR, "[captureThrFxn] Failed to execute resize job\n");
		return -1;
	}

#endif

	BufferGfx_setDimensions(hDstResize, &oldDim);
	return 0;
}

int LockCurrentResolution(unsigned char *data, int len)
{
	int flag = 0;

	if(len) {
		DEBUG(DL_DEBUG, "[LockCurrentResolution] [%d]\n", data[0]);

		if(LOCK_IN == data[0] && gResizeP.flag == RESIZE_INVALID) {
			gResizeP.dst_h = capture_height;
			gResizeP.dst_w = capture_width;
			gResizeP.enc_w = capture_width;
			gResizeP.enc_h = capture_height;
			gResizeP.x = 0;
			gResizeP.y = 0;
			gResizeP.flag = RESIZE_INVALID;
			//gResizeP.re_enc=RECREATE_OK;
			DEBUG(DL_DEBUG, "[LockCurrentResolution] LOCK_IN\n");
			lock_status = data[0];
		}

		if(LOCK_OUT == data[0] && lock_status) {
			if(gResizeP.enc_w != capture_width || gResizeP.enc_h != capture_height) {
				flag = 1;
			}

			gResizeP.enc_w = capture_width;
			gResizeP.enc_h = capture_height;
			gResizeP.flag = RESIZE_INVALID;

			if(flag == 1) {
				gResizeP.re_enc = RECREATE_OK;
			}

			DEBUG(DL_DEBUG, "[LockCurrentResolution] LOCK_OUT\n");
			lock_status = data[0];
		}
	}
}

void unLockCurrentResolution(void)
{
	lock_status = LOCK_OUT;
}

int calcLockResolution(OutputResolution val)
{
	switch(val) {
		case LOCK_BEGIN:
			gResizeP.enc_w = capture_width;
			gResizeP.enc_h = capture_height;
			gResizeP.flag = RESIZE_INVALID;
			setOutputResolution(val);
			break;

		case LOCK_1920x1080P:
			gResizeP.enc_w = 1920;
			gResizeP.enc_h = 1080;
			gResizeP.flag = RESIZE_INVALID;
			setOutputResolution(val);
			break;

		case LOCK_1280x720P:
			gResizeP.enc_w = 1280;
			gResizeP.enc_h = 720;
			gResizeP.flag = RESIZE_INVALID;
			setOutputResolution(val);
			break;

		case LOCK_1400x1050_VGA:
			gResizeP.enc_w = 1400;
			gResizeP.enc_h = 1050;
			gResizeP.flag = RESIZE_INVALID;
			setOutputResolution(val);
			break;

		case LOCK_1366x768_VGA:
			gResizeP.enc_w = 1366;
			gResizeP.enc_h = 768;
			gResizeP.flag = RESIZE_INVALID;
			setOutputResolution(val);
			break;

		case LOCK_1280x1024_VGA:
			gResizeP.enc_w = 1280;
			gResizeP.enc_h = 1024;
			gResizeP.flag = RESIZE_INVALID;
			setOutputResolution(val);
			break;

		case LOCK_1280x768_VGA:
			gResizeP.enc_w = 1280;
			gResizeP.enc_h = 768;
			gResizeP.flag = RESIZE_INVALID;
			setOutputResolution(val);
			break;

		case LOCK_1024x768_VGA:
			gResizeP.enc_w = 1024;
			gResizeP.enc_h = 768;
			gResizeP.flag = RESIZE_INVALID;
			setOutputResolution(val);
			break;

		case LOCK_720x480_D1:
			gResizeP.enc_w = 704;  //video encode need 32*n
			gResizeP.enc_h = 480;
			gResizeP.flag = RESIZE_INVALID;
			setOutputResolution(val);
			break;

		case LOCK_352x288_D1:
			gResizeP.enc_w = 352;
			gResizeP.enc_h = 288;
			gResizeP.flag = RESIZE_INVALID;
			setOutputResolution(val);
			break;

		default:
			gResizeP.enc_w = capture_width;
			gResizeP.enc_h = capture_height;
			gResizeP.flag = RESIZE_INVALID;
			setOutputResolution(val);
			break;
	}

	return 0;
}




#endif
