/*
****************************************************
Copyright (C), 1988-1999, Reach Tech. Co., Ltd.

File name:     	sysparam.c

Description:    参数保存flash、读取flash
				关于参数表的方法实现过程

Date:     		2010-11-16

Author:	  		yangshh

version:  		V1.0

*****************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <ctype.h>
#include <netinet/in_systm.h>
#include <netinet/ip_icmp.h>
#include <linux/videodev2.h>
#include <ti/sdo/dmai/Sound.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

#include "common.h"
#include "sysparam.h"
#include "tcpcom.h"
#include "capture.h"

#include "log_common.h"

#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct __SetMutexParam__ {
	pthread_mutex_t video_m;
	pthread_mutex_t audio_m;
	pthread_mutex_t sys_m;

} SetParamM;

/*音/视频参数表结构体*/
extern SavePTable 	gSysParaT;
/*音/视频编码库参数设置表*/
extern VAENC_params gVAenc_p;
extern ResizeParam gLowRate;
extern int g_FPGA_update;
extern DSPCliParam gDSPCliPara[PORT_NUM];
extern char IPdotdec[6][20];


/*线程互斥变量定义*/
PLock gSetP_m;
/*参数设置互斥变量*/
SetParamM 	gParam_M;
/*I frames 间隔*/
//unsigned int g_IframeInterval[MAX_VAL] 	= {300, 300};

const unsigned int FPGA_HV_TALBE[MAX_MODE_VALUE][4] = {
	//ad9880-H-A, ad9880-V-A,ad9880-H-D, ad9880-V-D
	{124, 28, 48, 27}, //640*480*60-----0
	{148, 24, 128, 29}, //640*480*72-----1
	{164, 12, 120, 13}, //640*480*75-----2
	{116, 20, 80, 21}, //640*480*85-----3			//{640,480,0,0,116,20,80,21,81,26},//640*480*85-----3
	{196, 19, 88, 19}, //800*600*60-----4
	{164, 21, 64, 21}, //800*600*72-----5
	{220, 17, 160, 16}, //800*600*75-----6
	{196, 22, 152, 22}, //800*600*85-----7
	{274, 27, 160, 28}, //1024*768*60-----8
	{258, 27, 144, 28}, //1024*768*70-----9
	{252, 24, 206, 24}, //1024*768*75-----10
	{284, 31, 208, 32}, //1024*768*85-----11
	{304, 19, 192, 19}, //1280*768*60-----12
	{318, 20, 240, 19}, //1280*800*60-----13
	{404, 31, 312, 31}, //1280*960*60-----14
	{364, 23, 232, 23}, //1440*900*60-----15
	{364, 29, 230, 31}, //1400*1050*60-----16
	{340, 33, 246, 34}, //1280*1024*60-----17
	{372, 34, 248, 33}, //1280*1024*75-----18
	{476, 40, 380, 41}, //1600*1200*60-----19
	{304, 17, 120, 19}, //1280X720X60VGA-----20		//	{1280,720,0,0,276,17,220,19,260,20},//1280X720X60VGA-----20
	{276, 17, 260, 25}, //1280X720X50-----21
	{276, 17, 260, 25}, //1280X720X60-----22		//{1280,720,0,0,276,17,260,25,260,20},//1280X720X60-----22
	{212, 13, 192, 22}, //1920X1080IX50-----23
	{212, 13, 192, 20}, //1920X1080IX60-----24
	{166, 29, 192, 41}, //1920*1Q2080*25P-----25
	{198, 34, 170, 39}, //1920*1080*30P-----26
	{216, 34, 192, 41}, //1920*1080*50P-----27
	{216, 34, 192, 41}, //1920*1080*60P-----28
	{110, 12, 110, 11}, //480i 60-----29
	{110, 16, 110, 16}, //576i 50----30		//{720,576,0,0,110,16,270,21,7,1},//576i 50----30
	{110, 12, 110, 11}, //720X480X60P----31
	{110, 16, 110, 16}, //720X576X50P----32
	{346, 19, 216, 19}, //1366*768*60-----33

};
/*线程互斥变量初始化*/
int initMutexPthread(void)
{
	pthread_mutex_init(&gSetP_m.sys_m, NULL);
	pthread_mutex_init(&gSetP_m.save_flush_m, NULL);
	pthread_mutex_init(&gSetP_m.send_m, NULL);
	pthread_mutex_init(&gSetP_m.send_audio_m, NULL);
	pthread_mutex_init(&gSetP_m.audio_video_m, NULL);
	pthread_mutex_init(&gSetP_m.net_send_m, NULL);
	pthread_mutex_init(&gSetP_m.save_sys_m, NULL);
#ifdef CL4000_DVI_SDI
	pthread_mutex_init(&gSetP_m.rtp_send_m, NULL);
#else
	pthread_mutex_init(&gSetP_m.save_dtype_m, NULL);
#endif
	return 0;
}

/*参数设置互斥变量初始化*/
int initSetParam(void)
{
	pthread_mutex_init(&gParam_M.audio_m, NULL);
	pthread_mutex_init(&gParam_M.video_m, NULL);
	pthread_mutex_init(&gParam_M.sys_m, NULL);
	return 0;
}

int DestroySetParam(void)
{
	pthread_mutex_destroy(&gParam_M.audio_m);
	pthread_mutex_destroy(&gParam_M.video_m);
	pthread_mutex_destroy(&gParam_M.sys_m);
	return 0;
}

/*线程互斥变量初始化*/
int DestroyMutexPthread(void)
{
	pthread_mutex_destroy(&gSetP_m.sys_m);
	pthread_mutex_destroy(&gSetP_m.save_flush_m);
	pthread_mutex_destroy(&gSetP_m.send_m);
	pthread_mutex_destroy(&gSetP_m.send_audio_m);
	pthread_mutex_destroy(&gSetP_m.audio_video_m);
	pthread_mutex_destroy(&gSetP_m.net_send_m);
	pthread_mutex_destroy(&gSetP_m.save_sys_m);
#ifdef CL4000_DVI_SDI
	pthread_mutex_destroy(&gSetP_m.rtp_send_m);
#else
	pthread_mutex_destroy(&gSetP_m.save_dtype_m);
#endif
	return 0;
}

/*发送数据加锁*/
int SendPthreadLock()
{
	pthread_mutex_lock(&gSetP_m.send_m);
	return 0;
}

/*发送数据解锁*/
int SendPthreadunLock()
{
	pthread_mutex_unlock(&gSetP_m.send_m);
	return 0;
}

void ParsePackageLock()
{
	pthread_mutex_lock(&gSetP_m.send_audio_m);
}

void ParsePackageunLock()
{
	pthread_mutex_unlock(&gSetP_m.send_audio_m);
}

/*
	建立索引和采样率之间的关系
	///0---8KHz
	///2---44.1KHz default----44100Hz
	///1---32KHz
	///3---48KHz
	///other-----96KHz
*/
int ChangeSampleIndex(int index)
{
	int ret = 44100;

	switch(index) {
		case 0:   //8KHz
		case 1:  //32KHz
		case 2:
			ret = 44100;
			break;

		case 3:
			ret = 48000;
			break;

		default:
			ret = 48000;
			break;
	}

	return (ret);
}


/*
##系统参数表初始化
*/
static void InitStandardParam(SYSPARAMS *std)
{
	std->dwAddr = inet_addr("192.168.7.92");
	std->dwGateWay =  inet_addr("192.168.7.254");
	std->dwNetMark = inet_addr("255.255.255.0");

	std->szMacAddr[0] = 0x00;
	std->szMacAddr[1] = 0x09;
	std->szMacAddr[2] = 0x30;
	std->szMacAddr[3] = 0x28;
	std->szMacAddr[4] = 0x12;
	std->szMacAddr[5] = 0x22;
	strcpy(std->strName, "DSS-ENC-MOD");
	//strcpy(std->strVer, );
	std->unChannel = 1;
	std->bType = ENCODE_TYPE;  //0 -------VGABOX  3-------200 4-------110 5-------120 6--------1200  8 --ENC-1100
	bzero(std->nTemp, sizeof(std->nTemp));

}

/*初始化视频参数表*/
static void InitVideoParam(VideoParam *video)
{
	bzero(video, sizeof(VideoParam));
	video->nDataCodec = 0x34363248;	//"H264"
	video->nWidth = 1920;			//video width
	video->nHight = 1080;			//video height
	video->nQuality = 45;				//quality (5---90)
	video->sCbr = 1;					// 0---quality  1---bitrate
	video->nFrameRate = 30;			//current framerate
	video->sBitrate = 4096;				//bitrate (128k---4096k)
	video->nColors = 24;			//24

}

/*初始化音频参数表*/
static void InitAudioParam(AudioParam *audio)
{
	//initial
	bzero(audio, sizeof(AudioParam));
	audio->Codec = ADTS;		//"ADTS"
	///0---8KHz
	///2---44.1KHz default----44100Hz
	///1---32KHz
	///3---48KHz
	///other-----96KHz
	audio->SampleRate = 3;
	audio->Channel = 2;					//audio channel (default 2)
	audio->SampleBit = 16;				//sample bit(default 16)
	audio->BitRate = 64000;				//bitrate (default 64000)
	audio->InputMode = LINE_INPUT;		//input mode (0 ---line 1----mic)
	audio->RVolume = 23;
	audio->LVolume = 23;
}

/*初始化参数表结构体*/
void InitSysParams()
{
	int dsp;

	/*系统参数表*/
	InitStandardParam(&gSysParaT.sysPara);

	for(dsp = 0; dsp < PORT_NUM ; dsp++) {

		/*视频参数表*/
		InitVideoParam(&gSysParaT.videoPara[dsp]);

		/*音频参数*/
		InitAudioParam(&gSysParaT.audioPara[dsp]);
	}

	return ;
}

/*初始化视频编码参数*/
int InitVideoEncParams(VideoParam *vparam)
{
	/*set video encode param availability*/
	gVAenc_p.venc.cbr = vparam->sCbr;
	gVAenc_p.venc.req_I = 0;
	gVAenc_p.venc.framerate = vparam->nFrameRate;
	gVAenc_p.venc.quality = vparam->nQuality;
	gVAenc_p.venc.bitrate = vparam->sBitrate;
	gVAenc_p.venc.width = vparam->nWidth;
	gVAenc_p.venc.height = vparam->nHight;
	return 0;
}


/*初始化低码率视频编码参数*/
int InitLowRateParams(VideoParam *vparam)
{
	/*低码流参数*/
	gVAenc_p.lowvenc.cbr = 1;		//定质量
	gVAenc_p.lowvenc.framerate = vparam->nFrameRate;
	gVAenc_p.lowvenc.quality = 90;
	gVAenc_p.lowvenc.height = vparam->nHight;
	gVAenc_p.lowvenc.width  = vparam->nWidth;
	gVAenc_p.lowvenc.bitrate = vparam->sBitrate;//1024000;	//  1M
	gVAenc_p.lowvenc.req_I = 0;

	return 0;
}
/*初始化音频编码参数*/
int InitAudioEncParams(AudioParam *aparam)
{
	/*set audio encode param availability*/
	gVAenc_p.aenc.bitrate = aparam->BitRate;

	if(aparam->InputMode == 1) {
		gVAenc_p.aenc.inputmode = Sound_Input_MIC;
	} else {
		gVAenc_p.aenc.inputmode = Sound_Input_LINE;
	}

	gVAenc_p.aenc.lvolume = aparam->LVolume;
	gVAenc_p.aenc.rvolume = aparam->RVolume;
	gVAenc_p.aenc.samplebit = aparam->SampleBit;
	gVAenc_p.aenc.samplerate = ChangeSampleIndex(aparam->SampleRate);

	DEBUG(DL_DEBUG, "aparam->BitRate = %d \n", aparam->BitRate);
	return 0;
}

/*设置编码的高*/
int SetH264Width(int width)
{

	if(width < 352) {
		width = 352;
	}

	gSysParaT.videoPara[PORT_ONE].nWidth = width;
	return (gSysParaT.videoPara[PORT_ONE].nWidth);
}


/*设置编码的高*/
int SetH264Height(int height)
{
	if(height < 288) {
		height = 288;
	}

	gSysParaT.videoPara[PORT_ONE].nHight = height;
	return (gSysParaT.videoPara[PORT_ONE].nHight);
}

/*获取编码宽*/
unsigned int H264Width(void)
{
	return (gSysParaT.videoPara[PORT_ONE].nWidth);
}

/*获取编码高*/
unsigned int H264Height(void)
{
	return (gSysParaT.videoPara[PORT_ONE].nHight);
}

/*get control frame rate value*/
unsigned int GetCtrlFrame()
{
	//PRINTF("%d\n", (gSysParaT.videoPara[PORT_ONE].nFrameRate));
	return(gSysParaT.videoPara[PORT_ONE].nFrameRate);
}

/*get control frame low rate value*/
unsigned int GetCtrlLowRateFrame()
{
	return(gSysParaT.videoPara[PORT_TWO].nFrameRate);
}

/*
## 将MAC地址的字符串转换成字符数组中
##
## 如:  src = "00:11:22:33:44:55" --->
##		dst[0] = 0x00 dst[1] = 0x11 dst[2] = 0x22;
##
*/
int SplitMacAddr(char *src, unsigned char *dst, int num)
{
	char *p;
	char *q = src;
	int val = 0  , i = 0;

	for(i = 0 ; i < num ; i++) {
		p = strstr(q, ":");

		if(!p) {
			val = strtol(q, 0, 16);
			dst[i]  = val ;

			if(i == num - 1) {
				continue;
			} else {
				return -1;
			}
		}

		*(p++) = '\0';
		val = strtol(q, 0, 16);
		dst[i]  = val ;
		q = p;
	}

	return 0;
}

