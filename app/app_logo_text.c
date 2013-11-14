#include <stdio.h>

#include <ti/sdo/dmai/Dmai.h>
#include <ti/sdo/dmai/BufTab.h>
#include <ti/sdo/dmai/Capture.h>
#include <ti/sdo/dmai/ColorSpace.h>
#include <ti/sdo/dmai/BufferGfx.h>

#include <ti/sdo/simplewidget/Screen.h>
#include <ti/sdo/simplewidget/Png.h>

#include "logo.h"
#include "app_logo_text.h"

#include "log_common.h"
#include "sysparam.h"
#include "middle_control.h"
#include "mid_task.h"
#include "app_video_output.h"

static DisplayLogo_Text  g_displayFlag = 0;
Logo_Handle gLogoinfo;
typedef struct __textinfo *Text_Handle ;
static Text_Handle  gtextInfo = NULL;

extern Png_Handle hPng;
extern pthread_mutex_t logomutex;
extern PLock            gSetP_m;



Logo_Handle getLogoInfoHandle(void)
{
	return gLogoinfo;
}

int setLogoInfoHandle(Logo_Handle newLogoInfo)
{
	PRINTF("newLogoInfo = %p\n", newLogoInfo);
	//gLogoinfo = newLogoInfo;
	memcpy(gLogoinfo, newLogoInfo, sizeof(LogoInfo));
	return 0;
}


int getLogoInfo(char *data, int vallen, char *outdata)
{
	int ret = 0;
	Logo_Handle Logo = NULL;
	Logo = getLogoInfoHandle();
	//ret = ReadLogoinfo(LOGOCONFIGNAME, Logo);

	if(ret == 0) {
		PRINTF("read logoinfo failed ret=%d\n", ret);
		memcpy(outdata, Logo, sizeof(LogoInfo));
		return ret;
	}

	memcpy(outdata, Logo, sizeof(LogoInfo));
	PRINTF("LOGO:x=%d,y=%d,enable=%d,alpha=%d", Logo->x, Logo->y, Logo->enable, Logo->alpha);
	return ret;
}

int setLogoInfo(char *data, int vallen, char *outdata)
{
	Logo_Handle logo = (Logo_Handle)data;

	Logo_Handle handle = NULL;
	int ret = 0;
	int logoflag = 0;

	if(vallen < sizeof(LogoInfo)) {
		ret = -1;
		return ret;
	}

	//memcpy(logo, data, sizeof(LogoInfo));
	PRINTF("LOGO:x=%d,y=%d,enable=%d,alpha=%d,name=%s\n", logo->x, logo->y, logo->enable, logo->alpha, logo->filename);

	if(logo->alpha > 100 || logo->alpha < 0 || logo->x < 0 || logo->x > 1920 || logo->y < 0 || logo->y > 1080 || logo->isThrough > 3 || logo->isThrough < 0) {
		ret = -2;
		return ret;
	}

	logo->enable = 1;
	strcpy(logo->filename, "logo.png");
	setLogoInfoHandle(logo);
	//	logoflag = getShowLogoTextFlag();
	//	logoflag = logoflag | OnlyShowLogo;
	//	PRINTF("LOGO:showlogotextflag = %d\n", logoflag);
	//	setShowLogoTextFlag(logoflag);
	PRINTF("LOGO:x=%d,y=%d,enable=%d,alpha=%d,name=%s\n",
	       gLogoinfo->x, gLogoinfo->y, gLogoinfo->enable, gLogoinfo->alpha, gLogoinfo->filename);

	//add by reset logo
	handle = getLogoInfoHandle();
	reloadLogo(handle);

	ret = WriteLogoinfo(LOGOCONFIGNAME, gLogoinfo);

	if(ret == 0) {
		PRINTF("save logoinfo to file failed ret =%d\n", ret);
		return ret;
	}

	return ret;
}

int webupLoadLogo(char *indata, char *outdata)
{
	int ret = 0;
	char com[256] = {0};
	char filename[256] = {0};

	system("sync");

	strcpy(filename, indata);

	strcpy(outdata, indata);


	// check the logo file
	ret =  checklogo(filename);

	if(ret == 0) {
		snprintf(com, sizeof(com), "mv %s %s", filename, LOGOFILE);
		system(com);
		PRINTF("success change logo file. com=%s\n", com);
	}

	PRINTF("check logo ret = %d\n", ret);
	return ret;
}
extern int capture_width, capture_height;
void appShowLogo(Logo_Handle lh, Buffer_Handle hCapBuf, int mode)
{
	//BufferGfx_Dimensions dim;
	int x, y, width, height;
	int max_width, max_height;
	pthread_mutex_lock(&logomutex);


	if(lh->enable) {
		//BufferGfx_getDimensions(hCapBuf, &dim);
		//printf("dstDim.width=%d,height=%d,lineleng=%d\n",dim.width,dim.height,dim.lineLength);
		//dim.width= 1280;
		//dim.height = 720;
		//dim.lineLength = 1920;
		//BufferGfx_setDimensions(hCapBuf, &dim);

		x = lh->x;
		y = lh->y;
		width =  lh->width;
		height = lh->height;

		if(x >= 1920 - width) {
			x = 1920;
		}

		if(y >= 1080 - height) {
			y = 1080;
		}

		x = (x * capture_width / 1920)  - width;
		y = (y * capture_height / 1080) - height;

		if(x < 0) {
			x = 0;
		}

		if(y < 0) {
			y = 0;
		}


		app_png_show(x, y, hCapBuf, lh->isThrough, 0, lh->alpha); //默认过滤掉黑色
		//	Png_showTransparency(hPng, x, y, hCapBuf, lh->isThrough, 0, lh->alpha); //默认过滤掉黑色
	}


	pthread_mutex_unlock(&logomutex);
}



