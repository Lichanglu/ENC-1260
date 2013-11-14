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
	800,   		//owidthd//�����
	600,   		//oheight//�����1001075510010100
	1920, 			//iwdith//����buf��linelength
	1088,     	 	//iheight//����buf�ĸ�
	0,			//cropirow//����ͼ����buf�е����dimֵ��x��y
	0,			//cropicol//
	1024,		//cropiwidth//�����ʵ����Ч���ؿ��
	768,		//cropiheight//
	1920 * 1088, 	//cropvuoffset//����buf��uv�������y���ݵ�ƫ��ֵ
	0,			//desktoprow//���ͼ�������buf�е������൱��dimֵ��x��y
	0,			//desktopcol//
	1920,		//desktopwidth//���ͼ��buf��linelength
	1088,		//desktopheight//���ͼ��buf�ĸ�
	1,			//samplemode//��������/�������ݲ����ʣ�=1 Ϊ4��2��0��=0 Ϊ4:2:2
	0,			//packetmode	////      �������ݷ�װ��ʽ��=0 ΪSEMI

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
	scaleparams.desktoprow = desktoprow; //Ĭ�϶��Ǵӵ�һ�У���ƫ����ʾ
	scaleparams.desktopcol = desktopcol;
	scaleparams.desktopwidth = desktopwidth; //����ԭʼ�����ʾ�Ŀ�� ��������DEC1000ʹ��
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