/*
##
## 例如:  src[0] = 0x00,src[1] = 0x11 src[2] = 0x22
##      src[3[ = 0x33,src[4] = 0x44 src[5] = 0x55
##		dst = "00:11:22:33:44:55"
##
*/
static int JoinMacAddr(char *dst, unsigned char *src, int num)
{
	sprintf(dst, "%02x:%02x:%02x:%02x:%02x:%02x", src[0],
	        src[1], src[2], src[3], src[4], src[5]);
	DEBUG(DL_DEBUG, "mac addr  = %s \n", dst);

	return 0;
}

/*检测IP地址正确性 */
static int CheckIPNetmask(int ipaddr, int netmask, int gw)
{
	int mask, ip, gateip;
	mask = netmask;
	mask = htonl(mask);
	ip = ipaddr;
	ip = htonl(ip);
	gateip = gw;
	gateip = htonl(gateip);

	if((((ip & 0xFF000000) >> 24) > 223) || ((((ip & 0xFF000000) >> 24 == 127)))) {
		return 0;
	}

	if(((ip & 0xFF000000) >> 24) < 1) {
		return 0;
	}

	if((((gateip & 0xFF000000) >> 24) > 223) || (((gateip & 0xFF000000) >> 24 == 127))) {
		return 0;
	}

	if(((gateip & 0xFF000000) >> 24) < 1) {
		return 0;
	}

	if((ip & mask) == 0) {
		return 0;
	}

	if((ip & (~mask)) == 0) {
		return 0;
	}

	if((~(ip | mask)) == 0) {
		return 0;
	}

	while(mask != 0) {
		if(mask > 0) {
			return 0;
		}

		mask <<= 1;
	}

	return 1;
}

/*Force I frame */
int ForceIframe(int start)
{
	if(start == START) {
		gVAenc_p.lowvenc.req_I = AVIIF_KEYFRAME;
		DEBUG(DL_DEBUG, "Force I frame low rate!!\n");
	}  else {
		gVAenc_p.venc.req_I = AVIIF_KEYFRAME;
		DEBUG(DL_DEBUG, "Force I frame high rate!!\n");
	}

	return 0;
}

/*get audio param*/
void GetAudioParam(int socket, int dsp, unsigned char *data, int len)
{
	unsigned char temp[200];
	int length = 0, ret;

	/*send audio param*/
	length = HEAD_LEN + sizeof(gSysParaT.audioPara[dsp]);
	PackHeaderMSG(temp, MSG_GET_AUDIOPARAM, length);

	memcpy(temp + HEAD_LEN, &gSysParaT.audioPara[dsp], sizeof(gSysParaT.audioPara[dsp]));
	ret = WriteData(socket, temp, length);

	if(ret < 0) {
		DEBUG(DL_ERROR, "Send GetAudioParam()return:%d socket:%d ERROR:%d\n", ret, socket, errno);
	}

	DEBUG(DL_DEBUG, "GetAudioParam() succeed:%d\n", ret);
	return ;
}


/*get video param*/
void GetVideoParam(int socket, int dsp, unsigned char *data, int len)
{
	unsigned char temp[200];
	int length = 0, ret;

	length = HEAD_LEN + sizeof(gSysParaT.videoPara[PORT_ONE]);
	PackHeaderMSG(temp, MSG_GET_VIDEOPARAM, length);

	memcpy(temp + HEAD_LEN, &gSysParaT.videoPara[PORT_ONE], sizeof(gSysParaT.videoPara[PORT_ONE]));
	ret = WriteData(socket, temp, length);

	if(ret < 0) {
		DEBUG(DL_ERROR, "Send GetAudioParam()return:%d socket:%d ERROR:%d\n", ret, socket, errno);
	}

	DEBUG(DL_DEBUG, "GetVideoParam() succeed:%d framerate:%d\n", ret, gSysParaT.videoPara[PORT_ONE].nFrameRate);

	return ;
}

//return 1 表示需要重建
//return 2 表示需要设置动态参数
/*beyong video param*/
int BeyondVideoParam(VideoParam *pNew, VideoParam *pOld, int *ret_value)
{
	int width_change = 0;

	if(pNew->nColors != pOld->nColors) {
		pOld->nColors = pNew->nColors;
		*ret_value = 1;
		DEBUG(DL_DEBUG, "videoParam nColors change :%d\n", pNew->nColors);
	}

	/*	if(pNew->nDataCodec != pOld->nDataCodec) {
			pOld->nDataCodec = pNew->nDataCodec;
			*ret_value=1;
			printf("videoParam nDataCodec change :%d\n",pNew->nDataCodec);
		}*/
	if((pNew->nFrameRate != pOld->nFrameRate) && (0 !=  pNew->nFrameRate)) {
		pOld->nFrameRate = pNew->nFrameRate;
		*ret_value = 3;
		DEBUG(DL_DEBUG, "videoParam nFrameRate change :%d\n", pNew->nFrameRate);
	}

	/*	if(pNew->nHight != pOld->nHight) {
			pOld->nHight = pNew->nHight;
			*ret_value=1;
			DEBUG(DL_DEBUG,"videoParam nHight change :%d\n",pNew->nHight);
		}*/
	if(pNew->nQuality != pOld->nQuality) {
		pOld->nQuality = pNew->nQuality;
		*ret_value = 1;
		DEBUG(DL_DEBUG, "videoParam nQuality change :%d\n", pNew->nQuality);
	}

	/*
		if(pNew->nWidth != pOld->nWidth) {
			pOld->nWidth = pNew->nWidth;
			*ret_value=1;
			DEBUG(DL_DEBUG,"videoParam nWidth change :%d\n",pNew->nWidth);
			width_change=1;
		}
	*/

	//如果改变，需要重建
	if(pNew->sCbr != pOld->sCbr) {
		pOld->sCbr = pNew->sCbr;
		*ret_value = 1;
		DEBUG(DL_DEBUG, "videoParam sCbr change :%d\n", pNew->sCbr);
	}

	if(pNew->sBitrate != pOld->sBitrate) {
		pOld->sBitrate = pNew->sBitrate;

		if(*ret_value == 0) {
			PRINTF("DEBUG,There is only bitrate changed\n");
			*ret_value = 2;
		}

		DEBUG(DL_DEBUG, "videoParam sBitrate change :%d\n", pNew->sBitrate);
	}

	return (width_change);
}

/*beyond audio param*/
int BeyondAudioParam(AudioParam *pNew, AudioParam *pOld)
{

	int ret = 0;

	if(pNew->BitRate != pOld->BitRate) {
		pOld->BitRate = pNew->BitRate;
		ret = 1;
		DEBUG(DL_DEBUG, "AudioParam BitRate change:%d\n", pNew->BitRate);
	}

	if(pNew->BitRate > 128000 || pNew->BitRate <= 0) {
		pOld->BitRate = pNew->BitRate = 128000;
	}

	if(pNew->BitRate != pOld->BitRate) {
		pOld->BitRate = pNew->BitRate ;
		ret = 1;
		DEBUG(DL_DEBUG, "AudioParam BitRate change:%d\n", pNew->BitRate);
	}

	if(pNew->Channel != pOld->Channel || pNew->Channel != 2) {
		pOld->Channel = 2;
		ret = 1;
		DEBUG(DL_DEBUG, "AudioParam Channel change:%d\n", pNew->Channel);
	}

	if(pNew->SampleBit != pOld->SampleBit || pNew->SampleBit != 16) {
		pOld->SampleBit = 16 ;
		ret = 1;
		DEBUG(DL_DEBUG, "AudioParam SampleBit change:%d\n", pNew->SampleBit);
	}

	if(pNew->SampleRate != pOld->SampleRate) {
		pOld->SampleRate = pNew->SampleRate ;
		
		ret = 1;
		DEBUG(DL_DEBUG, "AudioParam SampleRate change:%d\n", pNew->SampleRate);
	}

	//only have 48kHZ
	if(pOld->SampleRate != 3) {
		pOld->SampleRate = pNew->SampleRate = 3;
	}

	if(pNew->LVolume != pOld->LVolume) {
		pOld->LVolume = pNew->LVolume ;
		ret = 1;
		DEBUG(DL_DEBUG, "AudioParam LVolume change:%d\n", pNew->LVolume);
	}

	if(pNew->RVolume != pOld->RVolume) {
		pOld->RVolume = pNew->RVolume ;
		ret = 1;
		DEBUG(DL_DEBUG, "AudioParam RVolume change:%d\n", pNew->RVolume);
	}

	if(pNew->InputMode != pOld->InputMode) {
		pOld->InputMode = pNew->InputMode ;
		ret = 1;
		DEBUG(DL_DEBUG, "AudioParam InputMode change:%d\n", pNew->InputMode);
	}

	return (ret);
}

/*set video encode params value*/
void SetEncodeVideoParams(VideoParam *vparam, int dsp, int flag)
{
	if(flag == 2) {
		gVAenc_p.venc.bitrate = vparam->sBitrate;
		PRINTF("DEBUG,the video sbitrate is change %d,\n", gVAenc_p.venc.bitrate);
		return ;
	}

	/*set video encode param availability*/
	gVAenc_p.venc.cbr = vparam->sCbr;
	gVAenc_p.venc.framerate = vparam->nFrameRate;
	gVAenc_p.venc.quality = vparam->nQuality;
	gVAenc_p.venc.bitrate = vparam->sBitrate;
	gVAenc_p.venc.uflag = H264_ENC_PARAM_FLAG;
	return ;
}

/*set video param table*/
int SetVideoParam(int dsp, unsigned char *data, int len)
{
	unsigned char temp[200];
	VideoParam *Newp, *Oldp;
	int ret = 0, param_c = 0;

	pthread_mutex_lock(&gParam_M.video_m);
	memcpy(temp, data, len);
	Oldp = &gSysParaT.videoPara[PORT_ONE];
	Newp = (VideoParam *)&temp[0];
	//DEBUG(DL_FLOW, "set video param,param_c = %d mode : [%d], old nFrameRate : [%d], new nFrameRate : [%d] old bitrate : [%d],new bitrate:[%d]\n\n",
	//      param_c, gblGetMode(), Oldp->nFrameRate, Newp->nFrameRate, Oldp->sBitrate, Newp->sBitrate);

	ret = BeyondVideoParam(Newp, Oldp, &param_c);
#if 0

	if(gblGetMode() == VIDEO_720P60_PIXEL) {
		if(60 <= Oldp->nFrameRate) {
			Oldp->nFrameRate = 60;
		}
	} else if(gblGetMode() == VIDEO_720P50_PIXEL) {

		if(50 <= Oldp->nFrameRate) {
			Oldp->nFrameRate = 50;
		}
	} else {

		if(30 <=  Oldp->nFrameRate) {
			Oldp->nFrameRate = 30;
		}

	}
#else 
	if(30 <=  Oldp->nFrameRate) {
			Oldp->nFrameRate = 30;
		}

#endif


	//ret=BeyondVideoParam(Newp,Oldp,&param_c);
	//DEBUG(DL_FLOW, "set video param,param_c = %d\n\n", param_c);

	if(param_c) {
		if(param_c == 1 || param_c == 3) {
			//if just change frame rate ,not need recreate the video encode. add by zm
			if(param_c == 1)
				SetEncodeVideoParams(Oldp, dsp, 1);
			
			param_c = 1;
			/*send broadcast message*/
			SendBroadCastMsg(PORT_ONE, MSG_GET_VIDEOPARAM, (unsigned char *)Oldp, sizeof(VideoParam));
		} else if(param_c == 2) {
			SetEncodeVideoParams(Oldp, dsp, 2);
			ForceIframe(0);//


			//it meas need set tc
			app_mult_set_all_tc();
			stream_rtsp_set_all_tc();
			SendBroadCastMsg(PORT_ONE, MSG_GET_VIDEOPARAM, (unsigned char *)Oldp, sizeof(VideoParam));
		}
	}

	pthread_mutex_unlock(&gParam_M.video_m);
	return (param_c);
}

/*set audio encode params value*/
void SetEncodeAudioParams(AudioParam *aparam, int dsp)
{
	/*set audio encode param availability*/

	gVAenc_p.aenc.bitrate = aparam->BitRate;

	if(aparam->InputMode == 1) {
		gVAenc_p.aenc.inputmode = Sound_Input_MIC;
	} else {
		gVAenc_p.aenc.inputmode = Sound_Input_LINE;
	}

	gVAenc_p.aenc.lvolume = aparam->LVolume;
	gVAenc_p.aenc.rvolume = aparam->RVolume;
	gVAenc_p.aenc.samplebit = aparam->SampleBit;
	gVAenc_p.aenc.samplerate = ChangeSampleIndex(aparam->SampleRate);
	//gVAenc_p.aenc.samplerate= aparam->SampleRate;
	gVAenc_p.aenc.uflag = H264_ENC_PARAM_FLAG;

	return ;
}

/*set audio param table*/
int SetAudioParam(int dsp, unsigned char *data, int len)
{
	unsigned char temp[200];
	AudioParam *Newp, *Oldp;
	int ret = 0;
	int change = 0;
	pthread_mutex_lock(&gParam_M.audio_m);
	memcpy(temp, data, len);
	Oldp = &gSysParaT.audioPara[PORT_ONE];
	Newp = (AudioParam *)&temp[0];
	DEBUG(DL_FLOW, "<><><><><> set audio samplerate = %d, bitrate = %d InputMode : [%d] !!!!\n", Newp->SampleRate, Newp->BitRate, Newp->InputMode);
	PRINTF("old AudioParam LVolume  :%d,r=%d\n", Oldp->LVolume, Oldp->RVolume);
	PRINTF("AudioParam LVolume change:%d,r=%d\n", Newp->LVolume, Newp->RVolume);


	if((Newp->LVolume != Oldp->LVolume) || (Newp->RVolume != Oldp->RVolume)) {
		Oldp->LVolume = Newp->LVolume ;
		Oldp->RVolume = Newp->RVolume ;
		gVAenc_p.aenc.lvolume = Newp->LVolume;
		gVAenc_p.aenc.rvolume = Newp->RVolume;
		ChangeVolume(gVAenc_p);
		change = 1;
		//	SendBroadCastMsg(PORT_ONE, MSG_GET_AUDIOPARAM, (unsigned char *)Newp, sizeof(AudioParam));
		PRINTF("AudioParam LVolume change:%d,r=%d\n", Newp->LVolume, Newp->RVolume);
	}

	ret = BeyondAudioParam(Newp, Oldp);

	if(ret) {
		SetEncodeAudioParams(Newp, dsp);
	}

	if(ret || change == 1) {
		/*send broadcast message*/
		SendBroadCastMsg(PORT_ONE, MSG_GET_AUDIOPARAM, (unsigned char *)Newp, sizeof(AudioParam));
		ret  = 1;
	}

	pthread_mutex_unlock(&gParam_M.audio_m);
	return ret;
}