/*
****************************************************
******************************************************
******************************************************
*****************************************************
*/

int initTextinfo(void)
{
	textinfo *info = NULL;
	info = (textinfo *)malloc(sizeof(textinfo));
	info->xpos = 100;
	info->ypos = 200;
	info->enable = 0;
	info->alpha = 0;
	info->showtime = 0;
	strcpy(info->msgtext, "enc1260");
	gtextInfo = info;
	return 0;
}

int text_reset_info()
{
	if(gtextInfo == NULL) {
		PRINTF("\n");
		return -1;
	}

	textinfo *info = gtextInfo;
	//	memset(gtextInfo,0,sizeof(textinfo));
	info->xpos = 100;
	info->ypos = 200;
	info->enable = 0;
	info->alpha = 0;
	info->showtime = 0;
	strcpy(info->msgtext, "enc1260");
	return 0;

}
Text_Handle getTextInfoHandle(void)
{
	return gtextInfo;
}


int setTextInfoHandle(Text_Handle newtextInfo)
{
	//gtextInfo = newtextInfo;
	//need add some code
	PRINTF("newtextInfo = %p\n", newtextInfo);
	memcpy(gtextInfo, newtextInfo, sizeof(textinfo));
	return 0;
}


int getTextInfo(char *data, int vallen, char *outdata)
{
	int ret = 0;
	Text_Handle text = NULL;
	text = getTextInfoHandle();
	ret = readTextFromfile(ADDTEXT_FILE, text);

	if(ret == -1) {
		PRINTF("read textinfo failed ret=%d\n", ret);
		memcpy(outdata, text, sizeof(textinfo));
		return ret;
	}

	//outdata = text;
	memcpy(outdata, text, sizeof(textinfo));
	PRINTF("text:x=%d,y=%d,enable=%d,alpha=%d", text->xpos, text->ypos, text->enable, text->alpha);
	return ret;
}



int setTextInfo(char *data, int vallen, char *outdata)
{
	if(vallen != sizeof(textinfo)) {
		PRINTF("\n");
		return -1;
	}

	DEBUG(DL_DEBUG, "display text enter ....\n");
	textinfo *tinfo = (textinfo *)(data);
	int ret = 0;
	int textflag = 0;

	if(vallen < sizeof(textinfo)) {
		ret = -1;
		return ret;
	}

	//memcpy(&tinfo, data, sizeof(textinfo));

	if(tinfo->alpha > 100 || tinfo->alpha < 0 || tinfo->xpos < 0 || tinfo->xpos > 1920 || tinfo->ypos < 0 || tinfo->ypos > 1080) {
		ret = -2;
		return ret;
	}

	tinfo->enable = 1;
	DEBUG(DL_DEBUG, "tinfo->alpha =%d,tinfo.xpos:%d,tinfo.ypos:%d,tinfo.showtime:%d,tinfo.msgtext:%s\n",
	      tinfo->alpha, tinfo->xpos, tinfo->ypos, tinfo->showtime, tinfo->msgtext);
	ret = addtextdisplay(tinfo);
	//	textflag = getShowLogoTextFlag();
	//	textflag = textflag | OnlyShowText;
	//	setShowLogoTextFlag(textflag);
	WriteTextTofile(ADDTEXT_FILE, tinfo);
	setTextInfoHandle(tinfo);
	memcpy(outdata, tinfo, sizeof(tinfo));
	DEBUG(DL_DEBUG, "display text leave .... textflag = %d\n", textflag);
	return 0;
}


int getTextAlpha(void)
{
	return gtextInfo->alpha;
}

