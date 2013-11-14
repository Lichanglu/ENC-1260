#include <stdio.h>


#include "common.h"
#include "middle_control.h"
#include "log_common.h"
#include "sysparam.h"
#include "mid_task.h"
#include "app_video_output.h"

#define MAX_BITRATA		(1024 * 20)
#define MIN_BITRATA		(128)

static OutputVideoInfo g_outputparam;
extern  PLock  	gSetP_m;
extern  ResizeParam gResizeP;
extern  int capture_width;
extern  int  capture_height;
//extern unsigned int g_IframeInterval[MAX_VAL];
extern SavePTable  gSysParaT;
extern VAENC_params gVAenc_p;

static int checkOutputVideoinfo(OutputVideoInfo *pnew);
static int beyondOutputParam(OutputVideoInfo *pNew, OutputVideoInfo *pOld, int *ret_value);

int initOutputVideoParam(void)
{
	memset(&g_outputparam, 0, sizeof(g_outputparam));
	g_outputparam.resolution = 0;
	g_outputparam.resizeMode = 1;
	g_outputparam.encodelevel = 1;
	g_outputparam.preset = 3;
	g_outputparam.nFrameRate = 30;
	g_outputparam.IFrameinterval = 150;
	g_outputparam.sBitrate = 3000;
	return 0;
}

void getOutputvideohandle(OutputVideoInfo *out)
{
	memcpy(out, &g_outputparam, sizeof(OutputVideoInfo));
	return ;
}

void setOutputvideohandle(OutputVideoInfo *out)
{
	memcpy(&g_outputparam, out, sizeof(OutputVideoInfo));
	return ;
}



int DoLockResolution(int in, int *out)
{
	int resizemode = 0, tmp = 0;

	if(in <= LOCK_BEGIN || in >= LOCK_MAX) {
		in = LOCK_BEGIN;
	}

	getResizeMode(tmp, &resizemode);
	calcLockResolution(in);
	PRINTF("DoLockResolution resizemode = %d,enc_w:%d,enc_h:%d\n", resizemode, gResizeP.enc_w, gResizeP.enc_h);
	CalcReaolution(gResizeP.enc_w, gResizeP.enc_h, capture_width, capture_height, 1, resizemode);
	*out = in;

	if(g_outputparam.resolution != in) {
		g_outputparam.resolution = in;
		mid_timer_create(3, 1, (mid_func_t)writeOutputVideoParam);
	}

	return 0;
}
int webgetOutputResolution(int in, int *out)
{

	*out = g_outputparam.resolution;

	return 0;
}


int getOutputResolution(void)
{
	return g_outputparam.resolution;
}

void setOutputResolution(int val)
{
	g_outputparam.resolution = val;
}

int getResizeMode(int in, int *out)
{
	*out = g_outputparam.resizeMode;
	return 0;
}
int setResizeMode(int in , int *out)
{
	PRINTF("setResizeMode enter ....: %d\n", in);

	if(in < 0 || in > 1) {
		g_outputparam.resizeMode = 0;
		*out = 0;
		return -1;
	}

	if(g_outputparam.resizeMode != in) {
		g_outputparam.resizeMode = in;
		mid_timer_create(3, 1, (mid_func_t)writeOutputVideoParam);
	}

	*out = in;
	PRINTF("setResizeMode leave ....: %d\n", in);
	return 0;
}

int getEncodelevel(int in , int *out)
{
	*out = g_outputparam.encodelevel;
	return 0;
}

int setEncodelevel(int in, int *out)
{
	PRINTF("outputparam.encodelevel =%d, in = %d\n", g_outputparam.encodelevel, in);

	if(g_outputparam.encodelevel != in) {
		g_outputparam.encodelevel = in;
		mid_timer_create(3, 1, (mid_func_t)writeOutputVideoParam);
		*out = in;
	}

	return 0;
}
int setSceneconfig(int in, int *out)
{
	if(in < 0 || in > 3) {
		in = 0;
		g_outputparam.preset = in;
		return -1;
	}

	if(g_outputparam.preset != in) {
		g_outputparam.preset = in;
		mid_timer_create(3, 1, (mid_func_t)writeOutputVideoParam);
		*out = in;
	}

	return 0;
}

int getSceneconfig(int data, int *out)
{
	*out = g_outputparam.preset;
	return 0;
}
int getFrameRate(int data, int *out)
{
	*out = g_outputparam.nFrameRate;
	return 0;
}