/*SetSysParams*/
int SetSysParams(SYSPARAMS *pold, SYSPARAMS *pnew)
{
	int  chl = 0;
	int status = 0, flag = 0;
	int ret  = 0;

	ret = CheckIPNetmask(pnew->dwAddr, pnew->dwNetMark, pnew->dwGateWay);

	if(ret == 0) {
		DEBUG(DL_ERROR, "Set IP addr error!!!!\n");
		return -1;
	}

	pthread_mutex_lock(&gParam_M.sys_m);

#if 0

	for(i = 0; i < 8; i++) {
		if(pold->szMacAddr[i] == 0) {
			count++;
		}
	}

	if(count != 8) {
		for(i = 0; i < 8; i++) {
			if(pold->szMacAddr[i] != pnew->szMacAddr[i]) {
				pold->szMacAddr[i] = pnew->szMacAddr[i];
				status = 1;
			}
		}
	}

	if(status) {
		DEBUG(DL_DEBUG, "The User Has Changed The MAC Addr!\n");
	}

	status = 0;
#endif

	if(pold->dwAddr != 0 && pold->dwAddr != pnew->dwAddr)	{
		pold->dwAddr = pnew->dwAddr;
		DEBUG(DL_DEBUG, "The User Has Changed The IP Addr!\n");
		status = 1;
	}

	if(pold->dwGateWay != 0 && pold->dwGateWay != pnew->dwGateWay)	{
		pold->dwGateWay = pnew->dwGateWay;
		DEBUG(DL_DEBUG, "The User Has Changed The GateWay!\n");
		flag = 1;
	}

	if(pold->dwNetMark != 0 && pold->dwNetMark != pnew->dwNetMark)	{
		pold->dwNetMark = pnew->dwNetMark;
		DEBUG(DL_DEBUG, "The User Has Changed The NetMark!\n");
		status = 1;
	}


	//dhcp
	if(pold->nTemp[0] != pnew->nTemp[0])	{
		pold->nTemp[0]  = pnew->nTemp[0] ;
		DEBUG(DL_DEBUG, "The dhcp flag Has Changed  \n");
		status = 1;
	}


	if(strlen(pold->strName) != 0) {
		strcpy(pold->strName, pnew->strName);
	}

	if(strlen(pold->strVer) != 0) {
		strcpy(pold->strVer, pnew->strVer);
	}

	if(pold->unChannel != pnew->unChannel)	{
		DEBUG(DL_DEBUG, "The Channel numbers has changed !\n");
		pold->unChannel = pnew->unChannel;
		pold->unChannel = 1;
		chl = 1;
	}

#if (defined(DSS_ENC_1200) || defined(DSS_ENC_1260))
	pold->bType = 6 ;    //ENC-1200
#else
	pold->bType = 8 ;	 //ENC-1100
#endif

	/*	if(pold->sTemp != pnew->sTemp)
		{
			pold->sTemp = pnew->sTemp;
			DEBUG(DL_DEBUG,"The User Has Changed The sTemp!\n");
		}
	*/
	if(chl || status || flag) {
		SendBroadCastMsg(PORT_ONE, MSG_SYSPARAMS, (unsigned char *)pnew, sizeof(SYSPARAMS));
	}

	pthread_mutex_unlock(&gParam_M.sys_m);

	if(status || flag) {
		return 1;
	} else {
		return 0;
	}
}

//*********************************************************************
//设置和获取静音参数
//*********************************************************************
int SetMuteParams(unsigned char *data, int len)
{
	unsigned char status;
	unsigned char temp[20];
	unsigned char mutestatus = 0;

	if(len) {
		status = data[0];

		if(status) {
			gblSetMute(START);
		} else {
			gblSetMute(STOP);
		}
	}

	mutestatus = gblGetMuteStatus();
	memcpy(temp, &mutestatus, sizeof(mutestatus));
	//LowRateBroadCastMsg(PORT_ONE,MSG_MUTE,temp,sizeof(mutestatus));
	SendBroadCastMsg(PORT_ONE, MSG_MUTE, temp, sizeof(mutestatus));
	return 0;
}

/*检查参数合法性*/
int CheckLowRateParam(const LOW_BITRATE *pp)
{
	if(pp->nHeight > LOWRATE_MAX_HEIGHT || pp->nWidth > LOWRATE_MAX_WIDTH
	   || pp->nHeight < LOWRATE_MIN_HEIGHT || pp->nWidth < LOWRATE_MIN_WIDTH) {
		DEBUG(DL_ERROR, "CheckLowRateParam() ERROR width = %d height = %d\n", pp->nWidth, pp->nHeight);
		return 0;
	}

	if(pp->nBitrate < 128 || pp->nBitrate > 4096) {
		DEBUG(DL_ERROR, "CheckLowRateParam() ERROR low rate = %d \n", pp->nBitrate);
		return 0;
	}

	/*
	    if(pp->nFrame < 1 || pp->nFrame > 60) {
	        DEBUG(DL_ERROR,"CheckLowRateParam() ERROR frames = %d \n",pp->nFrame);
	        return 0;
	    }  */
	return 1;
}

/*比较参数是否改变*/
int BeyondLowRateParam(const LOW_BITRATE *src, ResizeParam *dst)
{
	if(src->nHeight == dst->dst_h &&
	   src->nWidth == dst->dst_w &&
	   src->nBitrate == dst->nBitrate &&
	   src->nFrame == dst->nFrame) {
		return 0;
	} else if(src->nHeight == dst->dst_h && src->nFrame == dst->nFrame &&
	          src->nWidth == dst->dst_w && src->nBitrate != dst->nBitrate) {
		PRINTF("DEBUG,the low video only change the bitrate,new=%d,old=%d\n", src->nBitrate, dst->nBitrate);
		dst->nBitrate = src->nBitrate;
		return 2;
	} else {
		dst->dst_h = src->nHeight;
		dst->dst_w = src->nWidth;
		dst->nBitrate = src->nBitrate;
		dst->nFrame = src->nFrame;

		if(src->nWidth - (H264Width() / 2) < 8) {
			dst->dst_w += 10;
		}

		return 1;
	}
}


/*返回低码流*/
static int RetLowRateParam(unsigned char *data, int *len)
{
	LOW_BITRATE param;
	int     length = 0;
	memset(&param, 0, sizeof(LOW_BITRATE));
	param.nWidth = gLowRate.dst_w;
	param.nHeight = gLowRate.dst_h;
	param.nBitrate = gLowRate.nBitrate;
	param.nFrame = gLowRate.nFrame;
	length = sizeof(LOW_BITRATE) ;
	memcpy(data, &param, sizeof(LOW_BITRATE));
	*len = length ;
	return 0;
}

/*请求低码流*/
int RequestLowRate(int num, unsigned char *data, int len, int nPos)
{
	unsigned char iis_lowrate = 0;
	LOW_BITRATE low;
	unsigned char temp[50];
	int ret = 0, length = 0;

	pthread_mutex_lock(&gParam_M.video_m);
	iis_lowrate = data[0];
	DEBUG(DL_DEBUG, "RequestLowRate() is start low rate status = %d \n", iis_lowrate);
	memcpy(&low, data + 1, sizeof(LOW_BITRATE));
	DEBUG(DL_FLOW, "<>lowbit.nWidth = %d,lowbit.nHeight = %d<>\n", low.nWidth, low.nHeight);
	DEBUG(DL_FLOW, "<>lowbit.nBitrate = %d,lowbit.nFrame = %d<>\n", low.nBitrate, low.nFrame);

	if(START == iis_lowrate) {
		ret = CheckLowRateParam(&low);

		if(0 == ret) {
			goto EXIT;
		}

		//        CalcLowRateParam();
		ret = BeyondLowRateParam(&low, &gLowRate);

		if(ret) {
			if(ret == 2) {
				ForceIframe(1);
			} else {
				gLowRate.re_enc = START;
				gVAenc_p.lowvenc.uflag = H264_ENC_PARAM_FLAG;
			}
		}

		if(0 != low.nFrame) {
			gLowRate.nFrame = low.nFrame;
		} else {
			if(0 !=  gSysParaT.videoPara[PORT_TWO].nFrameRate) {
				gLowRate.nFrame =  gSysParaT.videoPara[PORT_TWO].nFrameRate;
			}
		}

		gVAenc_p.lowvenc.bitrate = gLowRate.nBitrate;
		gVAenc_p.lowvenc.cbr = 1;
		gVAenc_p.lowvenc.height = gLowRate.dst_h;
		gVAenc_p.lowvenc.width = gLowRate.dst_w;
		gVAenc_p.lowvenc.framerate = gLowRate.nFrame;
		//save lowbit parameters
		gSysParaT.videoPara[PORT_TWO].nFrameRate = gLowRate.nFrame;
		gSysParaT.videoPara[PORT_TWO].sBitrate = gLowRate.nBitrate;
		gSysParaT.videoPara[PORT_TWO].nHight = gLowRate.dst_h;
		gSysParaT.videoPara[PORT_TWO].nWidth = gLowRate.dst_w;
		/*开启低码流编码*/
		SETLOWRATESTART(START);
		/*请求低码流状态*/
		SETLOWRATEFLAG(num, nPos, START);
		DEBUG(DL_DEBUG, "RequestLowRate() is start low rate num = %d , nPos = %d\n", num, nPos);
	} else {
		SETLOWRATESTART(STOP);
	}

EXIT:
	RetLowRateParam(temp, &length);
	LowRateBroadCastMsg(PORT_ONE, MSG_LOW_BITRATE, temp, length);
	pthread_mutex_unlock(&gParam_M.video_m);
	return 0;
}

/*调节图像的边框*/
int RevisePictureFrame(unsigned char *data, int len)
{
	pic_revise      *pic;
	int             ret = 0;
	short           hporch, vporch;

	pic = (pic_revise *)data;
	hporch = pic->hporch;
	vporch = pic->vporch;
	DEBUG(DL_DEBUG, "hporch = %d vporch= %d \n", hporch, vporch);
	ret = revise_picture(hporch, vporch);
	return 0;
}


/*Print current avaliable users*/
void PrintClientNum()
{
	int i;

	for(i = 0 ; i < MAX_CLIENT; i++) {
		if(ISUSED(PORT_ONE, i) && ISLOGIN(PORT_ONE, i)) {
			DEBUG(DL_DEBUG, "##################################################################\n");
			DEBUG(DL_DEBUG, "count = %d  sock = %d  ip = %s  STATUS:%s\n", i, GETSOCK(PORT_ONE, i), IPdotdec[i], (GETLOWRATEFLAG(PORT_ONE, i) ? "LOW" : "HIGH"));
			DEBUG(DL_DEBUG, "##################################################################\n");
		}
	}
}

#define PER_READ_LEN  256
/*升级FPGA程序*/
int updateFpgaProgram(const char *fpgafile)
{
	int ret = 0;
	char spidata[PER_READ_LEN];
	int spifd = -1;
	FILE *fpgafd = NULL;
	int readlen = 0;
	int writelen = 0;
	int totalwritelen  = 0;

	DEBUG(DL_DEBUG, "Enter into update FPGA Program \n");
	ret = clear_gpio_bit(GPIO_4);

	if(ret < 0) {
		clear_gpio_bit(GPIO_4);
	}

	ret = clear_gpio_bit(GPIO_6);

	if(ret < 0) {
		clear_gpio_bit(GPIO_6);
	}

	system("flash_eraseall /dev/mtd4");
	spifd =  open("/dev/mtd4", O_RDWR, 0);

	if(spifd < 0)	{
		DEBUG(DL_WARNING, "open the SPI flash Failed \n");
		ret = -1;
		goto cleanup;
	}

	fpgafd = fopen(fpgafile, "r+b");

	if(fpgafd == NULL)	{
		DEBUG(DL_ERROR, "open the FPGA bin Failed \n");
		ret = -1;
		goto cleanup;
	}

	rewind(fpgafd);

	while(1) {
		readlen = fread(spidata, 1, PER_READ_LEN, fpgafd);

		if(readlen < 1)	{
			DEBUG(DL_DEBUG, "file read end \n");
			break;
		}

		writelen = write(spifd, spidata, readlen);
		totalwritelen += writelen;

		if(feof(fpgafd)) {
			writelen = write(spifd, spidata, readlen);
			break;
		}
	}

	close(spifd);
	DEBUG(DL_DEBUG, "totalwritelen = %d \n", totalwritelen);
cleanup:
	DEBUG(DL_DEBUG, "002 flash_eraseall updateFpgaProgram \n");
	ret = set_gpio_bit(GPIO_4);

	if(ret == 0) {
		set_gpio_bit(GPIO_4);
	}

	ret = set_gpio_bit(GPIO_6);

	if(ret == 0) {
		set_gpio_bit(GPIO_6);
	}

	if(spifd > 0) {
		close(spifd);
	}

	if(fpgafd) {
		fclose(fpgafd);
	}

	int i = 0;
	int bit = 0;

	for(i = 0; i < 15; i++) {
		bit = get_gpio_bit(GPIO_5);

		if(bit) {
			break;
		}

		sleep(1);
		DEBUG(DL_DEBUG, "get gpio Bit = %d  time = %d\n", bit, i);
	}

	if(!bit) {    //FPGA Update Failed!!
		ret = -1;
	}

	DEBUG(DL_DEBUG, "Leave out  updateFpgaProgram   bit = %d , i = %d\n", bit, i);

	return ret;
}

