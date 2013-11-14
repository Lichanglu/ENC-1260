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

/* Include codec specific header file for extended params */

#include "scale.h"

static SCALE_Handle 	scalerHandle = NULL;
static SCALE_DynamicParams     scaleparams;
static SCALE_InArgs	scaleInArgs;
static SCALE_OutArgs	scaleOutArgs;
static SCALE_Params DefaultScaleParam = {
	sizeof(SCALE_Params),
	800,   		//owidthd//输出宽
	600,   		//oheight//输出高1001075510010100
	1920, 			//iwdith//输入buf的linelength
	1088,     	 	//iheight//输入buf的高
	0,			//cropirow//输入图像在buf中的左边dim值的x，y
	0,			//cropicol//
	1024,		//cropiwidth//输入的实际有效像素宽高
	768,		//cropiheight//
	1920 * 1088, 	//cropvuoffset//输入buf的uv数据相对y数据的偏移值
	0,			//desktoprow//输出图像在输出buf中的坐标相当于dim值的x，y
	0,			//desktopcol//
	1920,		//desktopwidth//输出图像buf的linelength
	1088,		//desktopheight//输出图像buf的高
	1,			//samplemode//设置输入/出的数据采样率，=1 为4：2：0；=0 为4:2:2
	0,			//packetmode	////      设置数据封装格式，=0 为SEMI

};


int scale_create(Engine_Handle hEngine, Char *codecName, SCALE_Params *params)
{
	int ret;

	if(NULL == params) {
		ret = scalerHandle = SCALE_create(hEngine, codecName, &(DefaultScaleParam));
	} else {
		ret = scalerHandle = SCALE_create(hEngine, codecName, (params));
	}

	return ret;
}

int scale_setparam(int iwidth, int iheight, int cropirow, int cropicol, int cropiwidth, int cropiheight, int cropvuoffset, int owidth, int oheight, int desktoprow , int desktopcol, int desktopwidth, int desktopheight, int upsampleout , int sampleMode)
{
	int ret;
	scaleparams.size = sizeof(SCALE_DynamicParams);
	scaleparams.iwidth = iwidth;
	scaleparams.iheight = iheight;
	scaleparams.cropirow = cropirow;
	scaleparams.cropicol = cropicol;
	scaleparams.cropiwidth = cropiwidth;
	scaleparams.cropiheight = cropiheight;
	scaleparams.cropvuoffset = cropvuoffset;
	scaleparams.owidth = owidth;
	scaleparams.oheight = oheight;
	scaleparams.desktoprow = desktoprow; //默认都是从第一行，无偏移显示
	scaleparams.desktopcol = desktopcol;
	scaleparams.desktopwidth = desktopwidth; //依赖原始输出显示的宽高 ，解码器DEC1000使用
	scaleparams.desktopheight = desktopheight;
	scaleparams.upsampleout = upsampleout;
	scaleparams.sampleMode	= sampleMode;
	ret = SCALE_control(scalerHandle, ISCALE_SETPARAMS, &scaleparams);
	return ret;

}

int scale_process(Buffer_Handle hInBuf, Buffer_Handle hOutBuf, SCALE_Params *params)
{
	int ret;
	scaleInArgs.inBufSize  = Buffer_getSize(hInBuf);
	scaleInArgs.outBufSize = Buffer_getSize(hOutBuf);
	ret = SCALE_process(scalerHandle, Buffer_getUserPtr(hInBuf), Buffer_getUserPtr(hOutBuf), &scaleInArgs, &scaleOutArgs);
	return ret;
}
int scale_delete()
{
	int ret;
	SCALE_delete(scalerHandle);
	return ret;

}