int setFrameRate(int data, int *out)
{
	int status = 0;

	if(data < 0) {
		data = 0;
		status = -1;
		goto EXIT;
	}

	if(gblGetMode() == VIDEO_720P60_PIXEL) {
		if(60 <= data) {
			data = 60;
			status = -1;
			goto EXIT;
		}
	} else if(gblGetMode() == VIDEO_720P50_PIXEL) {
		if(50 <= data) {
			data = 50;
			status = -1;
			goto EXIT;
		}
	} else {
		if(30 <=  data) {
			data = 30;
			status = -1;
			goto EXIT;
		}

	}

EXIT:
	gSysParaT.videoPara[0].nFrameRate = data;

	if(g_outputparam.nFrameRate != data) {
		g_outputparam.nFrameRate = data;
		mid_timer_create(3, 1, (mid_func_t)writeOutputVideoParam);
	}

	*out = data;
	return status;
}
int getIFrameInterval(int data, int *out)
{
	*out = g_outputparam.IFrameinterval;
	return 0;
}

#if 0
int setIFrameInterval(int data, int *out)
{
	int ret = 0;

	if(data > 300) {
		data = 300;
		*out = data;
		ret  = -1;
	}

	if(data <= 0) {
		data = 0;
		*out = data;
		ret = -1;
	}

	g_IframeInterval[HIGH_VIDEO] = data;
	*out = data;

	if(g_outputparam.IFrameinterval != data) {
		PRINTF("outputparam.IFrameinterval =%d\n", g_outputparam.IFrameinterval);
		g_outputparam.IFrameinterval = data;
		mid_timer_create(3, 1, (mid_func_t)writeOutputVideoParam);
	}

	return ret;
}
#endif

int getBitRate(int data, int *out)
{
	*out = (g_outputparam.sBitrate);
	return 0;
}

int setBitRate(int data, int *out)
{
	PRINTF("MSG_SETBITRATE setBitRate data =%d\n", data);
	int ret = 0;

	if(data >= MAX_BITRATA) {
		data = MAX_BITRATA;
		ret = -1;
	}

	if(data <= MIN_BITRATA) {
		data = MIN_BITRATA;
		ret = -1;
	}

	PRINTF("MSG_SETBITRATE setBitRate data =%d after.....\n", data);
	//gSysParaT.videoPara[0].sBitrate = data;
	*out = data;
	g_outputparam.sBitrate = data;
	PRINTF("MSG_SETBITRATE setBitRate outputparam.sBitrate =%d,data=%d\n", g_outputparam.sBitrate, data);
	mid_timer_create(3, 1, (mid_func_t)writeOutputVideoParam);
	PRINTF("MSG_SETBITRATE setBitRate data =%d\n", *out);
	return ret;
}

//void setlogotextflag(int flag)
//{
//	g_outputparam.logotext = flag;
//}