/*升级内核*/
static int updatekernel(void)
{
	char command[256] = {0};
	char filepath[256] = {0};
	int ret =  -2;

	//	system(command);
	sprintf(filepath, "/opt/update/%s", KERNELFILENAME);

	/*更新内核*/
	if(access(filepath, F_OK) == 0) {
		system("flash_eraseall /dev/mtd2");
		sprintf(command, "nandwrite -p /dev/mtd2 /opt/update/%s", KERNELFILENAME);
		system(command);
	}

	/*更新FPGA*/
	sprintf(filepath, "/opt/update/%s", FPGAFILENAME);

	if(access(filepath, F_OK) == 0) {
		SET_FPGA_UPDATE(FPGA_UPDATE);
		sleep(1);
		ret = updateFpgaProgram(filepath);
	}

	system("rm -rf /opt/update");
	return (ret);
}

/*update program*/
int DoUpdateProgram(int sSocket, unsigned char *data, int len)
{
	unsigned long filesize;
	unsigned char *p;
	unsigned long nLen;
	unsigned char temp[20];
	int ret;
	FILE *fp = NULL;
	p = NULL;
	p = malloc(8 * 1024);

	DEBUG(DL_DEBUG, "The File Updata Buffer Is 64KB!\n");

	if(!p) {
		DEBUG(DL_ERROR, "Malloc 8KB Buffer Failed!\n");
	} else {
		DEBUG(DL_DEBUG, "The Buffer Addr 0x%x\n", (unsigned int)p);
	}

	if((fp = fopen("/update.tgz", "w+")) == NULL) {
		DEBUG(DL_ERROR, "open %s error \n", "update.tgz");
		return -1;
	}

	DEBUG(DL_DEBUG, "szData:%x, %x, %x, %x, %x, %x,%x\n", data[0], data[1], data[2],
	      data[HEAD_LEN], data[HEAD_LEN + 1], data[HEAD_LEN + 2], data[HEAD_LEN + 3]);
	filesize = ((unsigned char)data[HEAD_LEN]) | ((unsigned char)data[HEAD_LEN + 1]) << 8 |
	           ((unsigned char)data[HEAD_LEN + 2]) << 16 | ((unsigned char)data[HEAD_LEN + 3]) << 24;
	DEBUG(DL_DEBUG, "Updata file size = 0x%x! \n", (unsigned int)filesize);

	while(filesize) {
		nLen = recv(sSocket, p, 8 * 1024, 0);
		DEBUG(DL_DEBUG, "recv Updata File 0x%x Bytes!\n", (unsigned int)nLen);

		if(nLen <= 0) {
			free(p);
			fclose(fp);
			p = NULL;
			return -1;
		}

		ret = fwrite(p, 1, nLen, fp);

		if(ret < 0) {
			DEBUG(DL_ERROR, "product update faith!\n");
			free(p);
			fclose(fp);
			p = NULL;
			PackHeaderMSG((BYTE *)temp, MSG_UpdataFile_FAILS, HEAD_LEN);
			WriteData(sSocket, temp, HEAD_LEN);
			return -1;
		}

		DEBUG(DL_DEBUG, "write data ...........................:0x%x\n", (unsigned int)filesize);
		filesize = filesize - nLen;
	}

	DEBUG(DL_DEBUG, "recv finish......................\n");
	free(p);
	fclose(fp);

#ifdef ADD_UPDATE_HEADER
	DEBUG(DL_DEBUG, "ADD_UPDATE_HEADER start...\n");
	char command[320] = {0};
	char filename[256] = {0};
	strcpy(filename, "/update.tgz");
	ret = app_header_info_ok(filename);
	DEBUG(DL_DEBUG, "ADD_UPDATE_HEADER start...ret : [%d]\n", ret);

	if(ret == 0) {
		if(app_del_extern_buff(filename) == -1) {
			PRINTF("error! del the file extern_buff is error\n");
			return -1;
		}

		sprintf(command, "rm -rf %s;mv %s_temp %s", filename, filename, filename);
		PRINTF("success del extern header,command=%s\n", command);
		system(command);

		//sleep(10);
		//return -1;
	} else {
		if(ret == 2) {
			PRINTF("This file isn't have add extern update header\n");
			PackHeaderMSG((BYTE *)temp, MSG_UpdataFile_FAILS, HEAD_LEN);
			WriteData(sSocket, temp, HEAD_LEN);
			return -1;
		} else {
			PRINTF("This file have add extern update header,but the header have some error\n");
			PackHeaderMSG((BYTE *)temp, MSG_UpdataFile_FAILS, HEAD_LEN);
			WriteData(sSocket, temp, HEAD_LEN);
			return -1;
		}
	}

	DEBUG(DL_DEBUG, "ADD_UPDATE_HEADER end...\n");
#endif


	system("rm -rf /opt/update");
	system("mkdir -p /opt/update");
	SET_FPGA_UPDATE(FPGA_UPDATE);
	sleep(1);
	p = NULL;
	ret = system("./update.sh");
	DEBUG(DL_DEBUG, "update succeed !! \n");

	/*升级内核*/
	ret = updatekernel();

	if(ret == 0) {
		PackHeaderMSG((BYTE *)temp, MSG_UpdataFile_SUCC, HEAD_LEN);
	} else {
		PackHeaderMSG((BYTE *)temp, MSG_UpdataFile_FAILS, HEAD_LEN);
	}

	WriteData(sSocket, temp, HEAD_LEN);
	system("sync");
	//modify by zm for update reboot
	app_set_reboot_flag(1);

	sleep(5);
	/*重启*/
	system("reboot -f");
	return 0;
}




/*获得文本长度*/
int GetTextLen(char *Data)
{
	int n_Len = strlen(Data);
	unsigned char *lpData = (unsigned char *)Data;
	int i = 0;
	int Sum = 0;

	for(i = 0; /**pStr != '\0'*/ i < n_Len ;) {
		if(*lpData <= 127) {
			i++;
			lpData++;
		} else {
			i += 2;
			lpData += 2;
		}

		Sum++;
	}

	return Sum;
}

int GetSizeLen(char *Data)
{
	int n_Len = strlen(Data);
	unsigned char *lpData = (unsigned char *)Data;
	int i = 0;
	int Sum = 0;

	for(i = 0; /**pStr != '\0'*/ i < n_Len ;) {
		if(*lpData <= 127) {
			i++;
			lpData++;
		} else {
			i += 2;
			lpData += 2;
		}

		Sum++;
	}

	return i;
}

#ifdef CL4000_DVI_SDI
/*从配置文件中读取参数*/
int ReadEncodeParamTable(char *config_file, SavePTable *tblParam)
{
	SavePTable 		tmpTable;
	char 			temp[512];
	int 			ret  = -1 ;
	struct in_addr	addr ;
	unsigned int 	value;
	unsigned int 	dwAddr, dwNetmask, dwGateWay;


	pthread_mutex_lock(&gSetP_m.save_sys_m);
	/*MAC Address*/
	DEBUG(DL_DEBUG, "read config = %s \n", config_file);
	ret =  ConfigGetKey(config_file, "sysParam", "macaddr", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key mac addr failed\n");
		goto EXIT;
	}

	ret = SplitMacAddr(temp, tmpTable.sysPara.szMacAddr, 6);

	if(ret < 0) {
		DEBUG(DL_ERROR, "[SplitMacAddr] mac addr error\n");
		goto EXIT;
	}
	printf("-------------mic=%x:%x:%x:%x:%x:%x\n",tmpTable.sysPara.szMacAddr[0],
		tmpTable.sysPara.szMacAddr[1],tmpTable.sysPara.szMacAddr[2],
		tmpTable.sysPara.szMacAddr[3],tmpTable.sysPara.szMacAddr[4],tmpTable.sysPara.szMacAddr[5]);

	/*IP address*/
	ret =  ConfigGetKey(config_file, "sysParam", "ipaddr", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key ipaddr failed\n");
		goto EXIT;
	}

	inet_aton(temp, &addr);
	memcpy(&value, &addr, 4);
	dwAddr = value;
	//	tmpTable.sysPara.dwAddr = value;

	/*GateWay address*/
	ret =  ConfigGetKey(config_file, "sysParam", "gateway", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key gateway failed\n");
		goto EXIT;
	}

	inet_aton(temp, &addr);
	memcpy(&value, &addr, 4);
	dwGateWay = value;
	//	tmpTable.sysPara.dwGateWay = value;

	/*NetMask*/
	ret =  ConfigGetKey(config_file, "sysParam", "netmask", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key netmask failed\n");
		goto EXIT;
	}

	inet_aton(temp, &addr);
	memcpy(&value, &addr, 4);
	dwNetmask = value;

	//	tmpTable.sysPara.dwNetMark= value;
	/*check IP netmask gateway*/
	if(CheckIPNetmask(dwAddr, dwNetmask, dwGateWay)) {
		tmpTable.sysPara.dwAddr = dwAddr;
		tmpTable.sysPara.dwGateWay = dwGateWay;
		tmpTable.sysPara.dwNetMark = dwNetmask;
	}	else {
		return 0;
	}

	/*Name*/
	ret =  ConfigGetKey(config_file, "sysParam", "name", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key name failed\n");
		goto EXIT;
	}

	strcpy(tmpTable.sysPara.strName, temp);

	/*Version*/
	ret =  ConfigGetKey(config_file, "sysParam", "version", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key version failed\n");
		goto EXIT;
	}

	strcpy(tmpTable.sysPara.strVer, temp);

	/*Channels*/
	ret =  ConfigGetKey(config_file, "sysParam", "channels", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key channels failed\n");
		goto EXIT;
	}

	tmpTable.sysPara.unChannel = atoi(temp);
	/*type*/
	ret =  ConfigGetKey(config_file, "sysParam", "type", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key type failed\n");
		goto EXIT;
	}

	//	tmpTable.sysPara.bType= atoi(temp);
	tmpTable.sysPara.bType = ENCODE_TYPE;
	/*video param */
	/*data codec*/
	ret =  ConfigGetKey(config_file, "videoParam", "codec", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key codec failed\n");
		goto EXIT;
	}

	tmpTable.videoPara[PORT_ONE].nDataCodec = strtol(temp, 0, 16);

	/*frame rate */
	ret =  ConfigGetKey(config_file, "videoParam", "framerate", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key framerate failed\n");
		goto EXIT;
	}

	tmpTable.videoPara[PORT_ONE].nFrameRate = atol(temp);
	/*width*/
	ret =  ConfigGetKey(config_file, "videoParam", "width", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key width failed\n");
		goto EXIT;
	}

	tmpTable.videoPara[PORT_ONE].nWidth = atol(temp);
	/*heigth*/
	ret =  ConfigGetKey(config_file, "videoParam", "height", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key height failed\n");
		goto EXIT;
	}

	tmpTable.videoPara[PORT_ONE].nHight = atol(temp);

	/*colors*/
	ret =  ConfigGetKey(config_file, "videoParam", "colors", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key colors failed\n");
		goto EXIT;
	}

	tmpTable.videoPara[PORT_ONE].nColors = atol(temp);
	/*quality*/
	ret =  ConfigGetKey(config_file, "videoParam", "quality", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key quality failed\n");
		goto EXIT;
	}

	tmpTable.videoPara[PORT_ONE].nQuality = atol(temp);

	/*CBR*/
	ret =  ConfigGetKey(config_file, "videoParam", "cbr", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key cbr failed\n");
		goto EXIT;
	}

	tmpTable.videoPara[PORT_ONE].sCbr = atoi(temp);
	/*bitrate*/
	ret =  ConfigGetKey(config_file, "videoParam", "bitrate", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key bitrate failed\n");
		goto EXIT;
	}

	tmpTable.videoPara[PORT_ONE].sBitrate = atoi(temp);

	/*Audio Param*/
	ret =  ConfigGetKey(config_file, "audioParam", "codec", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key codec failed\n");
		goto EXIT;
	}

	tmpTable.audioPara[PORT_ONE].Codec = strtol(temp, 0, 16);

	/*samplerate*/
	ret =  ConfigGetKey(config_file, "audioParam", "samplerate", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key samplerate failed\n");
		goto EXIT;
	}

	tmpTable.audioPara[PORT_ONE].SampleRate = atol(temp);

	/*bitrate*/
	ret =  ConfigGetKey(config_file, "audioParam", "bitrate", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key bitrate failed\n");
		goto EXIT;
	}

	tmpTable.audioPara[PORT_ONE].BitRate = atol(temp);
	/*channel*/
	ret =  ConfigGetKey(config_file, "audioParam", "channel", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key channel failed\n");
		goto EXIT;
	}

	tmpTable.audioPara[PORT_ONE].Channel = atol(temp);

	/*samplebit */
	ret =  ConfigGetKey(config_file, "audioParam", "samplebit", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key samplebit failed\n");
		goto EXIT;
	}

	tmpTable.audioPara[PORT_ONE].SampleBit = atol(temp);

	/*Lvolume */
	ret =  ConfigGetKey(config_file, "audioParam", "Lvolume", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key Lvolume failed\n");
		goto EXIT;
	}

	tmpTable.audioPara[PORT_ONE].LVolume = atoi(temp);

	/*Rvolume */
	ret =  ConfigGetKey(config_file, "audioParam", "Rvolume", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key Rvolume failed\n");
		goto EXIT;
	}

	tmpTable.audioPara[PORT_ONE].RVolume = atoi(temp);
	/*inputMode */
	ret =  ConfigGetKey(config_file, "audioParam", "inputMode", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key Rvolume failed\n");
		goto EXIT;
	}

	tmpTable.audioPara[PORT_ONE].InputMode = atoi(temp);

	memcpy(tblParam, &tmpTable, sizeof(SavePTable));
EXIT:
	pthread_mutex_unlock(&gSetP_m.save_sys_m);
	return 0;
}
#endif

