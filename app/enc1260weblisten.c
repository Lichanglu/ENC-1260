#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include <strings.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <xdc/std.h>
#include <errno.h>

#include <linux/types.h>
#include <linux/spi/spidev.h>

#include <ti/sdo/dmai/Capture.h>


//#include "netcom.h"
#include "demo.h"
#include "common.h"
#ifdef DSS_ENC_1100_1200
#include "remotectrl.h"
#include "protocalstatus.h"
//#include "setPHY.h"

#include "rtsp_server.h"

#include "log_common.h"
#include "settype.h"
#include "WEB_CGI/cgi-enc/weblib.h"
#include "WEB_CGI/cgi-enc/webTcpCom.h"
#include "logo.h"
#include "app_logo_text.h"
#include "app_video_output.h"
#include "./stream_output/stream_output_struct.h"
#include "build_info.h"
#include "enc1260weblisten.h"

#include "mid_timer.h"


extern int g_gpio_fd;
#define KERNELFILENAME "uImage"




//extern  StreamParam streamParams;
extern  RtspParam	rtspParams;
extern  OutputVideoInfo outputparam;
extern  ResizeParam 	gResizeP;
extern SavePTable  gSysParaT;
extern	int capture_width, capture_height;

extern void signal_set(void);
extern int getRGB_YPRPR(void);
extern int SetAudioParam(int dsp, unsigned char *data, int len);
extern int SetVideoParam(int dsp, unsigned char *data, int len);
extern int SetSysParams(SYSPARAMS *pold, SYSPARAMS *pnew);
extern int addtextdisplay(textinfo *text);
#define MAX_LISTEN		 10
#define DSP1			0
extern int g_gpio_fd;
/*FPGA update*/
extern int g_FPGA_update;
/*protocol struct*/
extern Protocol gProtocol;
/*HV table*/
extern HVTable gHVTable;
/*Mult Addr status*/
//MultAddr s_MultAddr[MAX_PROTOCOL];
//extern int g_IframeInterval[MAX_VAL];