//return 1 表示需要重建
/*beyong video param*/
static int beyondOutputParam(OutputVideoInfo *pNew, OutputVideoInfo *pOld, int *ret_value)
{
	int width_change = 0;
	int lock_flag = 0;
	int tmp = 0;
	int resizemode;
#if 1

	if(pNew->resolution != pOld->resolution) {
		pOld->resolution = pNew->resolution;
		lock_flag  = 1;
		width_change = 1;
		PRINTF("outputparam resolution change :%d\n", pNew->resolution);

	}

	if(pNew->resizeMode != pOld->resizeMode) {
		pOld->resizeMode = pNew->resizeMode;
		lock_flag = 1;
		width_change = 1;
		PRINTF("outputparam resizeMode change :%d\n", pNew->resizeMode);
	}

#endif

	if(pNew->encodelevel != pOld->encodelevel) {
		pOld->encodelevel = pNew->encodelevel;
		*ret_value = 1;
		width_change = 1;
		PRINTF("outputparam encodelevel change :%d\n", pNew->encodelevel);
	}

	if(pNew->preset != pOld->preset) {
		pOld->preset = pNew->preset;
		*ret_value = 1;
		width_change = 1;
		PRINTF("outputparam flag change :%d\n", pNew->preset);
	}

	if(pNew->nFrameRate != pOld->nFrameRate) {
		pOld->nFrameRate = pNew->nFrameRate;
		width_change = 1;
		PRINTF("outputparam nFrameRate change :%d\n", pNew->nFrameRate);
	}

	if(pNew->sBitrate != pOld->sBitrate) {
		pOld->sBitrate = pNew->sBitrate;
		width_change = 1;
		PRINTF("outputparam sBitrate change :%d\n", pNew->sBitrate);
	}

	if((pNew->IFrameinterval != pOld->IFrameinterval)) {
		pOld->IFrameinterval = pNew->IFrameinterval;
		*ret_value = 2;
		width_change = 1;
		PRINTF("outputparam IFrameinterval change :%d\n", pNew->IFrameinterval);
	}

	//	if((pNew->logotext != pOld->logotext)) {
	//		pOld->logotext = pNew->logotext;
	//		setShowLogoTextFlag(pOld->logotext);
	//		width_change = 1;
	//		PRINTF("outputparam logotext change :%d\n", pNew->logotext);
	//	}

	if((pNew->logo_show != pOld->logo_show)) {
		pOld->logo_show = pNew->logo_show;
		app_set_logoshow_flag(pOld->logo_show);
		width_change = 1;
		PRINTF("outputparam logo_show change :%d\n", pNew->logo_show);
	}

	if((pNew->text_show != pOld->text_show)) {
		pOld->text_show = pNew->text_show;
		app_set_textshow_flag(pOld->text_show);
		width_change = 1;
		PRINTF("outputparam text_show change :%d\n", pNew->text_show);
	}



	if(lock_flag) {
		calcLockResolution(pOld->resolution);
		CalcReaolution(gResizeP.enc_w, gResizeP.enc_h, capture_width, capture_height, 1, pOld->resizeMode);
	}

	return (width_change);
}

/*read param table from file enc1200*/
int readOutputVideoParam(char *config_file, OutputVideoInfo *Param)
{
	char 			temp[512] = {0};
	int 			ret  = -1 ;
	unsigned int 	value;
	unsigned int 	dwAddr, dwNetmask, dwGateWay;


	pthread_mutex_lock(&gSetP_m.save_sys_m);
	PRINTF("read config = %s \n", config_file);
	ret =  ConfigGetKey(config_file, "video", "resolution", temp);

	if(ret != 0) {
		PRINTF("Get Output video resolutoin failed\n");
		goto EXIT;
	}

	Param->resolution = atoi(temp);

	ret =  ConfigGetKey(config_file, "video", "resizemode", temp);

	if(ret != 0) {
		PRINTF("Get Key ipaddr failed\n");
		goto EXIT;
	}


	Param->resizeMode = atoi(temp);

	PRINTF("Get Output video resoluse=%d,the resizemode =%d\n", Param->resolution, Param->resizeMode);


	ret =  ConfigGetKey(config_file, "video", "encodelv", temp);

	if(ret != 0) {
		PRINTF("Get Key gateway failed\n");
		goto EXIT;
	}

	Param->encodelevel = atoi(temp);

	ret =  ConfigGetKey(config_file, "video", "scene", temp);

	if(ret != 0) {
		PRINTF("Get Key netmask failed\n");
		goto EXIT;
	}

	Param->preset = atoi(temp);

	/*Name*/
	ret =  ConfigGetKey(config_file, "video", "frame", temp);

	if(ret != 0) {
		PRINTF("Get Key name failed\n");
		goto EXIT;
	}

	Param->nFrameRate = atoi(temp);

	/*Version*/
	ret =  ConfigGetKey(config_file, "video", "Iframeinterval", temp);

	if(ret != 0) {
		PRINTF("Get Key version failed\n");
		goto EXIT;
	}

	Param->IFrameinterval = atoi(temp);

	/*Channels*/
	ret =  ConfigGetKey(config_file, "video", "bitrate", temp);

	if(ret != 0) {
		PRINTF("Get Key channels failed\n");
		goto EXIT;
	}

	PRINTF("read config = %s sBitrate=%d\n", config_file, atoi(temp));
	Param->sBitrate = atoi(temp);


	/*type*/
	//	ret =  ConfigGetKey(config_file, "video", "logotext", temp);

	//	if(ret != 0) {
	//		PRINTF("Get Key type failed\n");
	//		goto EXIT;
	//	}

	//	Param->logotext = atoi(temp);

	ret =  ConfigGetKey(config_file, "video", "logo_show", temp);

	if(ret != 0) {
		PRINTF("Get Key type failed\n");
		goto EXIT;
	}

	Param->logo_show = atoi(temp);

	ret =  ConfigGetKey(config_file, "video", "text_show", temp);

	if(ret != 0) {
		PRINTF("Get Key type failed\n");
		goto EXIT;
	}

	Param->text_show = atoi(temp);


EXIT:
	pthread_mutex_unlock(&gSetP_m.save_sys_m);
	return 0;
}