#ifdef DSS_ENC_1100_1200
#define DSP1 PORT_ONE
/*read param table from file enc1200*/
int ReadEncodeParamTable(char *config_file, SavePTable *tblParam)
{
	SavePTable 		tmpTable;
	char 			temp[512];
	int 			ret  = -1 ;
	struct in_addr	addr ;
	unsigned int 	value;
	unsigned int 	dwAddr, dwNetmask, dwGateWay;


	pthread_mutex_lock(&gSetP_m.save_sys_m);

#if 1
	/*MAC Address*/
	DEBUG(DL_DEBUG, "read config = %s \n", config_file);
	ret =  ConfigGetKey(config_file, "sysParam", "macaddr", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key mac addr failed\n");
		goto EXIT;
	}

	ret = SplitMacAddr(temp, tmpTable.sysPara.szMacAddr, 6);

	if(ret < 0) {
		DEBUG(DL_ERROR, "[SplitMacAddr] mac addr error\n");
		goto EXIT;
	}
#endif
	
	/*IP address*/
	ret =  ConfigGetKey(config_file, "sysParam", "ipaddr", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key ipaddr failed\n");
		goto EXIT;
	}

	inet_aton(temp, &addr);
	memcpy(&value, &addr, 4);
	dwAddr = value;
	//	tmpTable.sysPara.dwAddr = value;

	/*GateWay address*/
	ret =  ConfigGetKey(config_file, "sysParam", "gateway", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key gateway failed\n");
		goto EXIT;
	}

	inet_aton(temp, &addr);
	memcpy(&value, &addr, 4);
	dwGateWay = value;
	//	tmpTable.sysPara.dwGateWay = value;

	/*NetMask*/
	ret =  ConfigGetKey(config_file, "sysParam", "netmask", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key netmask failed\n");
		goto EXIT;
	}

	inet_aton(temp, &addr);
	memcpy(&value, &addr, 4);
	dwNetmask = value;

	//	tmpTable.sysPara.dwNetMark= value;
	/*check IP netmask gateway*/
	if(CheckIPNetmask(dwAddr, dwNetmask, dwGateWay)) {
		tmpTable.sysPara.dwAddr = dwAddr;
		tmpTable.sysPara.dwGateWay = dwGateWay;
		tmpTable.sysPara.dwNetMark = dwNetmask;
	}	else {
		return 0;
	}

	/*Name*/
	ret =  ConfigGetKey(config_file, "sysParam", "name", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key name failed\n");
		goto EXIT;
	}
	
	//modify by lichl
	if(0 == strncmp(temp,"DSS-ENC-MOD", 11))
	{
		ret =  ConfigGetKey("config.dat", "sysParam", "name", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "Get Key name failed\n");
			goto EXIT;
		}
	}
	
	strcpy(tmpTable.sysPara.strName, temp);

	/*Version*/
	ret =  ConfigGetKey(config_file, "sysParam", "version", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key version failed\n");
		goto EXIT;
	}

	strcpy(tmpTable.sysPara.strVer, temp);

	/*Channels*/
	ret =  ConfigGetKey(config_file, "sysParam", "channels", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key channels failed\n");
		goto EXIT;
	}

	tmpTable.sysPara.unChannel = atoi(temp);
	/*type*/
	ret =  ConfigGetKey(config_file, "sysParam", "type", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key type failed\n");
		goto EXIT;
	}

	tmpTable.sysPara.bType = atoi(temp);
#if (defined(DSS_ENC_1200) || defined(DSS_ENC_1260))
	tmpTable.sysPara.bType = 6;
#else   //DSS_ENC_1100
	tmpTable.sysPara.bType = 8;
#endif


	/*DHCP FLAG  == syspar.nTemp[0]*/
	ret =  ConfigGetKey(config_file, "sysParam", "dhcp", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key channels failed\n");
		goto EXIT;
	}

	tmpTable.sysPara.nTemp[0] = atoi(temp);

	if(tmpTable.sysPara.nTemp[0] != 0 && tmpTable.sysPara.nTemp[0] != 1) {
		tmpTable.sysPara.nTemp[0] = 0;
	}

	PRINTF("tmpTable.sysPara.nTemp[0] = %d\n", tmpTable.sysPara.nTemp[0]);

	/*video param */
	/*data codec*/
	ret =  ConfigGetKey(config_file, "videoParam", "codec", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key codec failed\n");
		goto EXIT;
	}

	tmpTable.videoPara[DSP1].nDataCodec = strtol(temp, 0, 16);

	/*frame rate */
	ret =  ConfigGetKey(config_file, "videoParam", "framerate", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key framerate failed\n");
		goto EXIT;
	}

	tmpTable.videoPara[DSP1].nFrameRate = atol(temp);
	/*width*/
	ret =  ConfigGetKey(config_file, "videoParam", "width", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key width failed\n");
		goto EXIT;
	}

	tmpTable.videoPara[DSP1].nWidth = atol(temp);
	/*heigth*/
	ret =  ConfigGetKey(config_file, "videoParam", "height", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key height failed\n");
		goto EXIT;
	}

	tmpTable.videoPara[DSP1].nHight = atol(temp);

	/*colors*/
	ret =  ConfigGetKey(config_file, "videoParam", "colors", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key colors failed\n");
		goto EXIT;
	}

	tmpTable.videoPara[DSP1].nColors = atol(temp);
	/*quality*/
	ret =  ConfigGetKey(config_file, "videoParam", "quality", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key quality failed\n");
		goto EXIT;
	}

	tmpTable.videoPara[DSP1].nQuality = atol(temp);

	/*CBR*/
	ret =  ConfigGetKey(config_file, "videoParam", "cbr", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key cbr failed\n");
		goto EXIT;
	}

	tmpTable.videoPara[DSP1].sCbr = atoi(temp);
	/*bitrate*/
	ret =  ConfigGetKey(config_file, "videoParam", "bitrate", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key bitrate failed\n");
		goto EXIT;
	}

	tmpTable.videoPara[DSP1].sBitrate = atoi(temp);

	/*Audio Param*/
	ret =  ConfigGetKey(config_file, "audioParam", "codec", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key codec failed\n");
		goto EXIT;
	}

	tmpTable.audioPara[DSP1].Codec = strtol(temp, 0, 16);

	/*samplerate*/
	ret =  ConfigGetKey(config_file, "audioParam", "samplerate", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key samplerate failed\n");
		goto EXIT;
	}
	//only have 48khz
	//tmpTable.audioPara[DSP1].SampleRate = atol(temp);
	tmpTable.audioPara[DSP1].SampleRate = 3;  //just 48KHZ
	/*bitrate*/
	ret =  ConfigGetKey(config_file, "audioParam", "bitrate", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key bitrate failed\n");
		goto EXIT;
	}

	tmpTable.audioPara[DSP1].BitRate = atol(temp);
	/*channel*/
	ret =  ConfigGetKey(config_file, "audioParam", "channel", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key channel failed\n");
		goto EXIT;
	}

	tmpTable.audioPara[DSP1].Channel = atol(temp);

	/*samplebit */
	ret =  ConfigGetKey(config_file, "audioParam", "samplebit", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key samplebit failed\n");
		goto EXIT;
	}

	tmpTable.audioPara[DSP1].SampleBit = atol(temp);

	/*Lvolume */
	ret =  ConfigGetKey(config_file, "audioParam", "Lvolume", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key Lvolume failed\n");
		goto EXIT;
	}

	tmpTable.audioPara[DSP1].LVolume = atoi(temp);

	/*Rvolume */
	ret =  ConfigGetKey(config_file, "audioParam", "Rvolume", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key Rvolume failed\n");
		goto EXIT;
	}

	tmpTable.audioPara[DSP1].RVolume = atoi(temp);
	/*inputMode */
	ret =  ConfigGetKey(config_file, "audioParam", "inputMode", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key Rvolume failed\n");
		goto EXIT;
	}

	tmpTable.audioPara[DSP1].InputMode = atoi(temp);

	memcpy(tblParam, &tmpTable, sizeof(SavePTable));
EXIT:
	pthread_mutex_unlock(&gSetP_m.save_sys_m);
	return 0;
}

/*protocol ctrl*/
int ReadProtocolIni(const char *config_file, Protocol *pp)
{
	char 			temp[512];
	int 			ret  = -1 ;

	pthread_mutex_lock(&gSetP_m.save_sys_m);
	pp->count = 0;
	/*status*/
	ret =  ConfigGetKey(config_file, "protocol", "status", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Failed to Get Protocol status\n");
		goto EXIT;
	}

	pp->status = atoi(temp);
	/*TS stream*/
	ret =  ConfigGetKey(config_file, "protocol", "TS", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Failed to Get Protocol TS stream \n");
		goto EXIT;
	}

	pp->TS = atoi(temp);

	if(pp->TS && pp->status) {
		pp->count++;
	}

	/*RTP protocol*/
	ret =  ConfigGetKey(config_file, "protocol", "RTP", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Failed to Get Protocol RTP protocol \n");
		goto EXIT;
	}

	pp->RTP = atoi(temp);

	if(pp->RTP && pp->status) {
		pp->count++;
	}

	/*RTSP protocol*/
	ret =  ConfigGetKey(config_file, "protocol", "RTSP", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Failed to Get Protocol RTSP protocol\n");
		goto EXIT;
	}

	pp->RTSP = atoi(temp);

	if(pp->RTSP && pp->status) {
		pp->count++;
	}

	/*RTMP protocol*/
	ret =  ConfigGetKey(config_file, "protocol", "RTMP", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Failed to Get Protocol RTMP protocol\n");
		goto EXIT;
	}

	pp->RTMP = atoi(temp);

	if(pp->RTMP && pp->status) {
		pp->count++;
	}

EXIT:
	pthread_mutex_unlock(&gSetP_m.save_sys_m);
	return 0;
}

#endif //DSS_ENC_1100_1200

/*从配置文件中读取低码率参数*/
int ReadLowbitParamTable(char *config_file, SavePTable *tblParam)
{
	char 			temp[512];
	int 			ret  = -1 ;

	if(access(config_file, F_OK) != 0) {

		tblParam->videoPara[PORT_TWO].nWidth = 352;
		tblParam->videoPara[PORT_TWO].nHight = 288;
		tblParam->videoPara[PORT_TWO].sBitrate = 384;
		tblParam->videoPara[PORT_TWO].nFrameRate = 25;
		SaveLowRateParamsToFlash();
		goto EXIT;
	}

	//	pthread_mutex_lock(&gSetP_m.save_sys_m);
	DEBUG(DL_DEBUG, "read lowbit config = %s \n", config_file);
	/*low rate width*/
	ret =  ConfigGetKey(config_file, "videoResizeParam", "width", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key Resize width failed\n");
		goto EXIT;
	}

	tblParam->videoPara[PORT_TWO].nWidth = atol(temp);
	DEBUG(DL_ERROR, "Get Key low bitrate width : %d\n", tblParam->videoPara[PORT_TWO].nWidth);

	/*low rate heigth*/
	ret =  ConfigGetKey(config_file, "videoResizeParam", "height", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key height failed\n");
		goto EXIT;
	}

	tblParam->videoPara[PORT_TWO].nHight = atol(temp);
	DEBUG(DL_ERROR, "Get Key low bitrate hight : %d\n", tblParam->videoPara[PORT_TWO].nHight);

	/*low rate bitrate*/
	ret =  ConfigGetKey(config_file, "videoResizeParam", "bitrate", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key low bitrate failed\n");
		goto EXIT;
	}

	tblParam->videoPara[PORT_TWO].sBitrate = atoi(temp);
	DEBUG(DL_ERROR, "Get Key low bitrate : %d\n", tblParam->videoPara[PORT_TWO].sBitrate);

	/*low rate framerate */
	ret =  ConfigGetKey(config_file, "videoResizeParam", "framerate", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key framerate failed\n");
		goto EXIT;
	}

	tblParam->videoPara[PORT_TWO].nFrameRate = atol(temp);
	DEBUG(DL_ERROR, "Get Key lowrate framerate : %dfp\n", tblParam->videoPara[PORT_TWO].nFrameRate);
EXIT:
	//	pthread_mutex_unlock(&gSetP_m.save_sys_m);
	return 0;

}

/*从配置文件中读取ip参数*/
int ReadIPParamTable(char *config_file, SavePTable *tblParam)
{
	char 			    temp[512];
	int 			    ret  = -1 ;
	struct in_addr	    addr ;
	unsigned int 	    ipAddr, ipNetmask, ipGateWay;

	//	pthread_mutex_lock(&gSetP_m.save_sys_m);
	DEBUG(DL_DEBUG, "read config = %s \n", config_file);
	/*IP address*/
	ret =  ConfigGetKey(config_file, "ipParam", "ipaddr", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key ipaddr failed\n");
		goto EXIT;
	}

	DEBUG(DL_FLOW, "Get Key ipaddr : %s\n", temp);
	inet_aton(temp, &addr);
	memcpy(&ipAddr, &addr, 4);
	//    tblParam->sysPara.dwAddr = ipAddr;

	/*GateWay address*/
	ret =  ConfigGetKey(config_file, "ipParam", "gateway", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key gateway failed\n");
		goto EXIT;
	}

	DEBUG(DL_ERROR, "Get Key gateway : %s\n", temp);
	inet_aton(temp, &addr);
	memcpy(&ipGateWay, &addr, 4);
	//    tblParam->sysPara.dwGateWay = ipGateWay;

	/*NetMask*/
	ret =  ConfigGetKey(config_file, "ipParam", "netmask", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key netmask failed\n");
		goto EXIT;
	}

	DEBUG(DL_ERROR, "Get Key netmask : %s\n", temp);
	inet_aton(temp, &addr);
	memcpy(&ipNetmask, &addr, 4);
	//    tblParam->sysPara.dwNetMark = ipNetmask;

	/*check IP netmask gateway*/
	if(CheckIPNetmask(ipAddr, ipNetmask, ipGateWay)) {
		tblParam->sysPara.dwAddr = ipAddr;
		tblParam->sysPara.dwGateWay = ipGateWay;
		tblParam->sysPara.dwNetMark = ipNetmask;
	} else {
		return 0;
	}

EXIT:
	//	pthread_mutex_unlock(&gSetP_m.save_sys_m);
	return 0;
}