int readTextFromfile(const char *config_file, textinfo *text)
{
	char 			temp[512] = {0};
	int 			ret  = 0 ;
	int 			enable = 0;
	int 			rst = 0;
	PRINTF("LOGO=%p\n", text);
	pthread_mutex_lock(&gSetP_m.save_sys_m);

	ret =  ConfigGetKey(config_file, "text", "x", temp);

	if(ret != 0) {
		PRINTF("Failed to Get logo x\n");
		goto EXIT;
	}

	text->xpos = atoi(temp);
	ret =  ConfigGetKey(config_file, "text", "y", temp);

	if(ret != 0) {
		PRINTF("Failed to Get logo y\n");
		goto EXIT;
	}

	text->ypos = atoi(temp);
	ret =  ConfigGetKey(config_file, "text", "enable", temp);

	if(ret != 0) {
		PRINTF("Failed to Get logo enable\n");
		goto EXIT;
	}

	text->enable = atoi(temp);

	ret =  ConfigGetKey(config_file, "text", "alpha", temp);

	if(ret != 0) {
		PRINTF("Failed to Get logo alpha\n");
		goto EXIT;
	}

	text->alpha = atoi(temp);

	ret =  ConfigGetKey(config_file, "text", "showtime", temp);

	if(ret != 0) {
		PRINTF("Failed to Get logo isThrough\n");
		goto EXIT;
	}

	text->showtime = atoi(temp);

	//pos type
	ret =  ConfigGetKey(config_file, "text", "postype", temp);

	if(ret != 0) {
		PRINTF("Failed to Get logo isThrough\n");
		goto EXIT;
	}

	text->postype = atoi(temp);

	ret =  ConfigGetKey(config_file, "text", "content", temp);

	if(ret != 0) {
		PRINTF("Failed to Get logo filename\n");
		goto EXIT;
	}

	//temp[15] = 0;
	strcpy(text->msgtext, temp);
	rst = 1;
EXIT:
	pthread_mutex_unlock(&gSetP_m.save_sys_m);

	return rst;
}


int WriteTextTofile(const char *config_file, textinfo *text)
{
	char 			temp[512] = {0};
	int 			ret  = 0 ;
	int 			enable = 0;
	int 			rst = 0;
	pthread_mutex_lock(&gSetP_m.save_sys_m);

	sprintf(temp, "%d", text->xpos);
	ret =  ConfigSetKey(config_file, "text", "x", temp);

	if(ret != 0) {
		PRINTF("Failed to Get logo x\n");
		goto EXIT;
	}

	sprintf(temp, "%d", text->ypos);
	ret =  ConfigSetKey(config_file, "text", "y", temp);

	if(ret != 0) {
		PRINTF("Failed to Get logo y\n");
		goto EXIT;
	}

	sprintf(temp, "%d", text->enable);
	ret =  ConfigSetKey(config_file, "text", "enable", temp);

	if(ret != 0) {
		PRINTF("Failed to Get logo enable\n");
		goto EXIT;
	}

	strcpy(temp, text->msgtext);
	temp[strlen(text->msgtext)] = '\0';
	ret =  ConfigSetKey(config_file, "text", "content", temp);

	if(ret != 0) {
		PRINTF("Failed to Get logo filename\n");
		goto EXIT;
	}

	sprintf(temp, "%d", text->alpha);
	ret =  ConfigSetKey(config_file, "text", "alpha", temp);

	if(ret != 0) {
		PRINTF("Failed to Get logo alpha\n");
		goto EXIT;
	}

	sprintf(temp, "%d", text->showtime);
	ret =  ConfigSetKey(config_file, "text", "showtime", temp);

	if(ret != 0) {
		PRINTF("Failed to Get logo isThrough\n");
		goto EXIT;
	}

	sprintf(temp, "%d", text->postype);
	ret =  ConfigSetKey(config_file, "text", "postype", temp);

	if(ret != 0) {
		PRINTF("Failed to Get logo isThrough\n");
		goto EXIT;
	}


	rst = 1;
EXIT:
	pthread_mutex_unlock(&gSetP_m.save_sys_m);

	return rst;
}





//int getShowLogoTextFlag(void)
//{
//	return g_displayFlag;
//}
//int setShowLogoTextFlag(DisplayLogo_Text in)
//{
//	if(in < 0 || in > 3) {
//		in = 0;
//	}

//	g_displayFlag = in;
//	setlogotextflag(in);
//	return g_displayFlag;
//}

#if 0
int webgetShowLogoTextFlag(DisplayLogo_Text in, DisplayLogo_Text *out)
{
	PRINTF("g_displayFlag =%d\n", g_displayFlag);
	*out = g_displayFlag;
	return g_displayFlag;
}

int websetShowLogoTextFlag(DisplayLogo_Text in, DisplayLogo_Text *out)
{
	if(in < 0 || in > 3) {
		in = 0;
		g_displayFlag = in;
		*out = in;
		return -1;
	}

	g_displayFlag = in;
	*out = g_displayFlag;
	mid_timer_create(3, 1, (mid_func_t)writeOutputVideoParam);
	return g_displayFlag;
}
#endif