/*消息头打包*/
void msgPacket(int identifier,unsigned char *data, webparsetype type, int len, int cmd, int ret)
{
	webMsgInfo_1260  msghead;
	int	cmdlen = sizeof(type);
	int retlen = sizeof(ret);
	msghead.identifier = identifier;
	msghead.type = type;
	msghead.len = len;
	memcpy(data, &msghead, MSGINFOHEAD);
	memcpy(data + MSGINFOHEAD, &cmd, cmdlen);
	memcpy(data + MSGINFOHEAD + cmdlen, &ret, retlen);
	//printf("msghead.len=%d\n", msghead.len);
	return ;
}
static  int webupdatesystem(const char *filename)
{
	char command[256] = {0};
	char filepath[256] = {0};
	int ret = 0;

#ifdef ADD_UPDATE_HEADER
	ret = app_header_info_ok(filename);

	if(ret == 0) {
		if(app_del_extern_buff(filename) == -1) {
			PRINTF("error! del the file extern_buff is error\n");
			return -1;
		}

		sprintf(command, "rm -rf %s;mv %s_temp %s", filename, filename, filename);
		PRINTF("command=%s\n", command);
		system(command);

	} else {
		if(ret == 2) {
			PRINTF("This file isn't have add extern update header\n");
			return -1;
		} else {
			PRINTF("This file have add extern update header,but the header have some error\n");
			return -1;
		}
	}

#endif

	system("rm -rf /opt/update");

	system("mkdir -p /opt/update");

	SET_FPGA_UPDATE(FPGA_UPDATE);
	sleep(1);
	sprintf(command, "tar -xzvf %s  -C /", filename);
	system(command);

	sprintf(filepath, "/opt/update/%s", KERNELFILENAME);

	/*更新内核*/
	if(access(filepath, F_OK) == 0)	{
		fprintf(stderr, "start update kernel \n");
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
	return 0;
}


int rebootSys(int data, int *out)
{
	int ret = 0;
	system("sync");
	usleep(500000);
	DEBUG(DL_DEBUG, "Restart sys\n");
	ret = system("reboot -f");

	if(ret < 0) {
		usleep(500000);
		ret = system("reboot -f");
	}

	return ret;
}
int getinputType(int in, int *out)
{
	int input = 0;
	//int	cmdlen, vallen, retlen;

	int ret = 0;

	switch(gblGetIDX()) {
		case AD9880_INDEX:
			input = DVI;
			DEBUG(DL_DEBUG, "Web get DVI input\n");
			break;

		case SDI_INDEX:
			input = SDI ;
			DEBUG(DL_DEBUG, "Web get SDI input\n");
			break;

		case AUTO_INDEX:
			input = AUTO ;
			DEBUG(DL_DEBUG, "Web get AUTO input\n");
			break;

		default:
			input = AUTO ;
			DEBUG(DL_DEBUG, "Web get default AUTO input\n");
			break;
	}

	*out = input;
	return ret;
}


int getsoftconfigtime(int identifier,int fd, int cmd, char *data, int vallen)
{
	int totallen;
	int	cmdlen, retlen;
	int ret = 0xff00;
	char curtime[64] = {0};
	unsigned char senddata[128] = {0};
	cmdlen = retlen = sizeof(int);
	totallen = MSGINFOHEAD + cmdlen + retlen + vallen;
	msgPacket(identifier,senddata, INT_TYPE, totallen, cmd, ret);
	memcpy(senddata + (totallen - vallen), curtime, vallen);
	send(fd, senddata, totallen, 0);
	return 0;
}
int getcpuload(int in, int *out)
{
	*out = getArmLoad();
	return 0;
}

int setColorSpace(int data, int *out)
{
	int ret = 0;
	int status = 0;

	int old = 0;

	if(data < 0 || data > 1) {
		DEBUG(DL_DEBUG, "setColorSpace failed data illegal\n");
		status = -1;
		goto EXIT;
	}

	getColorSpace(&old);

	if(data == old) {
		PRINTF("the color space is same\n");
		status = 0;
		goto EXIT;
	}

	DEBUG(DL_DEBUG, "setColorSpace enter!!\n");
	ret = GreenScreenAjust();

	if(ret < 0) {
		DEBUG(DL_DEBUG, "setColorSpace failed ret = %d!!\n", ret);
		status = ret;
		goto EXIT;
	}

EXIT:
	*out = data;
	DEBUG(DL_DEBUG, "setColorSpace leave!!\n");
	return status;
}
int getColorSpace(int *out)
{
	int ret = 0;
	
	int mode =  gblGetMode();
	int digital = app_get_digital_flag(mode);
	ret = app_get_green_adjust_value(digital, mode, out);
	
//	*out = getRGB_YPRPR();
//	PRINTF(".........RGB_YPRPR=%d.........\n",*out);
	return ret;
}

int restoresetparse(int in, int *out)
{
	PRINTF("restore default set enter ......\n");
	//	Logo_Handle logo;
	InitSysParams();
	initOutputVideoParam();
	//	logo = initLogoMod();
	//initTextinfo();
	text_reset_info();
	SaveParamsToFlash();
	writeOutputVideoParam();
	//	WriteLogoinfo(LOGOCONFIGNAME, logo);
	ResetLogoinfo();
	WriteTextTofile(ADDTEXT_FILE, getTextInfoHandle());
	PRINTF("restore default set leave ......\n");

	app_rtsp_config_save();
	app_mult_config_reset();

	sleep(10);
	system("reboot -f");
	return 0;
}

int setInputType(int in , int *out)
{
	//	inputtype stype;
	int val = 0;

	switch(in) {
		case DVI:
			DEBUG(DL_DEBUG, "Web Change to DVI \n");
			gblSetIDX(AD9880_INDEX, 1);
			val = DVI;
			break;

		case SDI:
			DEBUG(DL_DEBUG, "Web Change to SDI \n");
			gblSetIDX(SDI_INDEX, 1);
			val = SDI;
			break;

		case AUTO:
			DEBUG(DL_DEBUG, "Web Change to AUTO \n");
			gblSetIDX(AUTO_INDEX, 1);
			val = AUTO;
			break;

		default:
			DEBUG(DL_DEBUG, "default!!!!!!!! mode\n");
			gblSetIDX(AUTO_INDEX, 1);
			val = AUTO;
			break;
	}

	*out = val;
	return 0;
}

int getCtrlProto(int in, int *out)
{
	*out = gblGetRemoteIndex();
	return 0;
}

int setCtrlProto(int data, int *out)
{
	int ret = 0;

	if(data < MAX_FAR_CTRL_NUM) {
		gblSetRemoteIndex(data);
		ret = SaveLocalRemoteCtrlIndex();
	} else {
		ret = -1;
	}

	*out = data;
	return ret;
}



int getHDCPValue(int data, int *out)
{
	int ret = 0;
	SIGNAL_STA		op;
	ret = ioctl(g_gpio_fd, SIGNAL_STA_MODE, &op);

	if(ret == -1) {
		DEBUG(DL_DEBUG, "getHDCPValue op.HDMI_HDCP failed:%d\n", op.HDMI_HDCP);
		*out = -1;
		return -1;
	}

	*out = op.HDMI_HDCP;
	DEBUG(DL_DEBUG, "getHDCPValue op.HDMI_HDCP:%d\n", op.HDMI_HDCP);
	return 0;
}

int webGetMute(int data, int *out)
{
	*out = gblGetMuteStatus();
	return 0;

}
int webSetMute(int data, int *out)
{
	int ret = 0;

	if(data < 0 || data > 1) {
		*out = data;
		PRINTF("transform param error data = %d\n", data);
		ret = -1;
	}

	if(data) {
		gblSetMute(START);
	} else {
		gblSetMute(STOP);
	}

	return ret;
}
int revisePicture(int identifier,int fd, int cmd, char *data, int vallen)
{
	unsigned char sdata[256] = {0};
	int totallen = 0;
	int cmdlen = sizeof(int);
	int retlen = sizeof(int);
	int val = 0;
	int ret = 0xff00;
	unsigned char temp[20];
	short hporch, vporch;
	memcpy(temp, data, vallen);
	hporch = (short)(temp[0] | temp[1] << 8);
	vporch = (short)(temp[2] | temp[3] << 8);
	val = revise_picture(hporch, vporch);
	totallen = MSGINFOHEAD + cmdlen + vallen + retlen;
	msgPacket(identifier,sdata, STRING_TYPE, totallen, cmd, ret);
	memcpy(sdata + totallen - vallen, data, vallen);
	send(fd, sdata, totallen, 0);
	return 0;
}

int getSignaldetailInfo(int identifier,int fd, int cmd, char *data, int vallen)
{
#if 1
	int i;
	SIGNAL_STA		op;
	unsigned char sdata[2048] = {0};
	int totallen = 0;
	int cmdlen = sizeof(int);
	int retlen = sizeof(int);
	int ret = 0;
	char tmpbuf[128] = {0};
	char displaybuf[2048] = {0};
	getInputMode(tmpbuf);
	i = ioctl(g_gpio_fd, SIGNAL_STA_MODE, &op);
	DEBUG(DL_DEBUG, "ret=%d,op.HDMI_HDCP=%d,op->HPV=%d,op->RGB_YPRPR=%d,op->TMDS=%d,op->VsyncF=%d\n",
	      i, op.HDMI_HDCP, op.HPV, op.RGB_YPRPR, op.TMDS, op.VsyncF);
	sprintf(displaybuf, "Signal:\\t  %s\\n", tmpbuf);
	memset(tmpbuf, 0, strlen(tmpbuf));
	sprintf(tmpbuf, "HPV:\\t  %d\\n", op.HPV);
	strcat(displaybuf, tmpbuf);
	memset(tmpbuf, 0, strlen(tmpbuf));
	sprintf(tmpbuf, "TMDS:\\t  %d\\n", op.TMDS);
	strcat(displaybuf, tmpbuf);
	memset(tmpbuf, 0, strlen(tmpbuf));
	sprintf(tmpbuf, "VsyncF:\\t  %d\\n", op.VsyncF);
	strcat(displaybuf, tmpbuf);
	memset(tmpbuf, 0, strlen(tmpbuf));
	sprintf(tmpbuf, "HDCP:\\t  %d\\n", op.HDMI_HDCP);
	strcat(displaybuf, tmpbuf);
	memset(tmpbuf, 0, strlen(tmpbuf));
	sprintf(tmpbuf, "RGB_YPRPR: %d\\n", op.RGB_YPRPR);
	strcat(displaybuf, tmpbuf);
	vallen = strlen(displaybuf);
	totallen = MSGINFOHEAD + cmdlen + vallen + retlen;
	msgPacket(identifier,sdata, STRING_TYPE, totallen, cmd, ret);
	memcpy(sdata + totallen - vallen, displaybuf, vallen);
	send(fd, sdata, totallen, 0);
	return 0;
#endif
}


int getInputSignalInfo(int identifier,int fd, int cmd, char *data, int vallen)
{
	unsigned char sdata[256] = {0};
	int totallen = 0;
	int cmdlen = sizeof(int);
	int retlen = sizeof(int);
	int ret = 0;
	char info[64] = {0};
	getInputMode(info);
	vallen = strlen(info);
	totallen = MSGINFOHEAD + cmdlen + vallen + retlen;
	msgPacket(identifier,sdata, STRING_TYPE, totallen, cmd, ret);
	memcpy(sdata + totallen - vallen, info, vallen);
	send(fd, sdata, totallen, 0);
	return 0;
}

int app_get_build_time(int identifier,int fd, int cmd, char *data, int vallen)
{
	unsigned char sdata[256] = {0};
	int totallen = 0;
	int cmdlen = sizeof(int);
	int retlen = sizeof(int);
	int ret = 0;
	char info[64] = {0};
	//	getInputMode(info);
	snprintf(info, sizeof(info), "%s", g_make_build_date);
	vallen = strlen(info);
	totallen = MSGINFOHEAD + cmdlen + vallen + retlen;
	msgPacket(identifier,sdata, STRING_TYPE, totallen, cmd, ret);
	memcpy(sdata + totallen - vallen, info, vallen);
	send(fd, sdata, totallen, 0);
	return 0;
}


int getOutputVideoInfo(int identifier,int fd, int cmd, char *data, int vallen)
{

	unsigned char sdata[2048] = {0};
	int totallen = 0;
	int cmdlen = sizeof(int);
	int retlen = sizeof(int);
	int ret = 0;
	OutputVideoInfo param;
	memset(&param, 0, sizeof(OutputVideoInfo));

	getOutputvideohandle(&param);
	totallen = MSGINFOHEAD + cmdlen + vallen + retlen;
	msgPacket(identifier,sdata, STRUCT_TYPE, totallen, cmd, ret);
	memcpy(sdata + totallen - vallen, &param, vallen);
	PRINTF("getOutputVideoInfo...cmd =0x%x\n", cmd);
	send(fd, sdata, totallen, 0);
	return 0;
}

int setOutputVideoInfo(int identifier,int fd, int cmd, char *data, int vallen)
{
	OutputVideoInfo recvparam;
	OutputVideoInfo oldoutparam;
	int ret = 0x00, save;
	unsigned char sdata[256] = {0};
	int totallen = 0;
	int cmdlen = sizeof(int);
	int retlen = sizeof(int);

	memset(&oldoutparam, 0, sizeof(OutputVideoInfo));
	getOutputvideohandle(&oldoutparam);

	if(sizeof(recvparam) != vallen) {
		ret = SERVER_RET_INVAID_PARM_LEN;
		PRINTF("sizeof(recvparam)  =%d,%d\n", sizeof(recvparam) , vallen);
		goto EXIT;
	}

	memcpy(&recvparam, data, vallen);

	save = setOutputParame(&recvparam, &oldoutparam);

	setOutputvideohandle(&oldoutparam);
	PRINTF("\n");

	if(save) {
		mid_timer_create(3, 1, (mid_func_t)writeOutputVideoParam);
		mid_timer_create(3, 1, (mid_func_t)SaveParamsToFlash);
		//	writeOutputVideoParam();
		//	SaveParamsToFlash();
	}

EXIT:
	totallen = MSGINFOHEAD + cmdlen + vallen + retlen;
	msgPacket(identifier,sdata, STRUCT_TYPE, totallen, cmd, ret);
	memcpy(sdata + totallen - vallen, &oldoutparam, vallen);
	send(fd, sdata, totallen, 0);

	return 0;
}
int getSysParams(int identifier,int fd, int cmd, char *data, int vallen)
{
	unsigned char sdata[256] = {0};
	int totallen = 0;
	int cmdlen = sizeof(int);
	int retlen = sizeof(int);
	int ret = 0;
	totallen = MSGINFOHEAD + cmdlen + vallen + retlen;
	msgPacket(identifier,sdata, STRUCT_TYPE, totallen, cmd, ret);
	//DEBUG(DL_DEBUG, "ip =%d,gateway=%d\n", gSysParaT.sysPara.dwAddr, gSysParaT.sysPara.dwGateWay);
	memcpy(sdata + totallen - vallen, &(gSysParaT.sysPara), vallen);
	//DEBUG(DL_DEBUG, "getSysparam...cmd =%d sdata=%s\n", cmd, sdata);
	send(fd, sdata, totallen, 0);
	return 0;
}

int setSysParams(int identifier,int fd, int cmd, char *data, int vallen)
{
	SYSPARAMS sysParam;
	int ret = 0, save;
	unsigned char sdata[256] = {0};
	int totallen = 0;
	int cmdlen = sizeof(int);
	int retlen = sizeof(int);
	memcpy(&sysParam, data, vallen);
	save = SetSysParams(&gSysParaT.sysPara, &sysParam);

	if(save == 1) {
		SaveParamsToFlash();
	}

	if(save == -1) {
		ret = save;
	}

	totallen = MSGINFOHEAD + cmdlen + vallen + retlen;
	msgPacket(identifier,sdata, STRUCT_TYPE, totallen, cmd, ret);
	memcpy(sdata + totallen - vallen, &sysParam, vallen);
	DEBUG(DL_DEBUG, "SetVideoParam...cmd =0x%x\n", cmd);
	send(fd, sdata, totallen, 0);

	if(save == 1) {
		sleep(5);
		system("reboot -f");
	}

	return 0;
}
int getEncodeTime(int identifier,int fd, int cmd, char *data, int vallen)
{
#if 1
	unsigned char sdata[256] = {0};
	int totallen = 0;
	int cmdlen = sizeof(int);
	int retlen = sizeof(int);
	int ret = 0;
	DATE_TIME_INFO dtinfo;
	getSysTime(&dtinfo);
	totallen = MSGINFOHEAD + cmdlen + vallen + retlen;
	msgPacket(identifier,sdata, STRUCT_TYPE, totallen, cmd, ret);
	memcpy(sdata + totallen - vallen, &dtinfo, vallen);
	//DEBUG(DL_DEBUG, "getStreamParam...cmd =%d sdata=%s\n", cmd, sdata);
	send(fd, sdata, totallen, 0);
#endif
	return 0;
}

int getProtocol(int identifier,int fd, int cmd, char *data, int vallen)
{
	unsigned char sdata[256] = {0};
	int totallen = 0;
	int cmdlen = sizeof(int);
	int retlen = sizeof(int);
	int ret = 0;
	Protocol pro;
	DEBUG(DL_DEBUG, "Get Protocol Status!!\n");
	memcpy(&pro, &gProtocol, sizeof(Protocol));
	totallen = MSGINFOHEAD + cmdlen + vallen + retlen;
	msgPacket(identifier,sdata, INT_TYPE, totallen, cmd, ret);
	memcpy(sdata + totallen - vallen, &pro, sizeof(Protocol));
	DEBUG(DL_DEBUG, "SetAudioParam...cmd =0x%x\n", cmd);
	send(fd, sdata, totallen, 0);
	return 0;
}
int farControl(int identifier,int fd, int cmd, char *data, int vallen)
{
	unsigned char sdata[256] = {0};
	int totallen = 0;
	int cmdlen = sizeof(int);
	int retlen = sizeof(int);
	int ret = 0;
	unsigned char temp[20];
	memcpy(temp, data, vallen);
	PRINTF("vallen=%d\n",vallen);
	FarCtrlCamera(DSP1, temp, vallen);
	totallen = MSGINFOHEAD + cmdlen + vallen + retlen;
	msgPacket(identifier,sdata, STRING_TYPE, totallen, cmd, ret);
	memcpy(sdata + totallen - vallen, data, vallen);
	DEBUG(DL_DEBUG, "FarCtrlCamera...cmd =0x%x\n", cmd);
	send(fd, sdata, totallen, 0);
}
int updateFile(int identifier,int fd, int cmd, char *data, int vallen)
{
	unsigned char sdata[256] = {0};
	int totallen = 0;
	int cmdlen = sizeof(int);
	int retlen = sizeof(int);
	char filename[1024] = {0};
	int ret = 0;
	memset(filename, 0x000, vallen);
	memcpy(filename, data, vallen);
	DEBUG(DL_DEBUG, "update Ok!!!!!!!  filename = %s\n", filename);

	ret = webupdatesystem(filename);

	totallen = MSGINFOHEAD + cmdlen + vallen + retlen;
	msgPacket(identifier,sdata, STRING_TYPE, totallen, cmd, ret);
	memcpy(sdata + totallen - vallen, data, vallen);
	DEBUG(DL_DEBUG, "ret =%d webupdatesystem...cmd =0x%x totallen=%d=vallen=%d\n", ret , cmd, totallen, vallen);
	send(fd, sdata, totallen, 0);
	//modify by zm for update reboot
	app_set_reboot_flag(1);

	if(ret == 0) {
		PRINTF("\n");
		sleep(3);
		system("reboot -f");
	}

}

static int g_app_need_stop = 0;

int app_set_reboot_flag(int flag)
{
	g_app_need_stop  = flag;
}

int app_get_reboot_flag()
{
	return g_app_need_stop;
}

int SetDeviceType_1260(int identifier,int fd, int cmd, char *data, int vallen)
{
	/*设置设备型号*/
	DEBUG(DL_DEBUG, "Set Device Type\n");
	char dtype[16] = {0};
	unsigned char sdata[256] = {0};
	int totallen = 0;
	int cmdlen = sizeof(int);
	int retlen = sizeof(int);
	int ret;
	memset(dtype, 0x000, strlen(dtype));
	memcpy(dtype, data, vallen);
	ret = SetDeviceType(DTYPECONFIG_NAME, dtype);
	totallen = MSGINFOHEAD + cmdlen + vallen + retlen;
	msgPacket(identifier,sdata, STRING_TYPE, totallen, cmd, ret);
	memcpy(sdata + totallen - vallen, data, vallen);
	DEBUG(DL_DEBUG, "SetDeviceType...cmd =0x%x\n", cmd);
	send(fd, sdata, totallen, 0);
	return 0;
}
int getDeviceType_1260(int identifier,int fd, int cmd, char *data, int vallen)
{
	/*获得设备型号*/
	DEBUG(DL_DEBUG, "Get Device Type\n");
	char dtype[16] = {0};
	unsigned char sdata[256] = {0};
	int totallen = 0;
	int cmdlen = sizeof(int);
	int retlen = sizeof(int);
	int ret = 0;
	GetDeviceType(dtype);
	vallen = strlen(dtype);
	totallen = MSGINFOHEAD + cmdlen + vallen + retlen;
	msgPacket(identifier,sdata, STRING_TYPE, totallen, cmd, ret);
	memcpy(sdata + totallen - vallen, dtype, vallen);
	DEBUG(DL_DEBUG, "GetDeviceType...cmd =0x%x\n", cmd);
	send(fd, sdata, totallen, 0);
	//DEBUG(DL_DEBUG,"dtype = %s\n", dtype);
	//sprintf(dtype,"%d-%d-%d",op.HPV,op.TMDS,op.VsyncF);
	DEBUG(DL_DEBUG, "dtype = %s\n", dtype);
	return 0;
}

int GetVideoParam_1260(int identifier,int fd, int cmd, char *data, int vallen)
{
	unsigned char sdata[256] = {0};
	int totallen = 0;
	int cmdlen = sizeof(int);
	int retlen = sizeof(int);
	int ret = 0;
	totallen = MSGINFOHEAD + cmdlen + vallen + retlen;
	msgPacket(identifier,sdata, STRUCT_TYPE, totallen, cmd, ret);
	memcpy(sdata + totallen - vallen, &(gSysParaT.videoPara[DSP1]), vallen);
	DEBUG(DL_DEBUG, "GetVideoParam...cmd =0x%x\n", cmd);
	send(fd, sdata, totallen, 0);
	return 0;
}

int setVideoParam_1260(int identifier,int fd, int cmd, char *data, int vallen)
{
	VideoParam vParam;
	int ret = 0, save;
	unsigned char sdata[256] = {0};
	int totallen = 0;
	int cmdlen = sizeof(int);
	int retlen = sizeof(int);
	memcpy(&vParam, data, vallen);

	save = SetVideoParam(DSP1, (unsigned char *)&vParam, sizeof(VideoParam));

	if(save) {
		DEBUG(DL_DEBUG, "save param save = %d\n", save);
		SaveParamsToFlash();
	}

	totallen = MSGINFOHEAD + cmdlen + vallen + retlen;
	msgPacket(identifier,sdata, STRUCT_TYPE, totallen, cmd, ret);
	memcpy(sdata + totallen - vallen, &vParam, vallen);
	DEBUG(DL_DEBUG, "SetVideoParam...cmd =0x%x\n", cmd);
	send(fd, sdata, totallen, 0);
	return 0;
}

int GetAudioParam_1260(int identifier,int fd, int cmd, char *data, int vallen)
{
	unsigned char sdata[1024] = {0};
	int totallen = 0;
	int cmdlen = sizeof(int);
	int retlen = sizeof(int);
	int ret = 0;
	totallen = MSGINFOHEAD + cmdlen + vallen + retlen;
	msgPacket(identifier,sdata, STRUCT_TYPE, totallen, cmd, ret);
	memcpy(sdata + totallen - vallen, &(gSysParaT.audioPara[DSP1]), vallen);
	DEBUG(DL_DEBUG, "GetAudioParam...SampleRate =%d BitRate=%d\n",
	      gSysParaT.audioPara[0].SampleRate, gSysParaT.audioPara[0].BitRate);
	send(fd, sdata, totallen, 0);
	return 0;
}

int setAudioParam_1260(int identifier,int fd, int cmd, char *data, int vallen)
{
	AudioParam aParam;
	int ret = SERVER_RET_OK, save = 0, status = 0;
	unsigned char sdata[256] = {0};
	int totallen = 0;
	int cmdlen = sizeof(int);
	int retlen = sizeof(int);

	if(vallen < sizeof(AudioParam)) {
		ret = SERVER_RET_INVAID_PARM_LEN;
		status = -1;
		goto EXIT;
	}

	memcpy(&aParam, data, vallen);
	save = SetAudioParam(DSP1, (unsigned char *)&aParam, sizeof(AudioParam));

	if(save) {
		DEBUG(DL_DEBUG, "save audio param ret = %d\n", save);
		mid_timer_create(3, 1, (mid_func_t)SaveParamsToFlash);
		//SaveParamsToFlash();
	}

EXIT:

	totallen = MSGINFOHEAD + cmdlen + vallen + retlen;
	msgPacket(identifier,sdata, STRUCT_TYPE, totallen, cmd, ret);
	memcpy(sdata + totallen - vallen, &aParam, vallen);
	DEBUG(DL_DEBUG, "SetAudioParam...cmd =0x%x\n", cmd);
	send(fd, sdata, totallen, 0);
	return status;
}

int syncTime(int identifier,int fd, int cmd, char *data, int vallen)
{
	DATE_TIME_INFO dtinfo;
	int ret = 0;
	unsigned char sdata[256] = {0};
	int totallen = 0;
	int cmdlen = sizeof(int);
	int retlen = sizeof(int);
	memcpy(&dtinfo, data, vallen);
	PRINTF("dtinfo.year:%d,dtinfo.month:%d,dtinfo.mday:%d,dtinfo.hours:%d,dtinfo.min:%d,dtinfo.sec:%d\n",
	       dtinfo.year, dtinfo.month, dtinfo.mday, dtinfo.hours, dtinfo.min, dtinfo.sec);
	PRINTF("1 the run time =%d=%lld\n",get_run_time(),getCurrentTime());
	setRtcTime(dtinfo.year, dtinfo.month, dtinfo.mday, dtinfo.hours, dtinfo.min, dtinfo.sec);
	PRINTF("2 the run time =%d=%lld\n",get_run_time(),getCurrentTime());
#if 0  //del by zhangmin
	ReInitTimeTick();
#endif
	totallen = MSGINFOHEAD + cmdlen + vallen + retlen;
	msgPacket(identifier,sdata, STRUCT_TYPE, totallen, cmd, ret);
	memcpy(sdata + totallen - vallen, &dtinfo, vallen);
	DEBUG(DL_DEBUG, "syncTime...cmd =%d dtinfo:%d,%d,%d,%d,%d,%d\n",
	      cmd, dtinfo.year, dtinfo.month, dtinfo.mday, dtinfo.hours, dtinfo.min, dtinfo.sec);
	ret = send(fd, sdata, totallen, 0);

	if(ret < 0) {
		PRINTF("send failed ret:%d,totallen : %d\n", ret, totallen);
	}

	/*if time is change ,i must do some thing*/
	//app_build_reset_time();
	return 0;
}


int midParseInt(int identifier,int fd, char *data, int len)
{
	int recvlen;
	int cmd = 0;
	int actdata = 0;
	//char logoname[15] = "logo.png";
	int ret = 0;
	int web_ret = SERVER_RET_OK;

	int need_send = 0;


	char senddata[1024] = {0};
	int totallen = 0;

	recvlen = recv(fd, data, len, 0);
	int out = 0;

//	if(recvlen < 0) {
//		return -1;
//	}

	if(recvlen < 0 || len != sizeof(cmd) + sizeof(int)) {
		web_ret = SERVER_RET_INVAID_PARM_LEN;
		need_send = 1;
		goto EXIT;
	}


	memcpy(&cmd, data, sizeof(int));
	memcpy(&actdata, data + sizeof(int), len - sizeof(int));

	PRINTF("cmd = 0x%04x\n", cmd);

	switch(cmd) {
		case MSG_GETINPUTTYPE:
			PRINTF("cmd = 0x%04x is MSG_GETINPUTTYPE.\n", cmd);
			ret = getinputType(actdata, &out);
			need_send = 1;
			break;

		case MSG_SETINPUTTYPE:
			PRINTF("cmd = 0x%04x is MSG_SETINPUTTYPE.\n", cmd);
			ret = setInputType(actdata, &out);
			need_send = 1;
			break;

			//case MSG_STOP_MULT:
			//	stopMultParse(fd, cmd, &actdata);
			//	break;

		case MSG_GETCPULOAD:
			PRINTF("cmd = 0x%04x is MSG_GETCPULOAD.\n", cmd);
			ret = getcpuload(actdata, &out);
			need_send = 1;
			break;

		case MSG_RESTORESET:
			PRINTF("cmd = 0x%04x is MSG_RESTORESET.\n", cmd);
			ret = restoresetparse(actdata, &out);
			need_send = 1;
			break;

		case MSG_GET_CTRL_PROTO:
			PRINTF("cmd = 0x%04x is MSG_GET_CTRL_PROTO.\n", cmd);
			need_send = 1;
			getCtrlProto(actdata, &out);
			break;

		case MSG_SET_CTRL_PROTO:
			PRINTF("cmd = 0x%04x is MSG_SET_CTRL_PROTO.\n", cmd);
			need_send = 1;
			setCtrlProto(actdata, &out);
			break;

			/*
			case MSG_SAVEPARAM:
				saveParams(fd, cmd, &actdata);
				break;
			*/
#if 0

		case MSG_GETDHCPFLAG:
			PRINTF("cmd = 0x%04x is MSG_GETDHCPFLAG.\n", cmd);
			need_send = 1;
			PRINTF("MSG_GETDHCPFLAG actdata = %d\n", actdata);
			webgetDHCPFlag(actdata, &out);
			break;

		case MSG_SETDHCPFLAG:
			PRINTF("cmd = 0x%04x is MSG_SETDHCPFLAG.\n", cmd);
			need_send = 1;
			websetDHCPFlag(actdata, &out);
			break;
#endif

		case MSG_SETCOLORSPACE:
			PRINTF("cmd = 0x%04x is MSG_SETCOLORSPACE.\n", cmd);
			need_send = 1;
			ret = setColorSpace(actdata, &out);
			break;

		case MSG_GETCOLORSPACE:
			PRINTF("cmd = 0x%04x is MSG_GETCOLORSPACE.\n", cmd);
			getColorSpace(&out);
			need_send = 1;
			break;

			//case MSG_GETHDCPVAL:
			//	getHDCPValue(fd, cmd, &actdata);
			//	break;

		case MSG_REBOOTSYS:
			PRINTF("cmd = 0x%04x is MSG_REBOOTSYS.\n", cmd);
			need_send = 1;
			rebootSys(actdata, &out);
			break;

		case MSG_GETHDCPVALUE:
			PRINTF("cmd = 0x%04x is MSG_GETHDCPVALUE.\n", cmd);
			need_send = 1;
			getHDCPValue(actdata, &out);
			break;

		case MSG_LOCK_RESOLUTION:
			PRINTF("cmd = 0x%04x is MSG_LOCK_RESOLUTION.\n", cmd);
			ret = DoLockResolution(actdata, &out);
			need_send = 1;
			break;

		case MSG_GETOUTPUTRESOLUTION:
			PRINTF("cmd = 0x%04x is MSG_GETOUTPUTRESOLUTION.\n", cmd);
			ret = webgetOutputResolution(actdata, &out);
			need_send = 1;
			break;

		case MSG_RESIZE_MODE:
			PRINTF("cmd = 0x%04x is MSG_RESIZE_MODE.\n", cmd);
			//	setResizeMode(fd, cmd, &actdata);
			ret = setResizeMode(actdata, &out);
			need_send = 1;
			break;

		case MSG_GETRESIZEMODE:
			PRINTF("cmd = 0x%04x is MSG_GETRESIZEMODE.\n", cmd);
			//getResizeMode(fd, cmd, &actdata);
			ret = getResizeMode(actdata, &out);
			need_send = 1;
			break;

		case MSG_GETENCODELV:
			PRINTF("cmd = 0x%04x is MSG_GETENCODELV.\n", cmd);
			ret = getEncodelevel(actdata, &out);

			need_send = 1;
			break;

		case MSG_SETENCODELV:
			PRINTF("cmd = 0x%04x is MSG_SETENCODELV.\n", cmd);
			ret = setEncodelevel(actdata, &out);

			need_send = 1;
			break;

		case MSG_GETSCENE:
			PRINTF("cmd = 0x%04x is MSG_GETSCENE.\n", cmd);
			need_send = 1;
			getSceneconfig(actdata, &out);
			break;

		case MSG_SETSCENE:
			PRINTF("cmd = 0x%04x is MSG_SETSCENE.\n", cmd);
			need_send = 1;
			ret = setSceneconfig(actdata, &out);
			break;

		case MSG_GETFRAMERATE:
			PRINTF("cmd = 0x%04x is MSG_GETFRAMERATE.\n", cmd);
			need_send = 1;
			getFrameRate(actdata, &out);
			break;

		case MSG_SETFRAMERATE:
			PRINTF("cmd = 0x%04x is MSG_SETFRAMERATE.\n", cmd);
			need_send = 1;
			ret = setFrameRate(actdata, &out);
			break;

		case MSG_GETIFRAMEINTERVAL:
			PRINTF("cmd = 0x%04x is MSG_GETIFRAMEINTERVAL.\n", cmd);
			need_send = 1;
			getIFrameInterval(actdata, &out);
			break;

			//case MSG_SETIFRAMEINTERVAL:
			//	PRINTF("cmd = 0x%04x is MSG_SETIFRAMEINTERVAL.\n", cmd);
			//	need_send = 1;
			//	ret = setIFrameInterval(actdata, &out);
			//	break;

		case MSG_GETBITRATE:
			PRINTF("cmd = 0x%04x is MSG_GETBITRATE.\n", cmd);
			need_send = 1;
			getBitRate(actdata, &out);
			break;

		case MSG_SETBITRATE:
			PRINTF("cmd = 0x%04x is MSG_SETBITRATE.\n", cmd);
			need_send = 1;
			ret = setBitRate(actdata, &out);
			break;


		case MSG_SETMUTE:
			PRINTF("cmd = 0x%04x is MSG_SETMUTE.\n", cmd);
			need_send = 1;
			ret = webSetMute(actdata, &out);
			break;

		case MSG_GETMUTE:
			PRINTF("cmd = 0x%04x is MSG_GETMUTE.\n", cmd);
			webGetMute(actdata, &out);
			need_send = 1;
			break;

			//stream_ouput
		case MSG_RTSP_GET_USED:
		case MSG_RTSP_DEL_SERVER:
		case MSG_MULT_GET_NUM:
		case MSG_MULT_GET_TS_RATE:
		case MSG_MULT_GET_RTP_RATE:
		case MSG_MULT_GET_RTPTS_RATE:
			app_web_stream_output_process_int(cmd, actdata, &out);
			need_send = 1;
			break;

			//case MSG_GETSHOWTEXTLOGO:
			//	PRINTF("cmd = 0x%04x is MSG_GETSHOWTEXTLOGO.\n", cmd);
			//	webgetShowLogoTextFlag(actdata, &out);
			//	need_send = 1;
			//	break;

			//	case MSG_SETSHOWTEXTLOGO:
			//		PRINTF("cmd = 0x%04x is MSG_SETSHOWTEXTLOGO.\n", cmd);
			//		ret = websetShowLogoTextFlag(actdata, &out);
			//		need_send = 1;
			//		break;
			//		case MSG_GET_SHOWTEXT_FLAG:
			//			out = app_get_textshow_flag();
			//			need_send = 1;
			//			break;
			//		case MSG_SET_SHOWTEXT_FLAG:
			//			out = app_set_textshow_flag(actdata);
			//			need_send = 1;
			//			break;
			//		case MSG_GET_SHOWLOGO_FLAG:
			//			out = app_get_logoshow_flag();
			//			need_send = 1;
			//			break;
			//		case MSG_SET_SHOWLOGO_FLAG:
			//			out = app_set_logoshow_flag(actdata);
			//			need_send = 1;
			//			break;

		default:
			PRINTF("unkonwn cmd = %04x\n", cmd);
			need_send = 1;
			web_ret = SERVER_RET_UNKOWN_CMD;
			break;
			//	case
	}

	if(ret < 0) {
		web_ret = SERVER_RET_INVAID_PARM_VALUE;
	}

EXIT:

	if(need_send == 1) {
		totallen = MSGINFOHEAD + sizeof(cmd) + sizeof(web_ret) + sizeof(out);
		msgPacket(identifier,senddata, INT_TYPE, totallen, cmd, web_ret);
		memcpy(senddata + (totallen - sizeof(out)), &out, sizeof(out));
		PRINTF("the cmd =%04x,the value=%d,the ret=%04x\n", cmd, out, web_ret);
		send(fd, senddata, totallen, 0);
	}

	return 0;
}


int midParseString(int identifier,int fd, char *data, int len)
{
	int recvlen;
	int cmd = 0;
	char actdata[4096] = {0};
	int vallen = 0;
	int needsend = 0;

	char senddata[1024] = {0};
	int totallen = 0;

	char  out[4096] = "unknown cmd.";
	int web_ret = SERVER_RET_OK;
	int need_send = 0;

	recvlen = recv(fd, data, len, 0);

	vallen = len - sizeof(int);

	if(recvlen < 0 || vallen > sizeof(actdata)) {
		web_ret = SERVER_RET_INVAID_PARM_LEN;
		need_send = 1;
		goto EXIT;
	}


	//actdata = (char *)malloc(vallen);
	memcpy(&cmd, data, sizeof(int));
	memcpy(actdata, data + sizeof(int), vallen);
	PRINTF("cmd = %04x\n", cmd);

	switch(cmd) {
		case MSG_SIGNALDETAILINFO:
			PRINTF("cmd = 0x%04x is MSG_SIGNALDETAILINFO.\n", cmd);
			getSignaldetailInfo(identifier,fd, cmd, actdata, vallen);
			break;

		case MSG_GETINPUTSIGNALINFO:
			PRINTF("cmd = 0x%04x is MSG_GETINPUTSIGNALINFO.\n", cmd);
			getInputSignalInfo(identifier,fd, cmd, actdata, vallen);
			break;

		case MSG_REVISE_PICTURE:
			PRINTF("cmd = 0x%04x is MSG_REVISE_PICTURE.\n", cmd);
			revisePicture(identifier,fd, cmd, actdata, vallen);
			break;

		case MSG_FAR_CTRL:
			PRINTF("cmd = 0x%04x is MSG_FAR_CTRL.\n", cmd);
			//FarCtrlCamera(int dsp, unsigned char * data, int len);
			farControl(identifier,fd, cmd, actdata, vallen);
			break;

		case MSG_UPDATESYS:
			PRINTF("cmd = 0x%04x is MSG_UPDATESYS.\n", cmd);
			updateFile(identifier,fd, cmd, actdata, vallen);
			break;

		case MSG_SETDEVICETYPE:
			PRINTF("cmd = 0x%04x is MSG_SETDEVICETYPE.\n", cmd);
			SetDeviceType_1260(identifier,fd, cmd, actdata, vallen);
			break;

		case MSG_GETDEVICETYPE:
			PRINTF("cmd = 0x%04x is MSG_GETDEVICETYPE.\n", cmd);
			getDeviceType_1260(identifier,fd, cmd, actdata, vallen);
			break;

		case MSG_GETSOFTCONFIGTIME:
			PRINTF("cmd = 0x%04x is MSG_GETSOFTCONFIGTIME.\n", cmd);
			//getsoftconfigtime(fd, cmd, actdata, vallen);
			app_get_build_time(identifier,fd, cmd, actdata, vallen);
			break;

		case MSG_GETSOFTVERSION:
			PRINTF("cmd = 0x%04x is MSG_GETSOFTVERSION.\n", cmd);
			app_get_soft_version(out, sizeof(out));
			need_send = 1;
			break;

			//case  MSG_GET_BUILD_TIME:
			//	app_get_build_time(fd, cmd, actdata, vallen);
			//	break;
		case MSG_UPLOADIMG:
			PRINTF("cmd = 0x%04x is MSG_UPLOADIMG.\n", cmd);
			web_ret = webupLoadLogo(actdata, out);

			need_send = 1;
			break;
		case MSG_SDK_LOGIN: 
			{
				PRINTF("MSG_LOGIN fd =%d, data=%s\n", fd, actdata);
				web_ret = app_sdk_login_check(actdata, out);
				
				need_send = 1;
			}
			break;
		case MSG_GET_SDP_INFO:
			{
				PRINTF("MSG_GET_SDP_INFO  \n");
				web_ret = rtsp_get_sdp_describe(out, sizeof(out));
				 PRINTF("SDP:#%s#,len=%d\n",out,strlen(out));
				need_send = 1;
			}			
			break;
			
		default:
			PRINTF("Warnning,the cmd %d is UNKOWN\n", cmd);
			need_send = 1;
			break;
	}

EXIT:
	if(need_send == 1) {
		totallen = MSGINFOHEAD + sizeof(cmd) + sizeof(web_ret) + strlen(out);
		msgPacket(identifier,senddata, STRING_TYPE, totallen, cmd, web_ret);
		memcpy(senddata + (totallen - strlen(out)), out, strlen(out));
		PRINTF("the cmd =%04x,the out=%s,the ret=%04x\n", cmd, out, web_ret);
		send(fd, senddata, totallen, 0);
	}

	//	free(actdata);
	return 0;
}


int midParseStruct(int identifier,int fd, char *data, int len)
{
	int recvlen;
	int cmd = 0;
	//char *actualdata = NULL;
	char actualdata[1024] = {0};
	char out[2048] = {0};
	int  vallen = 0;
	int  status = 0;
	int  ret = 0;


	char senddata[1024] = {0};
	int totallen = 0;

	int web_ret = SERVER_RET_OK;
	int need_send = 0;


	recvlen = recv(fd, data, len, 0);

	if(recvlen < 0) {
		DEBUG(DL_ERROR, "recv failed,errno = %d,error message:%s \n", errno, strerror(errno));
		//return -1;
		web_ret = SERVER_RET_INVAID_PARM_LEN;
		status = -1;
		goto EXIT;
	}

	vallen = len - sizeof(int);
	//actualdata = (char *)malloc(vallen);
	memcpy(&cmd, data, sizeof(int));
	memcpy(actualdata, data + sizeof(int), len - sizeof(int));
	//PRINTF("cmd = 0x%04x,vallen=%d=len=%d\n", cmd, vallen, len);

	switch(cmd) {

		case MSG_SETOUTPUTVIDEOINFO:
			PRINTF("cmd = 0x%04x is MSG_SETOUTPUTVIDEOINFO.\n", cmd);
			setOutputVideoInfo(identifier,fd, cmd, actualdata, vallen);
			break;

		case MSG_GETOUTPUTVIDEOINFO:
			PRINTF("cmd = 0x%04x is MSG_GETOUTPUTVIDEOINFO.\n", cmd);
			getOutputVideoInfo(identifier,fd, cmd, actualdata, vallen);
			break;

		case MSG_GETVIDEOPARAM:
			PRINTF("cmd = 0x%04x is MSG_GETVIDEOPARAM.\n", cmd);
			GetVideoParam_1260(identifier,fd, cmd, actualdata, vallen);
			break;

		case MSG_GETAUDIOPARAM:
			PRINTF("cmd = 0x%04x is MSG_GETAUDIOPARAM.\n", cmd);
			GetAudioParam_1260(identifier,fd, cmd, actualdata, vallen);
			break;

		case MSG_GETSYSPARAM:
			PRINTF("cmd = 0x%04x is MSG_GETSYSPARAM.\n", cmd);
			getSysParams(identifier,fd, cmd, actualdata, vallen);
			break;

		case MSG_SETSYSPARAM:
			PRINTF("cmd = 0x%04x is MSG_SETSYSPARAM.\n", cmd);
			setSysParams(identifier,fd, cmd, actualdata, vallen);
			break;

		case MSG_SETVIDEOPARAM:
			PRINTF("cmd = 0x%04x is MSG_SETVIDEOPARAM.\n", cmd);
			setVideoParam_1260(identifier,fd, cmd, actualdata, vallen);
			break;

		case MSG_SETAUDIOPARAM:
			PRINTF("cmd = 0x%04x is MSG_SETAUDIOPARAM.\n", cmd);
			setAudioParam_1260(identifier,fd, cmd, actualdata, vallen);
			break;

		case MSG_SYNCTIME:
			PRINTF("cmd = 0x%04x is MSG_SYNCTIME.\n", cmd);
			syncTime(identifier,fd, cmd, actualdata, vallen);
			break;

		case MSG_GET_PROTOCOL:
			PRINTF("cmd = 0x%04x is MSG_GET_PROTOCOL.\n", cmd);
			getProtocol(identifier,fd, cmd, actualdata, vallen);
			break;

		case MSG_GET_LOGOINFO:
			PRINTF("cmd = 0x%04x is MSG_GET_LOGOINFO.\n", cmd);
			ret = getLogoInfo(actualdata, vallen, out);

			if(ret == 0) {
				web_ret = SERVER_RET_INVAID_PARM_VALUE;
			}

			need_send = 1;
			break;

		case MSG_SET_LOGOINFO:
			PRINTF("cmd = 0x%04x is MSG_SET_LOGOINFO.\n", cmd);
			ret = setLogoInfo(actualdata, vallen, out);

			if(ret == -1) {
				web_ret = SERVER_RET_INVAID_PARM_LEN;
			}

			if(ret == -2) {
				web_ret = SERVER_RET_INVAID_PARM_VALUE;
			}

			need_send  = 1;
			break;

		case MSG_GET_TEXTINFO:
			PRINTF("cmd = 0x%04x is MSG_GET_TEXTINFO.\n", cmd);
			ret = getTextInfo(actualdata, vallen, out);

			if(ret == -1) {
				web_ret = SERVER_RET_INVAID_PARM_VALUE;
			}

			need_send = 1;
			break;

		case MSG_SET_TEXTINFO:
			PRINTF("cmd = 0x%04x is MSG_SET_TEXTINFO.\n", cmd);
			ret = setTextInfo(actualdata, vallen, out);
			need_send = 1;
			break;

		case MSG_GETENCODETIME:
			PRINTF("cmd = 0x%04x is MSG_GETENCODETIME.\n", cmd);
			getEncodeTime(identifier,fd, cmd, actualdata, vallen);
			break;



		case MSG_RTSP_GET_GINFO:
		case MSG_RTSP_SET_GINFO:
		case MSG_RTSP_GET_CINFO:
		case MSG_RTSP_SET_CINFO:
		case MSG_RTSP_ADD_SERVER:
		case MSG_MULT_ADD_SERVER:
		case MSG_MULT_GET_CINFO:
		case MSG_MULT_SET_CINFO:
		case MSG_RTSP_SET_STATUS:
		case MSG_MULT_SET_STATUS:
		case MSG_MULT_DEL_SERVER:
			PRINTF("cmd = 0x%04x is MSG_STREAMOUTPUT\n", cmd);
			app_web_stream_output_process_struct(identifier,fd, cmd, actualdata, vallen);
			break;

		default:
			PRINTF("UNKOWN cmd %d\n", cmd);
			break;

	}

	if(ret < 0) {
		web_ret = SERVER_RET_INVAID_PARM_VALUE;
	}

EXIT:

	if(need_send == 1) {
		totallen = MSGINFOHEAD + sizeof(cmd) + sizeof(web_ret) + vallen;
		msgPacket(identifier,senddata, STRING_TYPE, totallen, cmd, web_ret);
		memcpy(senddata + (totallen - vallen), out, vallen);
		PRINTF("the cmd =%04x,,the ret=%04x\n", cmd,  web_ret);
		send(fd, senddata, totallen, 0);
	}

	//	free(actualdata);
	return status;
}

int app_web_stream_output_process_int(int cmd, int in, int *out)
{
	int ret = 0;

	switch(cmd) {
		case MSG_RTSP_GET_USED:
			ret = app_rtsp_server_used();

			if(ret < 0) {
				ret = -1;
			}

			*out = ret;
			break;

		case MSG_RTSP_DEL_SERVER:
			app_rtsp_server_delete();
			*out = 0;
			break;

		case MSG_MULT_GET_NUM:
			ret = app_multicast_get_total_num();

			if(ret < 0) {
				ret = -1;
			}

			*out = ret;
			break;

		case MSG_MULT_GET_TS_RATE:
		case MSG_MULT_GET_RTP_RATE:
		case MSG_MULT_GET_RTPTS_RATE:
			ret = stream_get_rate(cmd);

			if(ret < 0) {
				ret = -1;
			}

			*out = ret;
			break;

		default:
			break;
	}

	return ret;

}
int app_web_stream_output_process_struct(int identifier,int fd, int cmd, char *data, int valen)
{
	int ret = 0;
	char out[2048] = {0};
	char *temp = NULL;
	//	int outlen = 0;
	int needlen = 0;
	int msgheadlen = 0;
	int totallen = 0;
	stream_output_server_config *config = NULL;
	msgheadlen = MSGINFOHEAD + sizeof(int) + sizeof(ret);
	temp = out + msgheadlen;

	switch(cmd) {
		case MSG_RTSP_GET_GINFO:
			PRINTF("MSG_RTSP_GET_GINFO\n");
			needlen = sizeof(rtsp_server_config);

			if(needlen != valen) {
				PRINTF("needlen = %d,valen=%d\n", needlen, valen);
				ret = SERVER_RET_INVAID_PARM_LEN;
				goto EXIT;
			}

			ret =  app_rtsp_server_get_global_info((rtsp_server_config *)(temp));

			if(ret < 0) {
				ret = SERVER_INTERNAL_ERROR;
				goto EXIT;
			}

			break;

		case MSG_RTSP_SET_GINFO:
			PRINTF("MSG_RTSP_SET_GINFO\n");
			needlen = sizeof(rtsp_server_config);

			if(needlen != valen) {
				PRINTF("needlen = %d,valen=%d\n", needlen, valen);
				ret = SERVER_RET_INVAID_PARM_LEN;
				goto EXIT;
			}

			ret =  app_rtsp_server_set_global_info((rtsp_server_config *)data, (rtsp_server_config *)(temp));

			if(ret < 0) {
				ret = SERVER_INTERNAL_ERROR;
				goto EXIT;
			}

			break;

		case MSG_RTSP_GET_CINFO:
			PRINTF("MSG_RTSP_GET_CINFO\n");
			needlen = sizeof(stream_output_server_config);

			if(needlen != valen) {
				PRINTF("needlen = %d,valen=%d\n", needlen, valen);
				ret = SERVER_RET_INVAID_PARM_LEN;
				goto EXIT;
			}

			ret = app_rtsp_server_get_common_info((stream_output_server_config *)(temp));
			stream_server_config_printf((stream_output_server_config *)(temp));

			if(ret < 0) {
				ret = SERVER_INTERNAL_ERROR;
				goto EXIT;
			}

			break;

		case MSG_RTSP_SET_CINFO:
			PRINTF("MSG_RTSP_SET_CINFO\n");
			needlen = sizeof(stream_output_server_config);

			if(needlen != valen) {
				PRINTF("needlen = %d,valen=%d\n", needlen, valen);
				ret = SERVER_RET_INVAID_PARM_LEN;
				goto EXIT;
			}

			stream_server_config_printf((stream_output_server_config *)data);
			ret =  app_rtsp_server_set_common_info((stream_output_server_config *)data, (stream_output_server_config *)(temp));

			if(ret < 0) {
				ret = SERVER_INTERNAL_ERROR;
				goto EXIT;
			}

			stream_server_config_printf((stream_output_server_config *)(temp));
			break;

		case MSG_RTSP_SET_STATUS:
			PRINTF("MSG_RTSP_SET_STATUS\n");
			needlen = sizeof(stream_output_server_config);

			if(needlen != valen) {
				PRINTF("needlen = %d,valen=%d\n", needlen, valen);
				ret = SERVER_RET_INVAID_PARM_LEN;
				goto EXIT;
			}

			stream_server_config_printf((stream_output_server_config *)data);
			ret =  app_rtsp_server_set_status((stream_output_server_config *)data, (stream_output_server_config *)(temp));

			if(ret < 0) {
				ret = SERVER_INTERNAL_ERROR;
				goto EXIT;
			}

			stream_server_config_printf((stream_output_server_config *)(temp));
			break;


		case MSG_RTSP_ADD_SERVER:
			PRINTF("MSG_RTSP_ADD_SERVER\n");
			needlen = sizeof(stream_output_server_config);

			if(needlen != valen) {
				PRINTF("needlen = %d,valen=%d\n", needlen, valen);
				ret = SERVER_RET_INVAID_PARM_LEN;
				goto EXIT;
			}

			stream_server_config_printf((stream_output_server_config *)data);
			ret =  app_rtsp_server_add((stream_output_server_config *)data, (stream_output_server_config *)temp);

			if(ret < 0) {
				ret = SERVER_INTERNAL_ERROR;
				goto EXIT;
			}

			stream_server_config_printf((stream_output_server_config *)(temp));
			break;

		case MSG_MULT_ADD_SERVER:
			PRINTF("MSG_MULT_ADD_SERVER\n");
			needlen = sizeof(stream_output_server_config);

			if(needlen != valen) {
				PRINTF("needlen = %d,valen=%d\n", needlen, valen);
				ret = SERVER_RET_INVAID_PARM_LEN;
				goto EXIT;
			}

			stream_server_config_printf((stream_output_server_config *)data);
			ret =  app_multicast_add_server((stream_output_server_config *)data, (stream_output_server_config *)temp);

			if(ret < 0) {
				ret = SERVER_INTERNAL_ERROR;
				goto EXIT;
			}

			stream_server_config_printf((stream_output_server_config *)(temp));
			break;


		case MSG_MULT_GET_CINFO:
			PRINTF("MSG_MULT_GET_CINFO\n");

			config = (stream_output_server_config *)data;
			int num = config->num;
			needlen = sizeof(stream_output_server_config);

			if(needlen != valen) {
				PRINTF("needlen = %d,valen=%d\n", needlen, valen);
				ret = SERVER_RET_INVAID_PARM_LEN;
				goto EXIT;
			}

			PRINTF("MSG_MULT_GET_CINFO num=%d\n", num);
			ret = app_multicast_get_config(num, (stream_output_server_config *)(temp));
			stream_server_config_printf((stream_output_server_config *)(temp));

			if(ret < 0) {
				ret = SERVER_INTERNAL_ERROR;
				goto EXIT;
			}

			break;

		case MSG_MULT_SET_CINFO:
			PRINTF("MSG_MULT_SET_CINFO\n");
			needlen = sizeof(stream_output_server_config);

			if(needlen != valen) {
				PRINTF("needlen = %d,valen=%d\n", needlen, valen);
				ret = SERVER_RET_INVAID_PARM_LEN;
				goto EXIT;
			}

			stream_server_config_printf((stream_output_server_config *)data);
			ret =  app_multicast_set_config((stream_output_server_config *)data, (stream_output_server_config *)(temp));
			stream_server_config_printf((stream_output_server_config *)(temp));

			if(ret < 0) {
				ret = SERVER_INTERNAL_ERROR;
				goto EXIT;
			}

			break;

		case MSG_MULT_SET_STATUS:
			PRINTF("MSG_MULT_SET_STATUS\n");
			needlen = sizeof(stream_output_server_config);

			if(needlen != valen) {
				PRINTF("needlen = %d,valen=%d\n", needlen, valen);
				ret = SERVER_RET_INVAID_PARM_LEN;
				goto EXIT;
			}

			stream_server_config_printf((stream_output_server_config *)data);
			ret =  app_multicast_set_status((stream_output_server_config *)data, (stream_output_server_config *)(temp));
			stream_server_config_printf((stream_output_server_config *)(temp));

			if(ret < 0) {
				ret = SERVER_INTERNAL_ERROR;
				goto EXIT;
			}

			break;

		case MSG_MULT_DEL_SERVER:
			PRINTF("MSG_MULT_DEL_SERVER\n");
			needlen = sizeof(stream_output_server_config);

			if(needlen != valen) {
				PRINTF("needlen = %d,valen=%d\n", needlen, valen);
				ret = SERVER_RET_INVAID_PARM_LEN;
				goto EXIT;
			}

			stream_server_config_printf((stream_output_server_config *)data);
			ret =  app_multicast_delete_server((stream_output_server_config *)data);
			stream_server_config_printf((stream_output_server_config *)(temp));

			if(ret < 0) {
				ret = SERVER_INTERNAL_ERROR;
				goto EXIT;
			}

			break;

		default:
			break;

	}

EXIT:
	totallen = msgheadlen  + needlen;
	msgPacket(identifier,out, STRUCT_TYPE, totallen, cmd, ret);

	PRINTF("ret = %d,the totallen = %d\n", ret, totallen);
	send(fd, out, totallen, 0);
	return 0;

}

/*WEB server Thead Function*/
Void *weblistenThrFxn(Void *arg)
{
	PRINTF("get pid= %d\n", getpid());
	Void                   *status              = THREAD_SUCCESS;
	int 					listenSocket  		= 0 , flags = 0;
	struct sockaddr_in 		addr;
	//struct in_addr          addr_ip;
	int len, client_sock, opt = 1;
	struct sockaddr_in client_addr;
	webMsgInfo_1260		webinfo_1260;
	ssize_t			recvlen;
	//int count = 0;
	char  data_1260[2048] = {0};
	// signal_set();


	//	for(count = TS_STREAM; count < MAX_PROTOCOL; count++) {
	//		WebInitMultAddr(&s_MultAddr[count]);
	//	}

	len = sizeof(struct sockaddr_in);
	memset(&client_addr, 0, len);
	listenSocket =	socket(PF_INET, SOCK_STREAM, 0);

	if(listenSocket < 1)	{
		status  = THREAD_FAILURE;
		return status;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family =       AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(ENCODESERVER_PORT);

	setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	if(bind(listenSocket, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) != 0)  	{
		DEBUG(DL_ERROR, "[weblistenThrFxn] bind failed,errno = %d,error message:%s \n", errno, strerror(errno));
		status  = THREAD_FAILURE;
		return status;
	}


	if(-1 == listen(listenSocket, MAX_LISTEN))     {
		DEBUG(DL_ERROR, "listen failed,errno = %d,error message:%s \n", errno, strerror(errno));
		status  = THREAD_FAILURE;
		return status;
	}

	if((flags = fcntl(listenSocket, F_GETFL, 0)) == -1)	{
		DEBUG(DL_ERROR, "fcntl F_GETFL error:%d,error msg: = %s\n", errno, strerror(errno));
		gblSetQuit();
		status  = THREAD_FAILURE;
		return status ;
	}

	if(fcntl(listenSocket, F_SETFL, flags | O_NONBLOCK) == -1) {
		DEBUG(DL_ERROR, "fcntl F_SETFL error:%d,error msg: = %s\n", errno, strerror(errno));
		gblSetQuit();
		status  = THREAD_FAILURE;
		return status ;
	}

	//ProtocolInit();
	//InitProtocalStatus();





	while(!gblGetQuit()) {
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(listenSocket, &rfds);
		//接收recv_buf 复位为空!
		select(FD_SETSIZE, &rfds , NULL , NULL , NULL);
		//PRINTF("\n");
		client_sock = accept(listenSocket, (struct sockaddr *)&client_addr, (socklen_t *)&len);

		if(0 > client_sock)     {
			PRINTF("\n");

			if(errno == ECONNABORTED || errno == EAGAIN) 	{
				//usleep(20000);
				continue;
			}

			DEBUG(DL_DEBUG, "weblisten thread Function errno  = %d\n", errno);
			status  = THREAD_FAILURE;
			return status;
		}

		memset(&webinfo_1260, 0, sizeof(webinfo_1260));
		recvlen = recv(client_sock, &webinfo_1260, sizeof(webinfo_1260), 0);
		//PRINTF("recvlen = %d webinfo.len =%d webinfo.type = %d,id=%x\n",
		//       recvlen, webinfo_1260.len, webinfo_1260.type, webinfo_1260.identifier);

		if(recvlen < 1)	{
			PRINTF("recv failed,errno = %d,error message:%s,client_sock = %d\n", errno, strerror(errno), client_sock);
			status  = THREAD_FAILURE;
			return status;
		}

		if(webinfo_1260.identifier != WEB_IDENTIFIER) {
			PRINTF("id  error,client_sock = %d\n", client_sock);
			status  = THREAD_FAILURE;
			return status;
		}

		len = webinfo_1260.len - sizeof(webinfo_1260);

		PRINTF("web deal begin =%d\n", webinfo_1260.type);

		switch(webinfo_1260.type) {
			case INT_TYPE:
				//PRINTF("web deal begin =%d\n", webinfo_1260.type);
				midParseInt(webinfo_1260.identifier,client_sock, data_1260, len);
				//PRINTF("web deal end =%d\n", webinfo_1260.type);
				break;

			case STRING_TYPE:
				//	PRINTF("web deal begin =%d\n", webinfo_1260.type);
				midParseString(webinfo_1260.identifier,client_sock, data_1260, len);
				//	PRINTF("web deal end =%d\n", webinfo_1260.type);
				break;

			case STRUCT_TYPE:
				//	PRINTF("web deal begin =%d\n", webinfo_1260.type);
				midParseStruct(webinfo_1260.identifier,client_sock, data_1260, len);
				//	PRINTF("web deal end =%d\n", webinfo_1260.type);
				break;

			default:
				break;
		}

		PRINTF("web deal end =%d\n", webinfo_1260.type);
		close(client_sock);
	}

	//ProtocolExit();

	close(listenSocket);
	DEBUG(DL_DEBUG, "Web listen Thread Function Exit!!\n");
	return status;
}


#endif //DSS_ENC_1100_1200