/*保存参数到文件中*/
int SaveParamsToFlash()
{
	PRINTF("begin \n");
	SavePTable 		*pTbl = NULL;
	SavePTable            info;

	char 			temp[512] = {0};
	char config_file[512] = {0};
	char                    command[512] = {0};
	int 			ret  = -1 ;
	struct in_addr		addr ;

	memset(&info, 0, sizeof(SavePTable));

	pthread_mutex_lock(&gSetP_m.save_sys_m);
	memcpy(&info, &gSysParaT, sizeof(SavePTable));
	pthread_mutex_unlock(&gSetP_m.save_sys_m);

	pTbl = &info;





	snprintf(config_file, sizeof(config_file), "%s.bak", CONFIG_NAME);

	/*拷贝bak文件到ini*/
	snprintf(command, sizeof(command), "cp -rf %s %s;rm -rf %s", config_file, CONFIG_NAME, config_file);

	ret = CheckIPNetmask(pTbl->sysPara.dwAddr, pTbl->sysPara.dwNetMark, pTbl->sysPara.dwGateWay);

	if(ret == 0) {
		DEBUG(DL_ERROR, "IP addr no real!!!!\n");
		return 0;
	}

	//strcpy(config_file, CONFIG_NAME);
	/*MAC Address*/
	ret = JoinMacAddr(temp, pTbl->sysPara.szMacAddr, 6);

	if(ret < 0) {
		DEBUG(DL_ERROR, "[JoinMacAddr] mac addr error ret = %d\n", ret);
		goto EXIT;
	}

	DEBUG(DL_DEBUG, "write config = %s \n", config_file);
	ret =  ConfigSetKey(config_file, "sysParam", "macaddr", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Set Key mac addr failed\n");
		goto EXIT;
	}

	/*dhcp flag*/
	sprintf(temp, "%d", pTbl->sysPara.nTemp[0]);
	ret =  ConfigSetKey(config_file, "sysParam", "dhcp", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Set Key channels failed\n");
		goto EXIT;
	}


	//if(pTbl->sysPara.nTemp[0] == 0)
	{
		/*IP address*/
		memcpy(&addr, &pTbl->sysPara.dwAddr, 4);
		strcpy(temp, inet_ntoa(addr));
		ret =  ConfigSetKey(config_file, "sysParam", "ipaddr", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "Set Key ipaddr failed\n");
			goto EXIT;
		}

		/*GateWay address*/
		memcpy(&addr, &pTbl->sysPara.dwGateWay, 4);
		strcpy(temp, inet_ntoa(addr));
		ret =  ConfigSetKey(config_file, "sysParam", "gateway", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "Set Key gateway failed\n");
			goto EXIT;
		}

		/*NetMask*/
		memcpy(&addr, &pTbl->sysPara.dwNetMark, 4);
		strcpy(temp, inet_ntoa(addr));
		ret =  ConfigSetKey(config_file, "sysParam", "netmask", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "Set Key netmask failed\n");
			goto EXIT;
		}
	}

	/*Name*/
	strcpy(temp, pTbl->sysPara.strName);
	ret =  ConfigSetKey(config_file, "sysParam", "name", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Set Key name failed\n");
		goto EXIT;
	}

	/*Version*/
	strcpy(temp, pTbl->sysPara.strVer);
	ret =  ConfigSetKey(config_file, "sysParam", "version", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Set Key version failed\n");
		goto EXIT;
	}

	/*Channels*/
	sprintf(temp, "%d", pTbl->sysPara.unChannel);
	ret =  ConfigSetKey(config_file, "sysParam", "channels", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Set Key channels failed\n");
		goto EXIT;
	}

	sprintf(temp, "%d", ENCODE_TYPE);
	/*type*/
	ret =  ConfigSetKey(config_file, "sysParam", "type", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Set Key channels failed\n");
		goto EXIT;
	}





	/*video param */
	/*data codec*/
	sprintf(temp, "%x", pTbl->videoPara[PORT_ONE].nDataCodec);
	ret =  ConfigSetKey(config_file, "videoParam", "codec", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Set Key codec failed\n");
		goto EXIT;
	}

	/*frame rate */
	sprintf(temp, "%d", pTbl->videoPara[PORT_ONE].nFrameRate);
	ret =  ConfigSetKey(config_file, "videoParam", "framerate", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Set Key framerate failed\n");
		goto EXIT;
	}

	/*width*/
	sprintf(temp, "%d", pTbl->videoPara[PORT_ONE].nWidth);
	ret =  ConfigSetKey(config_file, "videoParam", "width", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Set Key width failed\n");
		goto EXIT;
	}

	/*heigth*/
	sprintf(temp, "%d", pTbl->videoPara[PORT_ONE].nHight);
	ret =  ConfigSetKey(config_file, "videoParam", "height", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Set Key height failed\n");
		goto EXIT;
	}

	/*colors*/
	sprintf(temp, "%d", pTbl->videoPara[PORT_ONE].nColors);
	ret =  ConfigSetKey(config_file, "videoParam", "colors", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Set Key colors failed\n");
		goto EXIT;
	}

	/*quality*/
	sprintf(temp, "%d", pTbl->videoPara[PORT_ONE].nQuality);
	ret =  ConfigSetKey(config_file, "videoParam", "quality", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Set Key quality failed\n");
		goto EXIT;
	}

	/*CBR*/
	sprintf(temp, "%d", pTbl->videoPara[PORT_ONE].sCbr);
	ret =  ConfigSetKey(config_file, "videoParam", "cbr", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Set Key cbr failed\n");
		goto EXIT;
	}

	/*bitrate*/
	sprintf(temp, "%d", pTbl->videoPara[PORT_ONE].sBitrate);
	ret =  ConfigSetKey(config_file, "videoParam", "bitrate", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Set Key bitrate failed\n");
		goto EXIT;
	}

	/*Audio Param*/
	sprintf(temp, "%x", pTbl->audioPara[PORT_ONE].Codec);
	ret =  ConfigSetKey(config_file, "audioParam", "codec", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Set Key codec failed\n");
		goto EXIT;
	}

	/*samplerate*/
	sprintf(temp, "%x", pTbl->audioPara[PORT_ONE].SampleRate);
	ret =  ConfigSetKey(config_file, "audioParam", "samplerate", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Set Key samplerate failed\n");
		goto EXIT;
	}

	/*bitrate*/
	sprintf(temp, "%d", pTbl->audioPara[PORT_ONE].BitRate);
	ret =  ConfigSetKey(config_file, "audioParam", "bitrate", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Set Key bitrate failed\n");
		goto EXIT;
	}

	/*channel*/
	sprintf(temp, "%d", pTbl->audioPara[PORT_ONE].Channel);
	ret =  ConfigSetKey(config_file, "audioParam", "channel", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Set Key channel failed\n");
		goto EXIT;
	}

	/*samplebit */
	sprintf(temp, "%d", pTbl->audioPara[PORT_ONE].SampleBit);
	ret =  ConfigSetKey(config_file, "audioParam", "samplebit", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Set Key samplebit failed\n");
		goto EXIT;
	}

	/*Lvolume */
	sprintf(temp, "%d", pTbl->audioPara[PORT_ONE].LVolume);
	ret =  ConfigSetKey(config_file, "audioParam", "Lvolume", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Set Key Lvolume failed\n");
		goto EXIT;
	}

	/*Rvolume */
	sprintf(temp, "%d", pTbl->audioPara[PORT_ONE].RVolume);
	ret =  ConfigSetKey(config_file, "audioParam", "Rvolume", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Set Key Rvolume failed\n");
		goto EXIT;
	}

	/*inputMode */
	sprintf(temp, "%d", pTbl->audioPara[PORT_ONE].InputMode);
	ret =  ConfigSetKey(config_file, "audioParam", "inputMode", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Set Key Rvolume failed\n");
		goto EXIT;
	}

	system(command);
	DEBUG(DL_DEBUG, "SAVE 	param     ok!\n");

EXIT:
	PRINTF("end \n");
	return 0;
}



/*保存参数到文件中*/
int SaveLowRateParamsToFlash()
{

	SavePTable 		*pTbl = &gSysParaT;
	char 			temp[512], config_file[512];
	int 			ret  = -1 ;

	pthread_mutex_lock(&gSetP_m.save_sys_m);
	strcpy(config_file, LOWBIT_PARAM);
	/*don't change ip , if you want to change ip ,change ipconfig.ini and reboot*/

	PRINTF("config_file = %s =%s\n", config_file, LOWBIT_PARAM);
	///*low rate frame rate *///
	sprintf(temp, "%d", pTbl->videoPara[PORT_TWO].nFrameRate);
	ret =  ConfigSetKey(config_file, "videoResizeParam", "framerate", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Set Key resize framerate failed\n");
		goto EXIT;
	}

	DEBUG(DL_FLOW, "Set Key resize framerate : %d\n", pTbl->videoPara[PORT_TWO].nFrameRate);

	///*low rate width *///
	sprintf(temp, "%d", pTbl->videoPara[PORT_TWO].nWidth);
	ret =  ConfigSetKey(config_file, "videoResizeParam", "width", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Set Key resize width failed\n");
		goto EXIT;
	}

	DEBUG(DL_FLOW, "Set Key resize width : %d\n", pTbl->videoPara[PORT_TWO].nWidth);

	/*lowrate heigth*/
	sprintf(temp, "%d", pTbl->videoPara[PORT_TWO].nHight);
	ret =  ConfigSetKey(config_file, "videoResizeParam", "height", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Set Key resize height failed\n");
		goto EXIT;
	}

	DEBUG(DL_FLOW, "Set Key resize height : %d\n", pTbl->videoPara[PORT_TWO].nHight);

	/*low rate bitrate*/
	sprintf(temp, "%d", pTbl->videoPara[PORT_TWO].sBitrate);
	ret =  ConfigSetKey(config_file, "videoResizeParam", "bitrate", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Set Key lowbit bitrate failed\n");
		goto EXIT;
	}

	DEBUG(DL_FLOW, "Set Key resize bitrate : %d\n", pTbl->videoPara[PORT_TWO].sBitrate);
	DEBUG(DL_DEBUG, "SAVE lowrate param ok!\n");

EXIT:
	pthread_mutex_unlock(&gSetP_m.save_sys_m);
	return 0;
}


/*initial hporch and vporch value*/
int InitHVTable(HVTable *pp)
{
	int i = 0;

	for(i = 0; i <= MODE_MAX ; i++) {
		pp->analog[i].hporch = FPGA_HV_TALBE[i][0];
		pp->analog[i].vporch = FPGA_HV_TALBE[i][1];
		pp->digital[i].hporch = FPGA_HV_TALBE[i][2];
		pp->digital[i].vporch = FPGA_HV_TALBE[i][3];
	}

	return 0;
}

/*read Hportch Vportch Table*/
int ReadHVTable(const HVTable *pp, int digital)
{
	int ret = 0, val = 0;
	char temp[512], config_file[512];
	HV *pHV = NULL;

	if(digital) {
		pHV = pp->analog;
		strcpy(config_file, HV_TABLE_A_NAME);
	} else {
		pHV = pp->digital;
		strcpy(config_file, HV_TABLE_D_NAME);
	}

	pthread_mutex_lock(&gSetP_m.save_sys_m);
	ret = ConfigGetKey(config_file, "640x480x60", "hporch", temp); // 0

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[0].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "640x480x60", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[0].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "640x480x72", "hporch", temp);	// 1

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[1].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "640x480x72", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[1].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "640x480x75", "hporch", temp);	// 2

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[2].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "640x480x75", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[2].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "640x480x85", "hporch", temp);	// 3

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[3].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "640x480x85", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[3].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "800x600x60", "hporch", temp);	// 4

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[4].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "800x600x60", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[4].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "800x600x72", "hporch", temp);	// 5

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[5].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "800x600x72", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[5].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "800x600x75", "hporch", temp);	// 6

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[6].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "800x600x75", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[6].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "800x600x85", "hporch", temp);	// 7

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[7].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "800x600x85", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[7].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1024x768x60", "hporch", temp);	// 8

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[8].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1024x768x60", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[8].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1024x768x70", "hporch", temp);	 // 9

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[9].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1024x768x70", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[9].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1024x768x75", "hporch", temp);	// 10

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[10].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1024x768x75", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[10].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1024x768x85", "hporch", temp);	// 11

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[11].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1024x768x85", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[11].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1280x768x60", "hporch", temp);	// 12

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[12].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1280x768x60", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[12].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1280x800x60", "hporch", temp);	// 13

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[13].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1280x800x60", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[13].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1280x960x60", "hporch", temp);	// 14

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[14].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1280x960x60", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[14].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1440x900x60", "hporch", temp);	// 15

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[15].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1440x900x60", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[15].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1400x1050x60", "hporch", temp);	// 16

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[16].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1400x1050x60", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[16].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1280x1024x60", "hporch", temp); // 17

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[17].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1280x1024x60", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[17].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1280x1024x75", "hporch", temp); // 18

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[18].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1280x1024x75", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[18].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1600x1200x60", "hporch", temp); // 19

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[19].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1600x1200x60", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[19].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1280x720V60", "hporch", temp);	 // 20

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[20].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1280x720V60", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[20].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1280x720P50", "hporch", temp);	// 21

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[21].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1280x720P50", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[21].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1280x720P60", "hporch", temp);	// 22

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[22].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1280x720P60", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[22].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1920x1080I50", "hporch", temp); // 23

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[23].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1920x1080I50", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[23].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1920x1080I60", "hporch", temp); // 24

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[24].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1920x1080I60", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[24].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1920x1080P25", "hporch", temp); // 25

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[25].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1920x1080P25", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[25].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1920x1080P30", "hporch", temp); // 26

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[26].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1920x1080P30", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[26].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1920x1080Px50", "hporch", temp); // 27

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[27].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1920x1080Px50", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[27].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1920x1080Px60", "hporch", temp); // 28

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[28].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1920x1080Px60", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[28].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "720x480I60", "hporch", temp);	 // 29

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[29].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "720x480I60", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[29].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "720x576I50", "hporch", temp);	// 30

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[30].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "720x576I50", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[30].vporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1366x768x60", "hporch", temp);	// 33

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[MODE_MAX].hporch = val;
		}
	}

	ret = ConfigGetKey(config_file, "1366x768x60", "vporch", temp);

	if(ret == 0) {
		val = atoi(temp);

		if(val > 0) {
			pHV[MODE_MAX].vporch = val;
		}
	}

	pthread_mutex_unlock(&gSetP_m.save_sys_m);
	return 0;
}