int writeOutputVideoParam(void)
{
	//	return 0;
	PRINTF("begin \n");

	int total = 0;
	int ret = 0;
	//int value = 0;
	int i = 0;
	char bak_name[512] = {0};
	char section[256] = {0};
	char cmd[512] = {0};
	char temp[64] = {0};
	OutputVideoInfo videoinfo;
	memset(&videoinfo, 0, sizeof(OutputVideoInfo));

	pthread_mutex_lock(&gSetP_m.save_sys_m);
	memcpy(&videoinfo, &g_outputparam, sizeof(OutputVideoInfo));
	pthread_mutex_unlock(&gSetP_m.save_sys_m);

	sprintf(bak_name, "%s.bak", VIDEOENCODE_FILE);




	sprintf(temp, "%d", videoinfo.resolution);

	ret = ConfigSetKey(bak_name, "video", "resolution", temp);

	if(ret != 0) {
		PRINTF("write resolution failed resolution:%d\n", videoinfo.resolution);
		goto EXIT;
	}

	PRINTF("write outputparam.resizeMode %d before ....\n", videoinfo.resizeMode);
	sprintf(temp, "%d", videoinfo.resizeMode);
	ret = ConfigSetKey(bak_name, "video", "resizemode", temp);
	PRINTF("write outputparam.resizeMode after ....\n");

	if(ret != 0) {
		PRINTF("write resizeMode failed resolution:%d\n", videoinfo.resizeMode);
		goto EXIT;
	}

	sprintf(temp, "%d", videoinfo.encodelevel);
	ret = ConfigSetKey(bak_name, "video", "encodelv", temp);

	if(ret != 0) {
		PRINTF("write encodelevel failed resolution:%d\n", videoinfo.encodelevel);
		goto EXIT;
	}

	sprintf(temp, "%d", videoinfo.preset);
	ret = ConfigSetKey(bak_name, "video", "scene", temp);

	if(ret != 0) {
		PRINTF("write scene failed resolution:%d\n", videoinfo.preset);
		goto EXIT;
	}

	PRINTF("set nFramerate rize value : %d \n", videoinfo.nFrameRate);
	sprintf(temp, "%d", videoinfo.nFrameRate);
	ret = ConfigSetKey(bak_name, "video", "frame", temp);

	if(ret != 0) {
		PRINTF("write nFrameRate failed resolution:%d\n", videoinfo.nFrameRate);
		goto EXIT;
	}

	sprintf(temp, "%d", videoinfo.IFrameinterval);
	ret = ConfigSetKey(bak_name, "video", "Iframeinterval", temp);

	if(ret != 0) {
		PRINTF("write IFrameinterval failed resolution:%d\n", videoinfo.IFrameinterval);
		goto EXIT;
	}

	PRINTF("write sBitrate :%d\n", videoinfo.sBitrate);
	sprintf(temp, "%d", videoinfo.sBitrate);
	ret = ConfigSetKey(bak_name, "video", "bitrate", temp);

	if(ret != 0) {
		PRINTF("write sBitrate failed resolution:%d\n", videoinfo.sBitrate);
		goto EXIT;
	}


	//	sprintf(temp, "%d", getShowLogoTextFlag());
	//	ret = ConfigSetKey(bak_name, "video", "logotext", temp);

	//	if(ret != 0) {
	//		PRINTF("write getShowLogoTextFlag failed resolution:%d\n", getShowLogoTextFlag());
	//		goto EXIT;
	//	}

	sprintf(temp, "%d", app_get_logoshow_flag());
	ret = ConfigSetKey(bak_name, "video", "logo_show", temp);

	if(ret != 0) {
		PRINTF("write logo show failed resolution:%d\n", app_get_logoshow_flag());
		goto EXIT;
	}

	sprintf(temp, "%d", app_get_textshow_flag());
	ret = ConfigSetKey(bak_name, "video", "text_show", temp);

	if(ret != 0) {
		PRINTF("write text show failed resolution:%d\n", app_get_textshow_flag());
		goto EXIT;
	}



	/*拷贝bak文件到ini*/
	if(ret != -1) {
		sprintf(cmd, "cp -rf %s %s;rm -rf %s", bak_name, VIDEOENCODE_FILE, bak_name);
		system(cmd);
		PRINTF("i will run cmd:%s\n", cmd);
	}

EXIT:
	PRINTF("end \n");
	return ret;
}