/*save HV  Table*/
int SaveHVTable(HVTable *pOld, HVTable *pNew, int digital)
{
	char  temp[512], config_file[512];
	HV *pHV = NULL, *pNewHV = NULL;
	int ret;

	if(digital) {
		pHV = pOld->digital;
		pNewHV = pNew->digital;
		strcpy(config_file, HV_TABLE_D_NAME);
	} else {
		pHV = pOld->analog ;
		pNewHV = pNew->analog;
		strcpy(config_file, HV_TABLE_A_NAME);
	}

	pthread_mutex_lock(&gSetP_m.save_sys_m);

	if(pNewHV[0].hporch != pHV[0].hporch) {
		pHV[0].hporch = pNewHV[0].hporch;
		sprintf(temp, "%d", pHV[0].hporch);
		ret = ConfigSetKey(config_file, "640x480x60", "hporch", temp); // 0

		if(ret != 0) {
			DEBUG(DL_ERROR, "640x480x60 hporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[0].vporch != pHV[0].vporch) {
		pHV[0].vporch = pNewHV[0].vporch;
		sprintf(temp, "%d", pHV[0].vporch);
		ret = ConfigSetKey(config_file, "640x480x60", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "640x480x60 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[1].hporch != pHV[1].hporch) {
		pHV[1].hporch = pNewHV[1].hporch;
		sprintf(temp, "%d", pHV[1].hporch);
		ret = ConfigSetKey(config_file, "640x480x72", "hporch", temp);	// 1

		if(ret != 0) {
			DEBUG(DL_ERROR, "640x480x72 hporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[1].vporch != pHV[1].vporch) {
		pHV[1].vporch = pNewHV[1].vporch;
		sprintf(temp, "%d", pHV[1].vporch);
		ret = ConfigSetKey(config_file, "640x480x72", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "640x480x72 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[2].hporch != pHV[2].hporch) {
		pHV[2].hporch = pNewHV[2].hporch;
		sprintf(temp, "%d", pHV[2].hporch);
		ret = ConfigSetKey(config_file, "640x480x75", "hporch", temp);	// 2

		if(ret != 0) {
			DEBUG(DL_ERROR, "640x480x75 hporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[2].vporch != pHV[2].vporch) {
		pHV[2].vporch = pNewHV[2].vporch;
		sprintf(temp, "%d", pHV[2].vporch);
		ret = ConfigSetKey(config_file, "640x480x75", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "640x480x75 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[3].hporch != pHV[3].hporch) {
		pHV[3].hporch = pNewHV[3].hporch;
		sprintf(temp, "%d", pHV[3].hporch);
		ret = ConfigSetKey(config_file, "640x480x85", "hporch", temp);	// 3

		if(ret != 0) {
			DEBUG(DL_ERROR, "640x480x75 hporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[3].vporch != pHV[3].vporch) {
		pHV[3].vporch = pNewHV[3].vporch;
		sprintf(temp, "%d", pHV[3].vporch);
		ret = ConfigSetKey(config_file, "640x480x85", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "640x480x85 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[4].hporch != pHV[4].hporch) {
		pHV[4].hporch = pNewHV[4].hporch;
		sprintf(temp, "%d", pHV[4].hporch);
		ret = ConfigSetKey(config_file, "800x600x60", "hporch", temp);	// 4

		if(ret != 0) {
			DEBUG(DL_ERROR, "800x600x60 hporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[4].vporch != pHV[4].vporch) {
		pHV[4].vporch = pNewHV[4].vporch;
		sprintf(temp, "%d", pHV[4].vporch);
		ret = ConfigSetKey(config_file, "800x600x60", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "800x600x60 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[5].hporch != pHV[5].hporch) {
		pHV[5].hporch = pNewHV[5].hporch;
		sprintf(temp, "%d", pHV[5].hporch);
		ret = ConfigSetKey(config_file, "800x600x72", "hporch", temp);	// 5

		if(ret != 0) {
			DEBUG(DL_ERROR, "800x600x72 hporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[5].vporch != pHV[5].vporch) {
		pHV[5].vporch = pNewHV[5].vporch;
		sprintf(temp, "%d", pHV[5].vporch);
		ret = ConfigSetKey(config_file, "800x600x72", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "800x600x72 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[6].hporch != pHV[6].hporch) {
		pHV[6].hporch = pNewHV[6].hporch;
		sprintf(temp, "%d", pHV[6].hporch);
		ret = ConfigSetKey(config_file, "800x600x75", "hporch", temp);	// 6

		if(ret != 0) {
			DEBUG(DL_ERROR, "800x600x75 hporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[6].vporch != pHV[6].vporch) {
		pHV[6].vporch = pNewHV[6].vporch;
		sprintf(temp, "%d", pHV[6].vporch);
		ret = ConfigSetKey(config_file, "800x600x75", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "800x600x75 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[7].hporch != pHV[7].hporch) {
		pHV[7].hporch = pNewHV[7].hporch;
		sprintf(temp, "%d", pHV[7].hporch);
		ret = ConfigSetKey(config_file, "800x600x85", "hporch", temp);	// 7

		if(ret != 0) {
			DEBUG(DL_ERROR, "800x600x85 hporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[7].vporch != pHV[7].vporch) {
		pHV[7].vporch = pNewHV[7].vporch;
		sprintf(temp, "%d", pHV[7].vporch);
		ret = ConfigSetKey(config_file, "800x600x85", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "800x600x85 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[8].hporch != pHV[8].hporch) {
		pHV[8].hporch = pNewHV[8].hporch;
		sprintf(temp, "%d", pHV[8].hporch);
		ret = ConfigSetKey(config_file, "1024x768x60", "hporch", temp);	// 8

		if(ret != 0) {
			DEBUG(DL_ERROR, "1024x768x60 hporch!!\n");
			goto EXIT;
		}

		DEBUG(DL_DEBUG, "1024x768 change hporch!!! = %s\n", temp);
	}

	if(pNewHV[8].vporch != pHV[8].vporch) {
		pHV[8].vporch = pNewHV[8].vporch;
		sprintf(temp, "%d", pHV[8].vporch);
		ret = ConfigSetKey(config_file, "1024x768x60", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "1024x768x60 hporch!!\n");
			goto EXIT;
		}

		DEBUG(DL_DEBUG, "1024x768 change vporch!!! = %s\n", temp);
	}

	if(pNewHV[9].hporch != pHV[9].hporch) {
		pHV[9].hporch = pNewHV[9].hporch;
		sprintf(temp, "%d", pHV[9].hporch);
		ret = ConfigSetKey(config_file, "1024x768x70", "hporch", temp);	 // 9

		if(ret != 0) {
			DEBUG(DL_ERROR, "1024x768x70 hporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[9].vporch != pHV[9].vporch) {
		pHV[9].vporch = pNewHV[9].vporch;
		sprintf(temp, "%d", pHV[9].vporch);
		ret = ConfigSetKey(config_file, "1024x768x70", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "1024x768x70 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[10].hporch != pHV[10].hporch) {
		pHV[10].hporch = pNewHV[10].hporch;
		sprintf(temp, "%d", pHV[10].hporch);
		ret = ConfigSetKey(config_file, "1024x768x75", "hporch", temp);	// 10

		if(ret != 0) {
			DEBUG(DL_ERROR, "1024x768x70 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[10].vporch != pHV[10].vporch) {
		pHV[10].vporch = pNewHV[10].vporch;
		sprintf(temp, "%d", pHV[10].vporch);
		ret = ConfigSetKey(config_file, "1024x768x75", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "1024x768x75 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[11].hporch != pHV[11].hporch) {
		pHV[11].hporch = pNewHV[11].hporch;
		sprintf(temp, "%d", pHV[11].hporch);
		ret = ConfigSetKey(config_file, "1024x768x85", "hporch", temp);	// 11

		if(ret != 0) {
			DEBUG(DL_ERROR, "1024x768x85 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[11].vporch != pHV[11].vporch) {
		pHV[11].vporch = pNewHV[11].vporch;
		sprintf(temp, "%d", pHV[11].vporch);
		ret = ConfigSetKey(config_file, "1024x768x85", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "1024x768x85 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[12].hporch != pHV[12].hporch) {
		pHV[12].hporch = pNewHV[12].hporch;
		sprintf(temp, "%d", pHV[12].hporch);
		ret = ConfigSetKey(config_file, "1280x768x60", "hporch", temp);	// 12

		if(ret != 0) {
			DEBUG(DL_ERROR, "1024x768x85 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[12].vporch != pHV[12].vporch) {
		pHV[12].vporch = pNewHV[12].vporch;
		sprintf(temp, "%d", pHV[12].vporch);
		ret = ConfigSetKey(config_file, "1280x768x60", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "1024x768x85 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[13].hporch != pHV[13].hporch) {
		pHV[13].hporch = pNewHV[13].hporch;
		sprintf(temp, "%d", pHV[13].hporch);
		ret = ConfigSetKey(config_file, "1280x800x60", "hporch", temp);	// 13

		if(ret != 0) {
			DEBUG(DL_ERROR, "1280x800x60 hporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[13].vporch != pHV[13].vporch) {
		pHV[13].vporch = pNewHV[13].vporch;
		sprintf(temp, "%d", pHV[13].vporch);
		ret = ConfigSetKey(config_file, "1280x800x60", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "1280x800x60 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[14].hporch != pHV[14].hporch) {
		pHV[14].hporch = pNewHV[14].hporch;
		sprintf(temp, "%d", pHV[14].hporch);
		ret = ConfigSetKey(config_file, "1280x960x60", "hporch", temp);	// 14

		if(ret != 0) {
			DEBUG(DL_ERROR, "1280x960x60 hporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[14].vporch != pHV[14].vporch) {
		pHV[14].vporch = pNewHV[14].vporch;
		sprintf(temp, "%d", pHV[14].vporch);
		ret = ConfigSetKey(config_file, "1280x960x60", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "1280x960x60 hporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[15].hporch != pHV[15].hporch) {
		pHV[15].hporch = pNewHV[15].hporch;
		sprintf(temp, "%d", pHV[15].hporch);
		ret = ConfigSetKey(config_file, "1440x900x60", "hporch", temp);	// 15

		if(ret != 0) {
			DEBUG(DL_ERROR, "1440x900x60 hporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[15].vporch != pHV[15].vporch) {
		pHV[15].vporch = pNewHV[15].vporch;
		sprintf(temp, "%d", pHV[15].vporch);
		ret = ConfigSetKey(config_file, "1440x900x60", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "1440x900x60 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[16].hporch != pHV[16].hporch) {
		pHV[16].hporch = pNewHV[16].hporch;
		sprintf(temp, "%d", pHV[16].hporch);
		ret = ConfigSetKey(config_file, "1400x1050x60", "hporch", temp);	// 16

		if(ret != 0) {
			DEBUG(DL_ERROR, "1400x1050x60 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[16].vporch != pHV[16].vporch) {
		pHV[16].vporch = pNewHV[16].vporch;
		sprintf(temp, "%d", pHV[16].vporch);
		ret = ConfigSetKey(config_file, "1400x1050x60", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "1400x1050x60 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[17].hporch != pHV[17].hporch) {
		pHV[17].hporch = pNewHV[17].hporch;
		sprintf(temp, "%d", pHV[17].hporch);
		ret = ConfigSetKey(config_file, "1280x1024x60", "hporch", temp); // 17

		if(ret != 0) {
			DEBUG(DL_ERROR, "1280x1024x60 hporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[17].vporch != pHV[17].vporch) {
		pHV[17].vporch = pNewHV[17].vporch;
		sprintf(temp, "%d", pHV[17].vporch);
		ret = ConfigSetKey(config_file, "1280x1024x60", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "1280x1024x60 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[18].hporch != pHV[18].hporch) {
		pHV[18].hporch = pNewHV[18].hporch;
		sprintf(temp, "%d", pHV[18].hporch);
		ret = ConfigSetKey(config_file, "1280x1024x75", "hporch", temp); // 18

		if(ret != 0) {
			DEBUG(DL_ERROR, "1280x1024x75 hporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[18].vporch != pHV[18].vporch) {
		pHV[18].vporch = pNewHV[18].vporch;
		sprintf(temp, "%d", pHV[18].vporch);
		ret = ConfigSetKey(config_file, "1280x1024x75", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "1280x1024x75 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[19].hporch != pHV[19].hporch) {
		pHV[19].hporch = pNewHV[19].hporch;
		sprintf(temp, "%d", pHV[19].hporch);
		ret = ConfigSetKey(config_file, "1600x1200x60", "hporch", temp); // 19

		if(ret != 0) {
			DEBUG(DL_ERROR, "1600x1200x60 hporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[19].vporch != pHV[19].vporch) {
		pHV[19].vporch = pNewHV[19].vporch;
		sprintf(temp, "%d", pHV[19].vporch);
		ret = ConfigSetKey(config_file, "1600x1200x60", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "1600x1200x60 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[20].hporch != pHV[20].hporch) {
		pHV[20].hporch = pNewHV[20].hporch;
		sprintf(temp, "%d", pHV[20].hporch);
		ret = ConfigSetKey(config_file, "1280x720V60", "hporch", temp);	 // 20

		if(ret != 0) {
			DEBUG(DL_ERROR, "1280x720V60 hporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[20].vporch != pHV[20].vporch) {
		pHV[20].vporch = pNewHV[20].vporch;
		sprintf(temp, "%d", pHV[20].vporch);
		ret = ConfigSetKey(config_file, "1280x720V60", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "1280x720V60 hporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[21].hporch != pHV[21].hporch) {
		pHV[21].hporch = pNewHV[21].hporch;
		sprintf(temp, "%d", pHV[21].hporch);
		ret = ConfigSetKey(config_file, "1280x720P50", "hporch", temp);	// 21

		if(ret != 0) {
			DEBUG(DL_ERROR, "1280x720P50 hporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[21].vporch != pHV[21].vporch) {
		pHV[21].vporch = pNewHV[21].vporch;
		sprintf(temp, "%d", pHV[21].vporch);
		ret = ConfigSetKey(config_file, "1280x720P50", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "1280x720P50 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[22].hporch != pHV[22].hporch) {
		pHV[22].hporch = pNewHV[22].hporch;
		sprintf(temp, "%d", pHV[22].hporch);
		ret = ConfigSetKey(config_file, "1280x720P60", "hporch", temp);	// 22

		if(ret != 0) {
			DEBUG(DL_ERROR, "1280x720P60 hporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[22].vporch != pHV[22].vporch) {
		pHV[22].vporch = pNewHV[22].vporch;
		sprintf(temp, "%d", pHV[22].vporch);
		ret = ConfigSetKey(config_file, "1280x720P60", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "1280x720P60 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[23].hporch != pHV[23].hporch) {
		pHV[23].hporch = pNewHV[23].hporch;
		sprintf(temp, "%d", pHV[23].hporch);
		ret = ConfigSetKey(config_file, "1920x1080I50", "hporch", temp); // 23

		if(ret != 0) {
			DEBUG(DL_ERROR, "1920x1080I50 hporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[23].vporch != pHV[23].vporch) {
		pHV[23].vporch = pNewHV[23].vporch;
		sprintf(temp, "%d", pHV[23].vporch);
		ret = ConfigSetKey(config_file, "1920x1080I50", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "1920x1080I50 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[24].hporch != pHV[24].hporch) {
		pHV[24].hporch = pNewHV[24].hporch;
		sprintf(temp, "%d", pHV[24].hporch);
		ret = ConfigSetKey(config_file, "1920x1080I60", "hporch", temp); // 24

		if(ret != 0) {
			DEBUG(DL_ERROR, "1920x1080I60 hporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[24].vporch != pHV[24].vporch) {
		pHV[24].vporch = pNewHV[24].vporch;
		sprintf(temp, "%d", pHV[24].vporch);
		ret = ConfigSetKey(config_file, "1920x1080I60", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "1920x1080I60 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[25].hporch != pHV[25].hporch) {
		pHV[25].hporch = pNewHV[25].hporch;
		sprintf(temp, "%d", pHV[25].hporch);
		ret = ConfigSetKey(config_file, "1920x1080P25", "hporch", temp); // 25

		if(ret != 0) {
			DEBUG(DL_ERROR, "1920x1080P25 hporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[25].vporch != pHV[25].vporch) {
		pHV[25].vporch = pNewHV[25].vporch;
		sprintf(temp, "%d", pHV[25].vporch);
		ret = ConfigSetKey(config_file, "1920x1080P25", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "1920x1080P25 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[26].hporch != pHV[26].hporch) {
		pHV[26].hporch = pNewHV[26].hporch;
		sprintf(temp, "%d", pHV[26].hporch);
		ret = ConfigSetKey(config_file, "1920x1080P30", "hporch", temp); // 26

		if(ret != 0) {
			DEBUG(DL_ERROR, "1920x1080P30 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[26].vporch != pHV[26].vporch) {
		pHV[26].vporch = pNewHV[26].vporch;
		sprintf(temp, "%d", pHV[26].vporch);
		ret = ConfigSetKey(config_file, "1920x1080P30", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "1920x1080P30 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[27].hporch != pHV[27].hporch) {
		pHV[27].hporch = pNewHV[27].hporch;
		sprintf(temp, "%d", pHV[27].hporch);
		ret = ConfigSetKey(config_file, "1920x1080Px50", "hporch", temp); // 27

		if(ret != 0) {
			DEBUG(DL_ERROR, "1920x1080Px60 hporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[27].vporch != pHV[27].vporch) {
		pHV[27].vporch = pNewHV[27].vporch;
		sprintf(temp, "%d", pHV[27].vporch);
		ret = ConfigSetKey(config_file, "1920x1080Px50", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "1920x1080Px60 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[28].hporch != pHV[28].hporch) {
		pHV[28].hporch = pNewHV[28].hporch;
		sprintf(temp, "%d", pHV[28].hporch);
		ret = ConfigSetKey(config_file, "1920x1080Px60", "hporch", temp); // 28

		if(ret != 0) {
			DEBUG(DL_ERROR, "1920x1080Px50 hporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[28].vporch != pHV[28].vporch) {
		pHV[28].vporch = pNewHV[28].vporch;
		sprintf(temp, "%d", pHV[28].vporch);
		ret = ConfigSetKey(config_file, "1920x1080Px60", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "1920x1080Px50 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[29].hporch != pHV[29].hporch) {
		pHV[29].hporch = pNewHV[29].hporch;
		sprintf(temp, "%d", pHV[29].hporch);
		ret = ConfigSetKey(config_file, "720x480I60", "hporch", temp);	 // 29

		if(ret != 0) {
			DEBUG(DL_ERROR, "720x480I60 hporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[29].vporch != pHV[29].vporch) {
		pHV[29].vporch = pNewHV[29].vporch;
		sprintf(temp, "%d", pHV[29].vporch);
		ret = ConfigSetKey(config_file, "720x480I60", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "720x480I60 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[30].hporch != pHV[30].hporch) {
		pHV[30].hporch = pNewHV[30].hporch;
		sprintf(temp, "%d", pHV[30].hporch);
		ret = ConfigSetKey(config_file, "720x576I50", "hporch", temp);	// 30

		if(ret != 0) {
			DEBUG(DL_ERROR, "720x576I50 hporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[30].vporch != pHV[30].vporch) {
		pHV[30].vporch = pNewHV[30].vporch;
		sprintf(temp, "%d", pHV[30].vporch);
		ret = ConfigSetKey(config_file, "720x576I50", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "720x576I50 vporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[MODE_MAX].hporch != pHV[MODE_MAX].hporch) {
		pHV[MODE_MAX].hporch = pNewHV[MODE_MAX].hporch;
		sprintf(temp, "%d", pHV[MODE_MAX].hporch);
		ret = ConfigSetKey(config_file, "1366x768x60", "hporch", temp);	// 33

		if(ret != 0) {
			DEBUG(DL_ERROR, "1366x768x60 hporch!!\n");
			goto EXIT;
		}
	}

	if(pNewHV[MODE_MAX].vporch != pHV[MODE_MAX].vporch) {
		pHV[MODE_MAX].vporch = pNewHV[MODE_MAX].vporch;
		sprintf(temp, "%d", pHV[MODE_MAX].vporch);
		ret = ConfigSetKey(config_file, "1366x768x60", "vporch", temp);

		if(ret != 0) {
			DEBUG(DL_ERROR, "1366x768x60 vporch!!\n");
			goto EXIT;
		}
	}

EXIT:
	pthread_mutex_unlock(&gSetP_m.save_sys_m);
	DEBUG(DL_DEBUG, "save HV table succeed!!\n");
	return 0;

}


/*compare beyond HV table*/
int BeyondHVTable(int mode, int digital, short hporch, short vporch, HVTable *pp)
{
	if(mode > MODE_MAX) {
		return -1;
	}

	if(digital) {
		pp->digital[mode].hporch = hporch;
		pp->digital[mode].vporch = vporch;
	} else {
		pp->analog[mode].hporch = hporch;
		pp->analog[mode].vporch = vporch;
	}

	return 0;
}

/*Read Remote Protocol*/
int ReadRemoteCtrlIndex(char *config_file, int *index)
{
	char 			temp[512];
	int 			ret  = -1 ;

	pthread_mutex_lock(&gSetP_m.save_sys_m);
	/*remote control index*/
	ret =  ConfigGetKey(config_file, "remote", "index", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Failed to Get remote index\n");
		goto EXIT;
	}

	*index = atoi(temp);
EXIT:
	pthread_mutex_unlock(&gSetP_m.save_sys_m);
	return 0;
}

/*save remote control index*/
int SaveRemoteCtrlIndex(int sock, int index, int length)
{
	char temp[200] = {0};
	char filename[15];
	int ret = 0 , save_index = 0;

	if(0 == length) {
		PackHeaderMSG(temp, MSG_CAMERACTRL_PROTOCOL, (HEAD_LEN + 1));
		temp[HEAD_LEN] = gblGetRemoteIndex();
		ret = WriteData(sock, temp, (1 + HEAD_LEN));

		if(ret < 0) {
			DEBUG(DL_ERROR, "SendBroadCastMsg()return:%d socket:%d ERROR:%d\n", ret, sock, errno);
		}

		return 0;
	}

	DEBUG(DL_DEBUG, "Save Remote Control Index = %d \n", index);

	//if index == remoteindex ,i didn't need reboot the system
	if(index < 0 || index >= MAX_FAR_CTRL_NUM || index == gblGetRemoteIndex()) {
		return -1;
	}

	strcpy(filename, REMOTE_NAME);
RESTART:
	pthread_mutex_lock(&gSetP_m.save_sys_m);
	/*index*/
	sprintf(temp, "%d", index);
	ret =  ConfigSetKey(filename, "remote", "index", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "set remote index failed\n");
		pthread_mutex_unlock(&gSetP_m.save_sys_m);
		return -1;
	}

	pthread_mutex_unlock(&gSetP_m.save_sys_m);
	ReadRemoteCtrlIndex(filename, &save_index);

	if(index == save_index) {
		DEBUG(DL_DEBUG, "Save Index Ok save_index = %d\n", save_index);
		return 1;
	} else {
		DEBUG(DL_DEBUG, "save Index Failed index = %d \n", index);
		goto RESTART;
	}

	return 1;

}

/*save remote control index*/
int SaveLocalRemoteCtrlIndex(void)
{
	char temp[256];
	char filename[512];
	int ret ;

	strcpy(filename, REMOTE_NAME);
	pthread_mutex_lock(&gSetP_m.save_sys_m);
	/*index*/
	sprintf(temp, "%d", gblGetRemoteIndex());
	ret =  ConfigSetKey(filename, "remote", "index", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "set remote index failed\n");
		goto EXIT;
	}

EXIT:
	pthread_mutex_unlock(&gSetP_m.save_sys_m);
	return 0;

}


static  unsigned int g_IframeInterval[MAX_VAL] = {300, 300};
int app_get_video_gop(int high)
{
	if(high != HIGH_VIDEO && high != LOW_VIDEO) {
		high = HIGH_VIDEO;
	}

	return g_IframeInterval[high];
}

void app_set_video_gop(int high , int value)
{
	if(high != HIGH_VIDEO && high != LOW_VIDEO) {
		return;
	}

	if(value < 0 || value > 1000) {
		value = 1000;
	}

	g_IframeInterval[high] = value;
	return ;

}



/*读 I 帧间隔*/
int ReadIframeInterval(char *config_file)
{
	int 			ret			 = 0;
	char 			temp[512];

	pthread_mutex_lock(&gSetP_m.save_sys_m);
	/*MAC Address*/
	DEBUG(DL_DEBUG, "read config = %s \n", config_file);
	ret =  ConfigGetKey(config_file, "param", "high", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get I frames high Interval failed\n");
		//g_IframeInterval[HIGH_VIDEO] = 300;
		app_set_video_gop(HIGH_VIDEO, 300);
		goto EXIT;
	}

	app_set_video_gop(HIGH_VIDEO, atol(temp));
	//g_IframeInterval[HIGH_VIDEO] =  atol(temp);
	DEBUG(DL_DEBUG, "I high interval = %d\n", atol(temp));
	ret =  ConfigGetKey(config_file, "param", "low", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get I frames low Interval failed\n");
		//g_IframeInterval[LOW_VIDEO] = 300;
		app_set_video_gop(LOW_VIDEO, 300);
		goto EXIT;
	}

	app_set_video_gop(LOW_VIDEO, atol(temp));
	//g_IframeInterval[LOW_VIDEO] =  atol(temp);
	DEBUG(DL_DEBUG, "I low interval = %d\n", atol(temp));
EXIT:
	pthread_mutex_unlock(&gSetP_m.save_sys_m);
	return 0;
}


int WriteIfreameInterval(char *config_file)
{
	int ret = 0;
	int value = 0;
	char temp[256] = {0};

	value = app_get_video_gop(HIGH_VIDEO);
	snprintf(temp, sizeof(temp), "%d", value);
	ret =  ConfigSetKey(config_file, "param", "high", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Set Key Rvolume failed\n");
	}

	value = app_get_video_gop(LOW_VIDEO);
	snprintf(temp, sizeof(temp), "%d", value);
	ret =  ConfigSetKey(config_file, "param", "low", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Set Key Rvolume failed\n");
	}

	return 0;
}


static char  g_local_ip[16] = {0};
int app_get_local_ip(char *ip)
{
	struct in_addr 		addr1;

	if(strlen(g_local_ip) == 0) {
		memcpy(&addr1, &(gSysParaT.sysPara.dwAddr), 4);
		memcpy(g_local_ip, inet_ntoa(addr1), 16);
		PRINTF("g_local_ip=%s\n", g_local_ip);

	}

	strcpy(ip, g_local_ip);
	return 0;
}


static unsigned long long g_start_time = 0;
unsigned long long app_get_start_time()
{
	if(g_start_time == 0)
		g_start_time = getCurrentTime();
	return g_start_time;
} 
int app_set_start_time()
{
	g_start_time = getCurrentTime();
	return 0;
}