#if 1
static int checkOutputVideoinfo(OutputVideoInfo *pnew)
{
	if(pnew->resolution < LOCK_BEGIN || pnew->resolution > LOCK_MAX) {
		pnew->resolution = LOCK_BEGIN;
	}

#if 0

	if(gblGetMode() == VIDEO_720P60_PIXEL) {
		if(60 <= pnew->nFrameRate) {
			pnew->nFrameRate = 60;
		}
	} else if(gblGetMode() == VIDEO_720P50_PIXEL) {

		if(50 <= pnew->nFrameRate) {
			pnew->nFrameRate = 50;
		}
	} else {
		if(30 <=  pnew->nFrameRate) {
			pnew->nFrameRate = 30;
		}

		if(pnew->nFrameRate <= 0) {
			pnew->nFrameRate = 0;
		}
	}
#else 
	if(30 <=  pnew->nFrameRate) {
		pnew->nFrameRate = 30;
		}

	if(pnew->nFrameRate <= 0) {
		pnew->nFrameRate = 0;
	}

#endif


	if(pnew->IFrameinterval < 0 || pnew->IFrameinterval > 1000) {
		PRINTF("IFrameinterval invalid \n", pnew->IFrameinterval);
		pnew->IFrameinterval = 150;
	}

	if(pnew->sBitrate < 128 || pnew->sBitrate > (20 * 1024)) {
		PRINTF("sBitrate invalid \n", pnew->sBitrate);
		pnew->sBitrate = 4000;
	}

}

int setOutputParame(OutputVideoInfo *pnew, OutputVideoInfo *pold)
{
	PRINTF("pnew->resolution:[%d], pnew->resizeMode:[%d], pnew->flag:[%d],pnew->encodelevel:[%d],pnew->IFrameinterval:[%d]\n",
	       pnew->resolution, pnew->resizeMode, pnew->preset, pnew->encodelevel, pnew->IFrameinterval);
	int ret_val = 0;
	int ret_recreate = 0;
	int ret_beyonparam = 0;
	int save = 0;
	VideoParam videoinfo = gSysParaT.videoPara[PORT_ONE];
	PRINTF("pnew->nFrameRate:[%d],pnew->sBitrate:[%d],videoinfo.nFrameRate:%d,videoinfo.sBitrate:%d\n",
	       pnew->nFrameRate, pnew->sBitrate, videoinfo.nFrameRate, videoinfo.sBitrate);
	checkOutputVideoinfo(pnew);
	ret_beyonparam = beyondOutputParam(pnew, pold, &ret_recreate);
	videoinfo.nFrameRate = pnew->nFrameRate;
	videoinfo.sBitrate = pnew->sBitrate;
	ret_val = SetVideoParam(0, &videoinfo, sizeof(VideoParam));


	if(ret_recreate == 1) {
		gVAenc_p.venc.uflag = H264_ENC_PARAM_FLAG;
	} else if(ret_recreate == 2) {
		//set gop
		app_set_video_gop(HIGH_VIDEO, pnew->IFrameinterval);
		WriteIfreameInterval(IFRAMES_NAME);
		ForceIframe(0);
	}

	save = ret_val | ret_beyonparam;
	PRINTF("ret_val:[%d],ret_recreate:[%d],save:[%d]\n", ret_val, ret_recreate, save);
	return save;
}


#endif


static int g_text_show = 0;
static int g_logo_show = 0;
int app_get_textshow_flag()
{
	return g_text_show;
}
int app_set_textshow_flag(int value)
{
	g_text_show = value;
	return g_text_show;
}

int app_get_logoshow_flag()
{
	return g_logo_show;
}
int app_set_logoshow_flag(int value)
{
	g_logo_show = value;
	return g_logo_show;
}

