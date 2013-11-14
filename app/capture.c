#include <xdc/std.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include <ti/sdo/dmai/Ccv.h>
#include <ti/sdo/dmai/Fifo.h>
#include <ti/sdo/dmai/Blend.h>
#include <ti/sdo/dmai/Pause.h>
#include <ti/sdo/dmai/BufTab.h>
#include <ti/sdo/dmai/Capture.h>
#include <ti/sdo/dmai/Display.h>
#include <ti/sdo/dmai/BufferGfx.h>
#include <ti/sdo/dmai/Rendezvous.h>
#include <ti/sdo/dmai/Resize.h>

#include <ti/sdo/ce/Engine.h>
#include <ti/sdo/ce/video2/viddec2.h>

#include "demo.h"
#include "common.h"
#include "capture.h"
#include "sysparam.h"
#include "tcpcom.h"
#include "kernel_common.h"
#include "logo.h"
#include "app_logo_text.h"

#ifdef HAVE_QUALITY_MODULE
#include "app_quality.h"
#endif
#include "log_common.h"



#define VIDENC_COPY				1
#define MODE_1080I50			23
#define MODE_1080I60			24

#define MODE_480I60				29
#define MODE_576I50				30

/* Buffering for the capture driver */
#define NUM_CAPTURE_BUFS        4
int g_gpio_fd = 0 ;

extern ResizeParam gLowRate;
extern ResizeParam gResizeP;
extern int g_FPGA_update;
/*HV Table */
extern HVTable gHVTable;
/*默认HV值表*/
extern const unsigned int FPGA_HV_TALBE[MAX_MODE_VALUE][4];

extern pthread_mutex_t		gCapEnc_m;

extern CurrentResolutionStatus lock_status;

extern int	gWriteFlag;
int capture_width = INITWIDTH, capture_height = INITHEIGHT;
extern void addtextosdtoender(Buffer_Handle hDstBuf);
static int green_screen_flag = 0;
static int gsignal_hpv = 0;
static int gsignal_vsyncF = 0;
static int gsignal_ret = 0;
static int gsignal_RGB_YPRPR=0;
static int ScreenAjust(void);
static void change_screen_adjust(int mode);

static int capture_get_i2c_error();
static int capture_set_i2c_error(int flag);

/*initial revise picture*/
int InitRevisePicture(void)
{

	dmai_v4l2_control ctrl;
	int mode = gblGetMode();
	union revise val;
	//	union revise nval;
	int ret = 0 , tmds  = 0;
	Capture_Handle hCapture = gblGetCapture();

	if(mode < 0 || mode >= MAX_MODE_VALUE) {
		return 0;
	}

	if(hCapture == NULL) {
		return -1;
	}

	//hv:
	get_digital_status(hCapture, &ctrl);
	tmds = ctrl.value ;
	DEBUG(DL_DEBUG, "tmds =%d\n", tmds);

	if(tmds) {
		val.h_v.h = gHVTable.digital[mode].hporch;
		val.h_v.v = gHVTable.digital[mode].vporch;
		DEBUG(DL_DEBUG, "digital channel\n");
	} else {
		val.h_v.h = gHVTable.analog[mode].hporch;
		val.h_v.v = gHVTable.analog[mode].vporch;
		DEBUG(DL_DEBUG, "analog channel\n");
	}

	DEBUG(DL_DEBUG, "matchbox val.h_v.h = %d val.h_v.v = %d\n", val.h_v.h, val.h_v.v);
	ctrl.value = val.value;
	hCapture = gblGetCapture();

	if(hCapture == NULL) {
		return -1;
	}

	ret = set_encoder_hv(hCapture, &ctrl);
	DEBUG(DL_DEBUG, "[InitRevisePicture] InitRevisePicture set_encoder_hv after ret = [%d]\n", ret);
#if 0

	ret = get_encoder_hv(hCapture, &ctrl);

	if(ret < 0) {
		gblCaptureunLock();
		DEBUG(DL_DEBUG, "[InitRevisePicture] error ret = %d\n", ret);
		return -1;
	}

	nval.value = ctrl.value;
	DEBUG(DL_DEBUG, "[InitRevisePicture] get nval.h_v.h = %d nval.h_v.v = %d\n", nval.h_v.h, nval.h_v.v);

	if((nval.h_v.h != val.h_v.h) || (nval.h_v.v != val.h_v.v)) {
		DEBUG(DL_DEBUG, "[InitRevisePicture] goto set falded#@@@@@@@@@@@@@@@@@@@@\n");
		gblCaptureunLock();
		goto hv;
	}

#endif
	return 0;
}

/*调节边框*/
int revise_picture(short h, short v)
{
	dmai_v4l2_control ctrl;
	dmai_v4l2_control before;
	Capture_Handle hCapture = NULL;
	int mode = gblGetMode();
	int ret = -1;
	union revise val;
	int tmds = 0;
	int idx = gblGetIDX() ;

	if(mode < 0 || (idx == SDI_INDEX)) {
		return 0;
	}

	gblCaptureLock();

	if(gblGetCapture() == NULL) {
		gblCaptureunLock();
		return -1;
	}

	hCapture = gblGetCapture();
	//	gblCaptureLock();
	ret = get_encoder_hv(hCapture, &before);
	get_digital_status(hCapture, &ctrl);
	tmds = ctrl.value;


	gblCaptureunLock();

	if(ret < 0) {
		DEBUG(DL_DEBUG, "[get_encoder_hv] error ret = %d\n", ret);
	}

	val.value = before.value;
	PRINTF("matchbox get mode =%d,dig=%d,H value = %d   V value = %d,h=%d,v=%d\n", mode, tmds, val.h_v.h, val.h_v.v, h, v);

	if((val.h_v.h + h) < 0) {
		DEBUG(DL_DEBUG, "[revise_picture] hproch value adjust min value = %d!!\n", (val.h_v.h + h));
		return 0;
	}

	if((val.h_v.v + v) < 0) {
		DEBUG(DL_DEBUG, "[revise_picture] vproch value adjust min!! value = %d\n", (val.h_v.v + v));
		return 0;
	}

	if(h == 0 && v == 0) {  //恢复默认值
		int digital = 0;
		get_digital_status(hCapture, &ctrl);
		digital = ctrl.value;

		if(digital) {
			val.h_v.h = FPGA_HV_TALBE[mode][2];
			val.h_v.v = FPGA_HV_TALBE[mode][3];
		} else {
			val.h_v.h = FPGA_HV_TALBE[mode][0];
			val.h_v.v = FPGA_HV_TALBE[mode][1];
		}

#ifdef CL4000_DVI

		if(1 != mode) {
			ScreenAjust();
		}

#endif
	} else {
		/*数字信号的时候，无法获取到hv值*/
		if(tmds == 1) {
			val.h_v.h = gHVTable.digital[mode].hporch;
			val.h_v.v = gHVTable.digital[mode].vporch;
		}

		PRINTF("matchbox get mode =%d,dig=%d,H value = %d   V value = %d,h=%d,v=%d\n", mode, tmds, val.h_v.h, val.h_v.v, h, v);

		val.h_v.h = val.h_v.h + h;
		val.h_v.v = val.h_v.v + v;
	}

	DEBUG(DL_DEBUG, "matchbox set val.h_v.h = %d val.h_v.v = %d\n", val.h_v.h, val.h_v.v);
	ctrl.value = val.value;
	gblCaptureLock();
	hCapture = gblGetCapture();

	if(hCapture == NULL) {
		DEBUG(DL_ERROR, "[revise_picture] hCapture == NULL");
		gblCaptureunLock();
		return -1;
	}

	ret = set_encoder_hv(hCapture, &ctrl);
	gblCaptureunLock();

	if(ret < 0) {
		DEBUG(DL_DEBUG, "[revise_picture] error ret = %d\n", ret);
		return -1;
	}

#if 0
	ret = get_encoder_hv(hCapture, &before);

	if(ret < 0) {
		DEBUG(DL_DEBUG, "22[revise_picture] error ret = %d\n", ret);
		return -1;
	}

	val.value = before.value;
	DEBUG(DL_DEBUG, "matchbox get val.h_v.h = %d val.h_v.v = %d\n", val.h_v.h, val.h_v.v);
#endif
	HVTable pNewHW;

	if(mode >= 0) {
		gblCaptureLock();
		hCapture = gblGetCapture();
		gblCaptureunLock();

		if(hCapture == NULL) {
			return -1;
		}

		memset(&ctrl, 0, sizeof(ctrl));
		get_digital_status(hCapture, &ctrl);
		tmds = ctrl.value;
		DEBUG(DL_DEBUG, "signal value tmds = %d\n", tmds);
		memcpy(&pNewHW, &gHVTable, sizeof(HVTable));
		ret = BeyondHVTable(mode, tmds, val.h_v.h, val.h_v.v, &pNewHW);
		ret = SaveHVTable(&gHVTable, &pNewHW, tmds);
	}

	return (ret);
}

/*
typedef struct SIGNAL_OP__{
	unsigned int HPV;
	unsigned int VsyncF;
	unsigned int TMDS;
	unsigned int RGB_YPRPR;
	unsigned int HDMI_HDCP;
	unsigned int HDMI_DVI;
}SIGNAL_STA_TEST;

	
#define SIGNAL_STA_MODE_TEST    _IOWR('g', 23, SIGNAL_STA_TEST)

*/
static 	char g_kernel_verison[20] = {0};
static 	unsigned short g_fpga_ver = 0 ;
int open_gpio_port()
{
	g_gpio_fd = OpenGPIODevice();

	if(g_gpio_fd < 0) {
		DEBUG(DL_ERROR, "open gpio port failed!!!!!\n");
		return -1;
	}

	DEBUG(DL_DEBUG, "###################################################\n");
	GetKernelVersion(g_gpio_fd, g_kernel_verison);
	GetFPGAVersion(g_gpio_fd, &g_fpga_ver);
	SetMacAddr(g_gpio_fd);
	DEBUG(DL_DEBUG, "g_gpio_fd = %d\n", g_gpio_fd);
	DEBUG(DL_DEBUG, "kernel version = %s\n", g_kernel_verison);
	DEBUG(DL_DEBUG, "FPGA verison = %x\n", g_fpga_ver);
	DEBUG(DL_DEBUG, "###################################################\n");

	return 0;
}
extern SavePTable 	gSysParaT;
int app_get_soft_version(char *buff, int len)
{
	snprintf(buff, len, "%s(%s/0x%04x)", gSysParaT.sysPara.strVer, g_kernel_verison, g_fpga_ver);
	PRINTF("soft version is %s\n", buff);
	return 0;
}

int set_gpio_bit(int bit)
{
	int ret = 0;
	ret = SetGPIOBitValue(g_gpio_fd, bit);

	if(ret < 0) {
		DEBUG(DL_DEBUG, "set_gpio_bit() failed!!!\n ");
		return -1 ;
	}

	DEBUG(DL_DEBUG, "balance mode switch !!!!\n");
	return 0;
}

int clear_gpio_bit(int bit)
{
	int ret = 0;
	ret = ClearGPIOBitValue(g_gpio_fd, bit);

	if(ret < 0) {
		DEBUG(DL_DEBUG, "clear_gpio_bit() failed!!!\n ");
		return -1 ;
	}

	return 0;
}

int get_gpio_bit(int bit)
{
	int ret =  -1;
	ret = GetGPIOBitValue(g_gpio_fd, bit);

	if(ret < 0) {
		DEBUG(DL_ERROR, "GetGPIOBitValue() failed!!!\n ");
		return -1 ;
	}

	return (ret);
}



/******************************************************************************
 * GetCurrentMode
 ******************************************************************************/
int GetCurrentMode(void)
{
	v412_Object 			v412_mode;
	int ret = -1;
	ret = Capture_getVideoMode(gblGetCapture(), &v412_mode);

	if(ret == 0) {
		if(v412_mode.mode < 0) {
			DEBUG(DL_DEBUG, "SetH264Width:%d  SetH264Height:%d\n\n",
			      H264Width(), H264Height());
		} else {
			if(v412_mode.height >= H264_MAX_ENCODE_HIGHT) {
				v412_mode.height = H264_MAX_ENCODE_HIGHT;
			}

			if(v412_mode.width >= H264_MAX_ENCODE_WIDTH) {
				v412_mode.width = H264_MAX_ENCODE_WIDTH;
			}

			if(v412_mode.width != 0) {
				SetH264Width((int)v412_mode.width);
				SetH264Height((int)v412_mode.height);
			}
		}

		gblSetMode(v412_mode.mode);
	} else {
		DEBUG(DL_ERROR, "[GetCurrentMode] Failed error No=%d\n", ret);
	}

	return 0;
}

#if 0
/*limit capture framerate*/
int limitFrame()
{
	struct timeval        tv;
	static	ULong		  prevTime = 0;
	static 	unsigned int  now_frame = 0;
	unsigned int		ctrl_frame = 0;
	ULong				detaTime, newTime, everyTime = 0;

	ctrl_frame = GetCtrlFrame(); //framecnt; //GetCtrlFrame();

	if(ctrl_frame > 60) {
		ctrl_frame = 60;
	}

	if(ctrl_frame < 1) {
		ctrl_frame = 1;
	}

	if(gettimeofday(&tv, NULL) == -1) {
		DEBUG(DL_ERROR, "Failed to get os time\n");
		return 1;
	}

	everyTime = (1000) / (ctrl_frame + 1);
	newTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;

	if(prevTime == 0) {
		prevTime = newTime ;
		now_frame = 0;
		return 1;
	}

	detaTime = (newTime - prevTime);
	//	DEBUG(DL_DEBUG,"ctrl_frame = %d everyTime = %d  detaTime = %d\n",ctrl_frame,everyTime,detaTime);

	if(detaTime > everyTime) {
		prevTime = newTime ;
		//		DEBUG(DL_DEBUG,"ctrl_frame = %d everyTime = %d  detaTime = %d\n",ctrl_frame,everyTime,detaTime);
		return 1;
	} else {
		//		DEBUG(DL_DEBUG,"ctrl_frame = %d everyTime = %d  detaTime = %d\n",ctrl_frame,everyTime,detaTime);
		return 0;
	}

	return 1;
}

/*limit lowbitrate capture framerate*/
int limitLowbitrateFrame()
{
	struct timeval        tv;
	static	ULong		  prev_time = 0;
	static 	unsigned int  now_lowrate_frame = 0;
	unsigned int		ctrl_frame = 0;
	ULong				detaTime, newTime, everyTime = 0;

	ctrl_frame = 30;////GetCtrlLowRateFrame(); //GetCtrlFrame();

	if(ctrl_frame > 60) {
		ctrl_frame = 60;
	}

	if(ctrl_frame < 1) {
		ctrl_frame = 1;
	}

	if(gettimeofday(&tv, NULL) == -1) {
		DEBUG(DL_ERROR, "Failed to get os time\n");
		return 1;
	}

	everyTime = (1000) / (ctrl_frame + 1);
	newTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;

	if(prev_time == 0) {
		prev_time = newTime ;
		now_lowrate_frame = 0;
		return 1;
	}

	detaTime = (newTime - prev_time);
	//	DEBUG(DL_DEBUG,"ctrl_frame = %d everyTime = %d  detaTime = %d\n",ctrl_frame,everyTime,detaTime);

	if(detaTime > everyTime) {
		prev_time = newTime ;
		//		DEBUG(DL_DEBUG,"ctrl_frame = %d everyTime = %d  detaTime = %d\n",ctrl_frame,everyTime,detaTime);
		return 1;
	} else {
		//		DEBUG(DL_DEBUG,"ctrl_frame = %d everyTime = %d  detaTime = %d\n",ctrl_frame,everyTime,detaTime);
		return 0;
	}

	return 1;
}
#endif

int limitCaputreFrameMain(int framecnt)
{
	struct 	timeval        		tv;
	static	ULong		  	prevTime2 = 0;

	unsigned int		ctrl_frame = 0;
	ULong			detaTime, newTime, everyTime = 0;

	ctrl_frame = framecnt;

	if(ctrl_frame > 60) {
		ctrl_frame = 60;
	}

	if(ctrl_frame < 1) {
		ctrl_frame = 1;
	}

	if(gettimeofday(&tv, NULL) == -1) {
		DEBUG(DL_ERROR, "Failed to get os time\n");
		return 1;
	}

	everyTime = (1000) / (ctrl_frame + 1);
	newTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;

	if(prevTime2 == 0) {
		prevTime2 = newTime ;
		return 1;
	}

	detaTime = (newTime - prevTime2);
	//	PRINTF("ctrl_frame = %d everyTime = %d  detaTime = %d\n",ctrl_frame,everyTime,detaTime);

	if(detaTime > everyTime) {
		prevTime2 = newTime ;
#if 0
		static int y = 0;
		y++;

		if(y % 10 == 0) {
			PRINTF("ctrl_frame = %d everyTime = %d  detaTime = %d,yy=%d\n", ctrl_frame, everyTime, detaTime, y);
		}

#endif
		return 1;
	} else {
		//		PRINTF("ctrl_frame = %d everyTime = %d  detaTime = %d\n",ctrl_frame,everyTime,detaTime);
		return 0;
	}

	return 1;
}

/*limit capture framerate*/
int limitCaputreFrame(int framecnt, int num, int initflag)
{

	static	ULong		  	prevTime = 0;
	static 	unsigned int  		now_frame = 0;

	struct timeval        tv;
	unsigned int		ctrl_frame = 0;
	ULong				detaTime, newTime, everyTime = 0;

	ctrl_frame = framecnt;

	if(ctrl_frame > LIMITFRAMES) {
		ctrl_frame = LIMITFRAMES;
	}

	if(ctrl_frame < 1) {
		ctrl_frame = 1;
	}

	if(num == 0) {
		num = 1;
	}

	if(gettimeofday(&tv, NULL) == -1) {
		DEBUG(DL_ERROR, "Failed to get os time\n");
		return 1;
	}

	if(initflag == 1) {
		prevTime = 0;
		now_frame = 0;
	}

#ifdef DSS_ENC_1100
	everyTime = (1000) / (ctrl_frame + 2);
#else
	everyTime = (1000) / (ctrl_frame + 1);
#endif

	newTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;

	if(prevTime == 0) {
		prevTime = newTime ;
		now_frame = 0;
		return 1;
	}

	detaTime = (newTime - prevTime);
	//	PRINTF("ctrl_frame = %d everyTime = %d  detaTime = %d\n",ctrl_frame,everyTime,detaTime);

	if(detaTime > everyTime * num) {
		if(num == ctrl_frame) {
			prevTime = newTime ;
		}

		return 1;
	} else {
		//		PRINTF("ctrl_frame = %d everyTime = %d  detaTime = %d\n",ctrl_frame,everyTime,detaTime);
		return 0;
	}

	return 1;
}


/*limit capture framerate*/
int limitCaputreFrameLow(int framecnt, int num, int initflag)
{

	static	ULong		  	prevTime3 = 0;
	static 	unsigned int  		now_frame3 = 0;

	struct timeval        tv;

	unsigned int		ctrl_frame = 0;
	ULong				detaTime, newTime, everyTime = 0;

	ctrl_frame = framecnt;

	if(ctrl_frame > 60) {
		ctrl_frame = 60;
	}

	if(ctrl_frame < 1) {
		ctrl_frame = 1;
	}

	if(num == 0) {
		num = 1;
	}

	if(gettimeofday(&tv, NULL) == -1) {
		DEBUG(DL_ERROR, "Failed to get os time\n");
		return 1;
	}

	if(initflag == 1) {
		prevTime3 = 0;
		now_frame3 = 0;
	}

	everyTime = (1000) / (ctrl_frame + 1);
	newTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;

	if(prevTime3 == 0) {
		prevTime3 = newTime ;
		now_frame3 = 0;
		return 1;
	}

	detaTime = (newTime - prevTime3);
	//	PRINTF("ctrl_frame = %d everyTime = %d  detaTime = %d\n",ctrl_frame,everyTime,detaTime);

	if(detaTime > everyTime * num) {
		if(num == ctrl_frame) {
			prevTime3 = newTime ;
		}

#if 0
		static int x3 = 0;
		x3++;

		if(x3 % 10 == 0) {
			PRINTF("ctrl_frame = %d everyTime = %d  detaTime = %d,x= %d\n", ctrl_frame, everyTime, detaTime, x);
		}

#endif
		return 1;
	} else {
		//		PRINTF("ctrl_frame = %d everyTime = %d  detaTime = %d\n",ctrl_frame,everyTime,detaTime);
		return 0;
	}

	return 1;
}

/*control run led on/off */
int CtrlRunLed(Capture_Handle hCapture)
{
	static int led_on = 1;
	static int times = 0;
	dmai_gpio_param led_ctrl;

	if(++times >= LED_FREQ_TIMES) {  //two times set led once
		times = 0;
	} else {
		return 0;
	}

	if(hCapture == NULL) {
		DEBUG(DL_WARNING, "[CtrlRunLed] hCapture == NULL!!\n");
		return -1;
	}

	if(led_on == 1) {
		led_ctrl.gpio_num = STATUS_LED_GPIO ;
		led_ctrl.gpio_data = led_on;
		led_on = 0;
		Ctrl_gpio_led(hCapture, &led_ctrl);
	} else {
		led_ctrl.gpio_num = STATUS_LED_GPIO ;
		led_ctrl.gpio_data = led_on;
		led_on = 1;
		Ctrl_gpio_led(hCapture, &led_ctrl);
	}

	return 0;
}

/*control status led on */
int CtrlFreqStatusLed(Capture_Handle hCapture)
{
	static int led_on = 1;
	static int input_status = 0; // 0,wuyuan 1youyuan,invadi 2 ok
	dmai_gpio_param led_ctrl;
	int mode = gblGetMode();

	if(hCapture == NULL) {
		DEBUG(DL_WARNING, "[CtrlRunLed] hCapture == NULL!!\n");
		return -1;
	}

	//PRINTF("gsignal_vsyncF =%d,led_on=%d,input=%d\n",gsignal_vsyncF,led_on,input_status);
	if(mode < 0) {
		if(gsignal_vsyncF > 0) {
			input_status = 1;

			if(led_on == 1) {
				led_ctrl.gpio_num = RUN_LED_GPIO ;
				led_ctrl.gpio_data = led_on;
				led_on = 0;
				Ctrl_gpio_led(hCapture, &led_ctrl);
			} else {
				led_ctrl.gpio_num = RUN_LED_GPIO ;
				led_ctrl.gpio_data = led_on;
				led_on = 1;
				Ctrl_gpio_led(hCapture, &led_ctrl);
			}
		} else {
			if(led_on == 1 || input_status != 0) {
				led_ctrl.gpio_num = RUN_LED_GPIO ;
				led_on = 0;
				led_ctrl.gpio_data = led_on;
				input_status = 0;
				Ctrl_gpio_led(hCapture, &led_ctrl);
			}
		}
	} else {
		input_status = 2;
		led_ctrl.gpio_num = RUN_LED_GPIO ;
		led_on = 1;
		led_ctrl.gpio_data = led_on;
		Ctrl_gpio_led(hCapture, &led_ctrl);
	}

	return 0;
}



int get_videoStd_and_h264_pixel(int mode , VideoStd_Type  *videoStd, int *h264_pixel)
{
	VideoStd_Type 	tmp_std = 0 ;
	int 				tmp_pixel  = 0;

	if(mode < 0) {
		tmp_std = VideoStd_1080P_30;
		tmp_pixel = VIDEO_NO_SOURCE;
		goto END;
	}

	switch(mode) {
		case VIDEO_480I_PIXEL:
			//ENC1100 未进缓冲
			tmp_std = VideoStd_1080I_30;

			tmp_pixel = VIDEO_480P_PIXEL;
			break;

		case VIDEO_576I_PIXEL:

			tmp_std = VideoStd_1080I_25;
			tmp_pixel = VIDEO_576P_PIXEL;
			break;

		case VIDEO_1080P60_PIXEL:
		case VIDEO_1080P30_PIXEL:
			tmp_std = VideoStd_1080P_30;
			tmp_pixel = VIDEO_1080_PIXEL;
			break;

		case VIDEO_1080P50_PIXEL:
		case VIDEO_1080P25_PIXEL:
			tmp_std = VideoStd_1080P_25;
			tmp_pixel = VIDEO_1080_PIXEL;
			break;

		case VIDEO_1080I30_PIXEL:
			tmp_std = VideoStd_1080I_30;
			tmp_pixel = VIDEO_1080_PIXEL;
			break;

		case VIDEO_1080I25_PIXEL:
			tmp_std = VideoStd_1080I_25;
			tmp_pixel = VIDEO_1080_PIXEL;
			break;

		case VIDEO_720P60_PIXEL:
			tmp_std = VideoStd_720P_60;
			tmp_pixel = VIDEO_720_PIXEL;
			break;

		case VIDEO_720P50_PIXEL:
			tmp_std = VideoStd_720P_50;
			tmp_pixel = VIDEO_720_PIXEL;
			break;

		default:
			tmp_std = VideoStd_1080P_30;
			tmp_pixel = VIDEO_VGA_PIXEL;
			break;

	}

END:
	*videoStd = tmp_std;
	*h264_pixel = tmp_pixel;
	return 1;
}


#define MAX_CAP_COUNT  50
/******************************************************************************
 * 改变采集方式重启创建VP 口和采集buffer
 ******************************************************************************/
int mChangeCapture(BufTab_Handle *phBufTab, BufferGfx_Attrs *pAttrs, Capture_Attrs *pcAttrs, VideoStd_Type videoStd, int mode)
{
	int 					h264_pixel 	= VIDEO_INVALID;
	Int32                   bufSize;
	get_videoStd_and_h264_pixel(mode, &videoStd, &h264_pixel);


	//gblSetH264pixel(h264_pixel);
	DEBUG(DL_DEBUG, "current setting capture standard  videoStd= %d h264_pixel= %d, hBufTab = %p\n"
	      , videoStd, h264_pixel, *phBufTab);

	/*delete buffer table*/
	if(*phBufTab) {
		BufTab_delete(*phBufTab);
	}

	if(gblGetCapture()) {
		Capture_delete(gblGetCapture());
		gblSetCapture(NULL);
	}

	/* Calculate the dimensions of a video standard given a color space */
	if(BufferGfx_calcDimensions(videoStd,
	                            ColorSpace_YUV422PSEMI, &pAttrs->dim) < 0) {
		DEBUG(DL_ERROR, "Failed to calculate Buffer dimensions\n");
		return -1;
	}

	/* Calculate buffer size needed of a video standard given a color space */
	bufSize = BufferGfx_calcSize(videoStd, ColorSpace_YUV422PSEMI);

	if(bufSize < 0) {
		DEBUG(DL_ERROR, "Failed to calculate size for capture driver buffers\n");
		return -1;
	}

	*phBufTab = BufTab_create(NUM_CAPTURE_BUFS, bufSize,
	                          BufferGfx_getBufferAttrs(pAttrs));

	if(*phBufTab == NULL) {
		DEBUG(DL_ERROR, "Failed to allocate contiguous buffers\n");
		return -1;
	}

	//PRINTF("the display the width=%d,height=%d,linlen=%d\n",
	//	pAttrs->dim.width,pAttrs->dim.height,pAttrs->dim.lineLength)	;
	writeWatchDog();
#ifdef CL4000_SDI
	pcAttrs->videoStd = videoStd;
#endif

	writeWatchDog();
	DEBUG(DL_ERROR, "[mChangeCapture]  Capture_create before .cAttrs.decoderIdx=%d,videostd=%d..\n",pcAttrs->decoderIdx,pcAttrs->videoStd);
	Capture_Handle cap_h = Capture_create(*phBufTab, pcAttrs);
	DEBUG(DL_ERROR, "[mChangeCapture]   Capture_create after ...\n");

	if(NULL == cap_h) {
		DEBUG(DL_ERROR, "\n\n\n\n\n\n[mChangeCapture] ################### Capture_create is NULL!!!\n\n\n\n\n\n\n");
		return -1;
	}

	gblSetCapture(cap_h);

	if(gblGetCapture() ==  NULL) {
		DEBUG(DL_ERROR, "[mChangeCapture]  gblGetCapture NULL!!!!!\n");
		return -1;
	}

	InitRevisePicture();
#ifdef CL4000_DVI
	change_green_screen_adjust(gblGetMode());
	change_screen_adjust(gblGetMode());
#endif
#ifdef DSS_ENC_1100_1200
	change_green_screen_adjust(gblGetMode());
#endif
	//  gblSetH264pixel(h264_pixel);
	writeWatchDog();
	DEBUG(DL_DEBUG, "create capture succeed!!!!!\n");
	return 0;
}

int mResetCapture(BufTab_Handle hBufTab, BufferGfx_Attrs *pAttrs, Capture_Attrs *pcAttrs)
{
	Int32   bufSize;
	VideoStd_Type videoStd = Capture_getVideoStd(gblGetCapture());

	if(hBufTab) {
		BufTab_delete(hBufTab);
	}

	/* Calculate the dimensions of a video standard given a color space */
	if(BufferGfx_calcDimensions(videoStd, ColorSpace_YUV422PSEMI, &pAttrs->dim) < 0) {
		DEBUG(DL_ERROR, "Failed to calculate Buffer dimensions\n");
		return -1;
	}

	/* Calculate buffer size needed of a video standard given a color space */
	bufSize = BufferGfx_calcSize(videoStd, ColorSpace_YUV422PSEMI);

	if(bufSize < 0) {
		DEBUG(DL_ERROR, "Failed to calculate size for capture driver buffers\n");
		return -1;
	}

	hBufTab = BufTab_create(NUM_CAPTURE_BUFS, bufSize,
	                        BufferGfx_getBufferAttrs(pAttrs));

	if(hBufTab == NULL) {
		DEBUG(DL_ERROR, "Failed to allocate contiguous buffers\n");
		return -1;
	}

	writeWatchDog();
	gblCaptureLock();

	if(gblGetCapture()) {
		Capture_delete(gblGetCapture());
		gblSetCapture(NULL);
	}

	writeWatchDog();
	gblSetCapture(Capture_create(hBufTab, pcAttrs));

	if(gblGetCapture() ==  NULL) {
		DEBUG(DL_ERROR, "[mResetCapture]  Capture gblGetCapture NULL!!!!!\n");
		gblCaptureunLock();
		return -1;
	}

	writeWatchDog();
	DEBUG(DL_DEBUG, "create capture succeed!!!!!\n");
	gblCaptureunLock();
	return 0;
}

/*设置绿屏和hv,每次切换源必须调用*/
int mSetCapture()
{
	PRINTF("\n");
	InitRevisePicture();
#ifdef CL4000_DVI
	change_green_screen_adjust(gblGetMode());
	change_screen_adjust(gblGetMode());
#endif
#ifdef DSS_ENC_1100_1200
	change_green_screen_adjust(gblGetMode());
#endif
	return 0;
}
int Deinterlace(VIDDEC2_Handle *hpVe, Buffer_Handle hDstBuf, int mode)
{
	Void   *status				= THREAD_SUCCESS;
	VIDDEC2_Handle hVe = (*hpVe);

	if(NULL == hDstBuf) {
		DEBUG(DL_ERROR, "[Deinterlace] Failed hDstBuf is NULL!!!\n");
		return -1;
	}


	static volatile VIDDEC2_Status				encStatus;
	static volatile VIDDEC2_DynamicParams		encDynParams;

	/* declare codec I/O buffer descriptors for the codec's process() func. */
	XDM1_BufDesc      inBufDesc;
	XDM_BufDesc      encodedBufDesc;
	/* declare in and out argument descriptors for process() */
	VIDDEC2_InArgs    encoderInArgs;
	VIDDEC2_OutArgs   encoderOutArgs;

	/* declare arrays describing I/O buffers and their sizes */
	XDM1_SingleBufDesc inSingBuf;
	XDAS_Int8       *encodedBufs    [ XDM_MAX_IO_BUFFERS ];
	XDAS_Int32       encodedBufSizes[ XDM_MAX_IO_BUFFERS ];
#if 1

	if(mode == MODE_1080I50 || mode == MODE_1080I60) {
		encDynParams.decodeHeader = 1080; //height;
		encDynParams.displayWidth = 1920; //width;
		encDynParams.frameSkipMode = 3133440 * 2 / 3;
		encDynParams.mbDataFlag = 1920;
		encDynParams.newFrameFlag = 1;
		encDynParams.frameOrder = 1;

	} else if(mode == MODE_480I60) {
		encDynParams.decodeHeader = 480; //height;
		encDynParams.displayWidth = 720; //width;
		encDynParams.frameSkipMode = 3133440 * 2 / 3;
		encDynParams.mbDataFlag = 1920;
		encDynParams.newFrameFlag = 1;
		encDynParams.frameOrder = 1;
	} else if(mode == MODE_576I50) {
		encDynParams.decodeHeader = 576; //height;
		encDynParams.displayWidth = 720; //width;
		encDynParams.frameSkipMode = 3133440 * 2 / 3;
		encDynParams.mbDataFlag = 1920;
		encDynParams.newFrameFlag = 1;
		encDynParams.frameOrder = 1;
	} else {
		DEBUG(DL_ERROR, "[Deinterlace] mode Error = %d \n", mode);
		return -1;
	}

#endif

	encDynParams.size = sizeof(encDynParams);
	encStatus.size = sizeof(encStatus);
	status = VIDDEC2_control(hVe, XDM_SETPARAMS, &encDynParams, &encStatus);

	if(status != VIDDEC2_EOK) {
		DEBUG(DL_ERROR, "[VIDDEC2_control] Failed to VIDENC_control\n");
		return -1;
	}

	/* define the arrays describing I/O buffers and their sizes */
	inSingBuf.buf       = (XDAS_Int8 *)Buffer_getUserPtr(hDstBuf);
	inSingBuf.bufSize	= 1920 * 1088 * 3 / 2;
	inSingBuf.accessMask =	XDM_ACCESSMODE_READ;

	encodedBufs[0]  = (XDAS_Int8 *)Buffer_getUserPtr(hDstBuf);
	encodedBufSizes[0]  = 1920 * 1088 * 3 / 2;

	/* define I/O buffer descriptors using lengths and addrs of arrays above */

	inBufDesc.numBufs       = 1;
	inBufDesc.descs[0]      = inSingBuf;

	encodedBufDesc.numBufs  = 1;
	encodedBufDesc.bufs     = encodedBufs;
	encodedBufDesc.bufSizes = encodedBufSizes;
	/* fill in the input arguments structure; we have nothing for this case */
	encoderInArgs.size = sizeof(encoderInArgs);
	encoderOutArgs.size = sizeof(encoderOutArgs);
	status = (void *) VIDDEC2_process(hVe, &inBufDesc, &encodedBufDesc, &encoderInArgs, &encoderOutArgs);

	if(status  != VIDDEC2_EOK) {
		DEBUG(DL_ERROR, "VIDDEC2_process() failed, status = %d\n", (int)status);
		return -1;
	}

	return 0;
}

int write_yuv_422(Buffer_Handle hCapBuf, int height, int width)
{

	FILE *fp;
	unsigned char *pY, *pU, *pV;
	unsigned int i;
	static int time = 0;
	char buf[100];

	sprintf(buf, "./%dx%d_%d.yuv", width, height, time++);
	//    if(time != 50)
	//       return 1;
	fp = fopen(buf, "wb");
	pY = (unsigned char *)Buffer_getUserPtr(hCapBuf);
	pU = (unsigned char *)Buffer_getUserPtr(hCapBuf) + width * height;
	pV = (unsigned char *)Buffer_getUserPtr(hCapBuf) + width * height + 1;
	
	PRINTF("fp=%p,pY=%p=%p=%p\n",fp,pY,pU,pV);
	for(i = 0; i < height * width; i++) {
		fputc(*pY, fp);
		pY++;
	}

#if 0
	for(i = 0; i < height * width / 2; i++) {
		fputc(*pU, fp);
		pU++;
		pU++;
	}

	for(i = 0; i < height * width / 2; i++) {
		fputc(*pV, fp);
		pV++;
		pV++;
	}
#endif
	fclose(fp);
	DEBUG(DL_DEBUG, "write over  height=%d   width=%d   time = %d\n", height, width, time);
	return 1;
}


int gCaptureFlag = 0;

/*VP口采集数据-YUV422格式*/
void *captureThrFxn(Void *arg)
{
	DEBUG(DL_DEBUG, "[captureThrFxn] start ..%d.\n", getpid());
	CaptureEnv             *envp            = (CaptureEnv *) arg;
	Void                   *status          = THREAD_SUCCESS;
	Capture_Attrs           cAttrs          = Capture_Attrs_DM6467_DEFAULT;
	BufferGfx_Attrs         gfxAttrs        = BufferGfx_Attrs_DEFAULT;
	Ccv_Attrs               ccvAttrs        = Ccv_Attrs_DEFAULT;
	Ccv_Handle              hCcv            = NULL;
	BufTab_Handle           hBufTab         = NULL;
	BufferGfx_Dimensions    srcDim, dstDim;
	//BufferGfx_Dimensions 	testdim;
	Buffer_Handle           hDstBuf, hCapBuf ;
	Buffer_Handle			hDstBufResize;
	VideoStd_Type           videoStd;
	Int32                   bufSize;
	Int                     fifoRet, ret, capture_ret = -1;
	Int                     bufIdx, mode = -3, videoMode = 0;
	static int change_mode_count = 0;
	static int Capture_get_failed_count = 0;

	int  cap_count = 0;
	Buffer_Handle           hTempBuf = NULL;
	Logo_Handle				logoenv = NULL;
	textinfo				*textenv = NULL;

#ifdef HAVE_QUALITY_MODULE
	int 					resize_ret = 0;
	int 					quality_flag = 0;

	Buffer_Handle           hQualitybResize	= NULL;
	Resize_Handle   		hRsz			= NULL;
	Resize_Attrs            rszAttrs        = Resize_Attrs_DEFAULT;
	int encodeIdx = AD9880_INDEX;

#endif

#if VIDENC_COPY
	VIDDEC2_Handle 			hVe					= NULL;
	Engine_Handle           hEngine             = NULL;
	static String 			encoderName  = "viddec2_copy";
#endif

#ifdef CL4000_SDI
	encodeIdx = SDI_INDEX;
#elif  CL4000_DVI
	encodeIdx = AD9880_INDEX;
#else
	encodeIdx = gblGetIDX();
#endif

	/*限帧参数*/
	int frame_init = 0;
	int new_high_video_frame = 0;
	int old_high_video_frame = 0;
	int high_frame_ok = 0;
	int high_num = 1 ;

	int new_low_video_frame = 0;
	int old_low_video_frame = 0;
	int low_frame_ok = 0;
	int low_num = 1 ;

	unsigned int capture_time = 0;

	/* Calculate the dimensions of a video standard given a color space */
	if(BufferGfx_calcDimensions(VideoStd_1080P_30,
	                            ColorSpace_YUV422PSEMI, &gfxAttrs.dim) < 0) {
		DEBUG(DL_ERROR, "Failed to calculate Buffer dimensions\n");
		cleanup(THREAD_FAILURE);
	}

	/* Calculate buffer size needed of a video standard given a color space */
	bufSize = BufferGfx_calcSize(VideoStd_1080P_30, ColorSpace_YUV422PSEMI);

	if(bufSize < 0) {
		DEBUG(DL_ERROR, "Failed to calculate size for capture driver buffers\n");
		cleanup(THREAD_FAILURE);
	}

	// CtrlRunLed(gblGetCapture());
	writeWatchDog();
	/* Create a table of buffers to use with the device drivers */
	gfxAttrs.colorSpace = ColorSpace_YUV422PSEMI;
	hBufTab = BufTab_create(NUM_CAPTURE_BUFS, bufSize,
	                        BufferGfx_getBufferAttrs(&gfxAttrs));

	if(hBufTab == NULL) {
		DEBUG(DL_ERROR, "Failed to allocate contiguous buffers\n");
		cleanup(THREAD_FAILURE);
	}

#if VIDENC_COPY
	hEngine = Engine_open("encode", NULL, NULL);

	if(hEngine == NULL) {
		DEBUG(DL_ERROR, "Failed to open codec engine %s\n", "encode");
		cleanup(THREAD_FAILURE);
	}

	hVe = VIDDEC2_create(hEngine, encoderName, NULL);

	if(hVe == NULL) {
		DEBUG(DL_ERROR, "Failed to VIDENC_create\n");
		cleanup(THREAD_FAILURE);
	}

#endif
	writeWatchDog();
	/* Create capture device driver instance */
	cAttrs.numBufs = NUM_CAPTURE_BUFS;
	cAttrs.decoderIdx = encodeIdx;
	DEBUG(DL_DEBUG, "00cAttrs.decoderIdx = %d \n", cAttrs.decoderIdx);
	writeWatchDog();
	gblCaptureLock();
	gblSetCapture(Capture_create(hBufTab, &cAttrs));

	if(NULL == gblGetCapture()) {
		DEBUG(DL_ERROR, "Failed to create capture device\n");
		gblCaptureunLock();
		cleanup(THREAD_FAILURE);
	}

	DEBUG(DL_DEBUG, "11cAttrs.decoderIdx = %d \n", cAttrs.decoderIdx);
	gblCaptureunLock();
	writeWatchDog();
	GetCurrentMode();
	videoStd = Capture_getVideoStd(gblGetCapture());
	/* Create color conversion job for 422Psemi to 420Psemi conversion */
	ccvAttrs.accel = TRUE;
	hCcv = Ccv_create(&ccvAttrs);
	DEBUG(DL_DEBUG, "22cAttrs.decoderIdx = %d \n", cAttrs.decoderIdx);

	if(hCcv == NULL) {
		DEBUG(DL_ERROR, "Failed to create color conversion job\n");
		cleanup(THREAD_FAILURE);
	}

#ifdef HAVE_QUALITY_MODULE
	/* Create the resize job */
	hRsz = Resize_create(&rszAttrs);

	if(hRsz == NULL) {
		DEBUG(DL_ERROR, "[captureThrFxn] Failed to create resize job\n");
		cleanup(THREAD_FAILURE);
	}

#if 1
	hQualitybResize = CreateBuffer(1920, 1088, ColorSpace_YUV420PSEMI);

	if(hQualitybResize == NULL) {
		DEBUG(DL_ERROR, "Failed to allocate contiguous buffers\n");
		cleanup(THREAD_FAILURE);
	}

#endif
#endif

	DEBUG(DL_DEBUG, "33cAttrs.decoderIdx = %d \n", cAttrs.decoderIdx);
	/* Get a buffer from the video thread */
	fifoRet = Fifo_get(envp->from_video_c, &hDstBuf);

	if(fifoRet < 0) {
		DEBUG(DL_ERROR, "Failed to get buffer from video thread\n");
		cleanup(THREAD_FAILURE);
	}

	DEBUG(DL_DEBUG, "44cAttrs.decoderIdx = %d \n", cAttrs.decoderIdx);
	/* Get a buffer from the video thread *///ddd
	fifoRet = Fifo_get(envp->from_resize_c, &hDstBufResize);

	if(fifoRet < 0) {
		DEBUG(DL_ERROR, "[captureThrFxn] Failed to get buffer from video thread\n");
		cleanup(THREAD_FAILURE);
	}

	DEBUG(DL_DEBUG, "55cAttrs.decoderIdx = %d \n", cAttrs.decoderIdx);

	/* Did the video thread flush the fifo? */
	if(fifoRet == Dmai_EFLUSH) {
		cleanup(THREAD_SUCCESS);
	}

	BufferGfx_getDimensions(hDstBuf, &dstDim);

	/* Only process the part of the image which is going to be encoded */
	for(bufIdx = 0; bufIdx < NUM_CAPTURE_BUFS; bufIdx++) {
		hCapBuf = BufTab_getBuf(hBufTab, bufIdx);
		BufferGfx_getDimensions(hCapBuf, &srcDim);
		srcDim.x = 0 ;
		srcDim.y = 0 ;
		DEBUG(DL_DEBUG, "srcDim.width:%d  height:%d\n", (int)srcDim.width, (int)srcDim.height);
		BufferGfx_setDimensions(hCapBuf, &srcDim);
	}

	/* Signal that initialization is done and wait for other threads */
	Rendezvous_meet(envp->hRendezvousInit);
	writeWatchDog();
	gblSetPortStart();



	while(!gblGetQuit()) {
		if(IIS_FPGA_UPDATE()) {
			if(app_get_reboot_flag() != 1) {
				writeWatchDog();
			}

			sleep(1);
			continue;
		}

		/*在720P 60fps 模式下不限制帧率*/
		if((gblGetMode() == VIDEO_720P50_PIXEL || gblGetMode() == VIDEO_720P60_PIXEL) && (60 == GetCtrlFrame())) {
			;
		} else {
			/*limit capture frame rate*/
			ret = limitCaputreFrameMain(GetCtrlFrame());

			if(ret == 0) {
				usleep(1000);
				writeWatchDog();
				continue;
			}
		}

		gCaptureFlag = 2;

		/*高码率限帧函数*/
		{
			frame_init = 0;
			new_high_video_frame = GetCtrlFrame();

			if(new_high_video_frame != old_high_video_frame) {
				frame_init = 1;
				high_num  = 1;
				DEBUG(DL_ERROR, "high frame : old = <%d>, new = <%d>\n", old_high_video_frame, new_high_video_frame);
			}

			old_high_video_frame = new_high_video_frame;
			high_frame_ok = limitCaputreFrame(new_high_video_frame, high_num, frame_init);

			if(high_frame_ok == 1) {
				high_num ++;
			}

			if(high_num > new_high_video_frame) {
				high_num = 1;
			}

		}
		/*低码率限帧函数*/
		{
			frame_init = 0;
			new_low_video_frame = GetCtrlLowRateFrame();

			if(new_low_video_frame != old_low_video_frame) {
				frame_init = 1;
				low_num  = 1;
				DEBUG(DL_ERROR, "low frame old = %d,new=%d,\n", old_low_video_frame, new_low_video_frame);
			}

			old_low_video_frame = new_low_video_frame;
			low_frame_ok = limitCaputreFrameLow(new_low_video_frame, low_num, frame_init);

			if(low_frame_ok == 1) {
				low_num ++;
			}

			if(low_num > new_low_video_frame) {
				low_num = 1;
			}
		}


		gCaptureFlag = 3;

		writeWatchDog();
		gblCaptureLock();
#ifdef DSS_ENC_1100_1200
		CtrlFreqStatusLed(gblGetCapture());
#endif
		CtrlRunLed(gblGetCapture());
		gblCaptureunLock();
		//        DEBUG(DL_DEBUG,"[captureThrFxn]------------------------------------ gblGetCapture = [%p] \n", gblGetCapture());
		writeWatchDog();
		gCaptureFlag = 4;
		videoMode = gblGetMode();
#ifdef DSS_ENC_1100_1200

		if(videoMode != mode)
#else
		if(videoMode != mode)
#endif
		{
			DEBUG(DL_DEBUG, "[captureThrFxn]+++++++++++++++++New Mode = [%d], Old Mode = %d, change mode count : [ %d ],encodeidx=%d\n"
			      , videoMode, mode, change_mode_count ++, gblGetIDX());

			mode = videoMode;
#ifdef DSS_ENC_1100_1200

			if(mode < 0)
#else
			if(mode < 0)
#endif
			{
				//gblCaptureLock();
				goto NORET_VPIF;
			}


			gCaptureFlag = 5;

			mSetCapture();
			cap_count = 0;
			gCaptureFlag = 6;
		}

		if(gblGetIDX() !=  encodeIdx || capture_get_i2c_error() == 1) {
			gblCaptureLock();
#ifdef DSS_ENC_1100_1200
			encodeIdx = cAttrs.decoderIdx  = gblGetIDX();
#endif
			//pthread_mutex_lock(&gCapEnc_m);
			ret = mChangeCapture(&hBufTab, &gfxAttrs, &cAttrs, videoStd, mode);
			//pthread_mutex_unlock(&gCapEnc_m);
			gblCaptureunLock();
			writeWatchDog();
			
			if(0 > ret) {
				DEBUG(DL_ERROR, "[captureThrFxn]  000 mChangeCapture failed count : [%d]!!!\n", cap_count);
				usleep(20000);

				if(cap_count ++ > MAX_CAP_COUNT) {

					cleanup(THREAD_FAILURE);
				}

				continue;
			}
			capture_set_i2c_error(0);	
		}

		writeWatchDog();

		if(videoMode >= 0) {
			gCaptureFlag = 7;
			gblCaptureLock();
			gCaptureFlag = 8;
			capture_ret = Capture_get(gblGetCapture(), &hCapBuf);

			//	write_yuv_422(hCapBuf, 1080, 1920);
		//	BufferGfx_getDimensions(hCapBuf, &testdim);
		//	PRINTF("capbuf w=%d,h=%d\n",testdim.width,testdim.height);
			
			gblCaptureunLock();

			if(capture_ret < 0) {
				DEBUG(DL_ERROR, "capture get failed!!!!\n");
			}
		} else {
			capture_ret = -1;
		}


		gCaptureFlag = 9;



		if(capture_ret < 0) {
	
			if(videoMode >= 0) {
				if(Capture_get_failed_count >MAX_CAP_COUNT/2)
				{
					//add by zm,if capture get failed ,maybe need sleep.
					usleep(500000);
				}

								
				if(MAX_CAP_COUNT == Capture_get_failed_count ++) {
					DEBUG(DL_ERROR, "capture get failed MAX_CAP_COUNT!!!!\n");
					cleanup(THREAD_FAILURE);
				}

				gCaptureFlag = 10;

				DEBUG(DL_ERROR, "videoMode = %d, capture get failed!!!!\n", videoMode);
				gblCaptureLock();
				GetCurrentMode();
				gblCaptureunLock();

				if(gblGetMode() >= 0) {
					gCaptureFlag = 11;
					gblCaptureLock();
					//pthread_mutex_lock(&gCapEnc_m);
					gCaptureFlag = 12;
					ret = mChangeCapture(&hBufTab, &gfxAttrs, &cAttrs, videoStd, videoMode);
					//pthread_mutex_unlock(&gCapEnc_m);
					gblCaptureunLock();
				}
			}

		NORET_VPIF:

			if(videoMode < 0) {
				gCaptureFlag = 13;

				if(IISLOWRATESTART()) {
					fifoRet = Fifo_put(envp->to_resize_c, hDstBufResize);

					if(fifoRet < 0) {
						ERR("[captureThrFxn] Failed to send buffer to display thread\n");
						cleanup(THREAD_FAILURE);
					}

					fifoRet = Fifo_get(envp->from_resize_c, &hDstBufResize);

					if(fifoRet < 0) {
						ERR("[captureThrFxn] Failed to get buffer from video thread\n");
						cleanup(THREAD_FAILURE);
					}
				}

				gCaptureFlag = 14;

				if(Fifo_put(envp->to_video_c, hDstBuf) < 0) {
					DEBUG(DL_ERROR, "Failed to send buffer to display thread\n");
					cleanup(THREAD_FAILURE);
				}

				fifoRet = Fifo_get(envp->from_video_c, &hDstBuf);

				if(fifoRet < 0) {
					DEBUG(DL_ERROR, "Failed to get buffer from video thread\n");
					cleanup(THREAD_FAILURE);
				}

			}

			gCaptureFlag = 72;
			usleep(4000);
			gCaptureFlag = 76;
			continue;
		}

		Capture_get_failed_count = 0;
		// DEBUG(DL_WARNING,"VPIF re configure succeed !!!!!!!!!mode ---------------------[%d]\n", mode);

		logoenv = getLogoInfoHandle();
		textenv = getTextInfoHandle();

		if(app_get_textshow_flag() == 1) {
			addtextosdtoender(hCapBuf);
		}

		if(app_get_logoshow_flag() == 1) {
			appShowLogo(logoenv, hCapBuf, gblGetMode());
		}

		//	if(getShowLogoTextFlag() == NOdisplay)
		//	{
		//
		//	}
		//	else if(getShowLogoTextFlag() == OnlyShowText) {
		//		addtextosdtoender(hCapBuf);
		//	} else if(getShowLogoTextFlag() == OnlyShowLogo) {
		//		appShowLogo(logoenv, hCapBuf, gblGetMode());
		//	} else {
		//PRINTF("logoenv x = %d,y=%d,alpha=%d,enable=%d\n",
		//       logoenv->x, logoenv->y, logoenv->alpha, logoenv->enable);
		//		addtextosdtoender(hCapBuf);
		//		appShowLogo(logoenv, hCapBuf, gblGetMode());
		//	}

#if VIDENC_COPY

		//	mode = gblGetMode();
		if(gblGetMode() == MODE_1080I50 || gblGetMode() == MODE_1080I60 ||
		   gblGetMode() == MODE_480I60 || gblGetMode() == MODE_576I50) {
			Deinterlace(&hVe, hCapBuf, gblGetMode());
		}

#endif





		if(1 == high_frame_ok) {
			hTempBuf = hDstBuf;

#ifdef HAVE_QUALITY_MODULE
			quality_flag = 0;

			if(AppIsResizeFlag() && ISZOOMOUT) {
				//if(AppgetIISResize() == 0)
				{
					hTempBuf = hQualitybResize;
					quality_flag = 1;
				}
			}

#endif

			if(Ccv_config(hCcv, hCapBuf, hTempBuf) < 0) {
				DEBUG(DL_ERROR, "Failed to configure color conversion job\n");
				cleanup(THREAD_FAILURE);
			}

			/*将YUV 422转YUV 420 */
			if(Ccv_execute(hCcv, hCapBuf, hTempBuf) < 0) {
				DEBUG(DL_ERROR, "Failed to execute color conversion job\n");
				cleanup(THREAD_FAILURE);
			}

#ifdef HAVE_QUALITY_MODULE

			//need qualityresize
			if(quality_flag == 1 && hTempBuf == hQualitybResize) {
				resize_ret = AppQuality420YUV(hRsz, hQualitybResize, hDstBuf);

				if(resize_ret < 0) {
					cleanup(THREAD_FAILURE);
				}
			}

#endif
		//	BufferGfx_getDimensions(hCapBuf, &testdim);
		//	PRINTF("capbuf w=%d,h=%d\n",testdim.width,testdim.height);
			//BufferGfx_getDimensions(hDstBuf, &testdim);
			//PRINTF("hDstBuf w=%d,h=%d\n",testdim.width,testdim.height);
			/* 将YUV 420的buffer还给编码线程 */
			if(Fifo_put(envp->to_video_c, hDstBuf) < 0) {
				DEBUG(DL_ERROR, "Failed to send buffer to display thread\n");
				cleanup(THREAD_FAILURE);
			}

			/* Get a buffer from the video thread */
			fifoRet = Fifo_get(envp->from_video_c, &hDstBuf);

			if(fifoRet < 0) {
				DEBUG(DL_ERROR, "Failed to get buffer from video thread\n");
				cleanup(THREAD_FAILURE);
			}

			/* Did the video thread flush the fifo? */
			if(fifoRet == Dmai_EFLUSH) {
				cleanup(THREAD_SUCCESS);
			}
		}


		if(IISLOWRATESTART() && (1 == low_frame_ok)) {
			/* Configure color conversion job */
			if(Ccv_config(hCcv, hCapBuf, hDstBufResize) < 0) {
				DEBUG(DL_ERROR, "[captureThrFxn] Failed to configure color conversion job\n");
				cleanup(THREAD_FAILURE);
			}

			/* Color convert the captured buffer from 422Psemi to 420Psemi */
			if(Ccv_execute(hCcv, hCapBuf, hDstBufResize) < 0) {
				DEBUG(DL_ERROR, "[captureThrFxn] Failed to execute color conversion job\n");
				cleanup(THREAD_FAILURE);
			}

			/* Send color converted buffer to video thread for encoding */
			fifoRet = Fifo_put(envp->to_resize_c, hDstBufResize);

			if(fifoRet < 0) {
				DEBUG(DL_ERROR, "[captureThrFxn] Failed to send buffer to display thread\n");
				cleanup(THREAD_FAILURE);
			}

			/* Get a buffer from the video thread */
			fifoRet = Fifo_get(envp->from_resize_c, &hDstBufResize);

			if(fifoRet < 0) {
				DEBUG(DL_ERROR, "[captureThrFxn] Failed to get buffer from video thread\n");
				cleanup(THREAD_FAILURE);
			}

			/* Did the video thread flush the fifo? */
			if(fifoRet == Dmai_EFLUSH) {
				cleanup(THREAD_SUCCESS);
			}
		}

		gCaptureFlag = 15;

		/* 将采集buffer归还给采集设备 */
		gblCaptureLock();
		gCaptureFlag = 16;
		ret = Capture_put(gblGetCapture(), hCapBuf);
		gblCaptureunLock();

		gCaptureFlag = 17;

		if(ret < 0) {
			DEBUG(DL_ERROR, "Failed to put capture buffer\n");
			cleanup(THREAD_FAILURE);
		}

		/* Incremement statistics for the user interface */
		gblIncFrames();

	}

cleanup:
	DEBUG(DL_DEBUG, "[captureThrFxn] cleanup XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
	/* Make sure the other threads aren't waiting for us */
	Rendezvous_force(envp->hRendezvousInit);
	Fifo_flush(envp->to_video_c);
	/* Meet up with other threads before cleaning up */
	Rendezvous_meet(envp->hRendezvousCleanup);
	gblCaptureLock();

	if(gblGetCapture()) {
		Capture_delete(gblGetCapture());
		gblSetCapture(0);
	}

	gblCaptureunLock();

	if(hBufTab) {
		BufTab_delete(hBufTab);
	}

	if(hCcv) {
		Ccv_delete(hCcv);
	}
	system("reboot -f");
	DEBUG(DL_DEBUG, "[captureThrFxn] cleanup XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX end\n");
	return status;
}


/******************************************************************************
 * DetectThrFxn
 ******************************************************************************/
void *detectThrFxn(Void *arg)
{

	SIGNAL_STA		op;
	PRINTF("get pid= %d\n", getpid());
	DEBUG(DL_DEBUG, "[detectThrFxn] start ...\n");
	DetectEnv          *envp   			= (DetectEnv *) arg;
	v412_Handle			v412_mode 		= NULL;
	int 				oldmode 		= -3;
	int 				ret 			= -1;
	Void                *status			= THREAD_SUCCESS;
	char 				ipaddr[20];
	struct in_addr 		addr1;
	int					i = 0;
	int 				resizemode = 0, tmp  = 0;
	int 				source_size = -1;
	//	SIGNAL_STA		op;
	int invaild_flag = 1;
	memcpy(&addr1, &ipaddr, 4);
	strcpy(ipaddr, inet_ntoa(addr1));
	v412_mode = malloc(sizeof(v412_Object));
	memset(v412_mode, 0, sizeof(v412_Object));
	/*Signal that initialization is done and wait for other threads*/
	Rendezvous_meet(envp->hRendezvousInit);

	while(!gblGetQuit()) {
		if(IIS_FPGA_UPDATE()) {
			sleep(1);
			continue;
		}

		gblCaptureLock();

		if(NULL != gblGetCapture()) {
			ret = Capture_getVideoMode(gblGetCapture(), v412_mode);
			gblCaptureunLock();
	//	DEBUG(DL_DEBUG, "detectThrFxn: v412_mode->width:%d height:%d h= %d v=%d mode=%d,v412_mode->ad9880_mode=0x%02x\n",
	// 	v412_mode->width, v412_mode->height, v412_mode->hporch,
	//	  v412_mode->vporch, v412_mode->mode,v412_mode->ad9880_mode);
#if 1

			if(ret == 0) {
				if(v412_mode->ad9880_mode == 0xaa)
				{
					capture_set_i2c_error(1);
				}
				
				if(v412_mode->mode  < 0) {
					if(invaild_flag == 1) {
						getResizeMode(tmp, &resizemode);
						source_size = getOutputResolution();

						if(source_size >= LOCK_BEGIN && source_size < LOCK_MAX) {
							PRINTF("gResizeP.req_w=%d,gResizeP.req_h=%d\n", gResizeP.enc_w, gResizeP.enc_h);
							calcLockResolution(source_size);
							CalcReaolution(gResizeP.enc_w, gResizeP.enc_h, capture_width, capture_height, 1, resizemode);

						}

						invaild_flag = 0;
					}

					gblSetMode(v412_mode->mode);

					DEBUG(DL_DEBUG, "source is't connected!!!:%d\n", gblGetMode());
				} else {
#if (!defined(DSS_ENC_1200 ) && !defined(DSS_ENC_1260))

					//#ifndef DSS_ENC_1200
					if(gblGetIDX() == SDI_INDEX) {
						if(oldmode != 0 && v412_mode->mode != oldmode) {
							DEBUG(DL_WARNING, "002 restart the detectThrFxn 002 \n");
							gblCaptureunLock();
							cleanup(THREAD_SUCCESS);
						}
					}

#endif

					if(v412_mode->height >= H264_MAX_ENCODE_HIGHT) {
						v412_mode->height = H264_MAX_ENCODE_HIGHT;
					}

					if(v412_mode->width > H264_MAX_ENCODE_WIDTH) {
						v412_mode->width = H264_MAX_ENCODE_WIDTH;
					}


					if(v412_mode->width != 0) {
						getResizeMode(tmp, &resizemode);
						source_size = getOutputResolution();
						SetH264Width((int)v412_mode->width);
						SetH264Height((int)v412_mode->height);

						if((invaild_flag == 1)  || (capture_width != (int)v412_mode->width) || (capture_height != (int)v412_mode->height)) {
							//每次侦测到模式后赋值到全局视频宽高参数
							capture_width = (int)v412_mode->width;
							capture_height = (int)v412_mode->height;
							PRINTF("gResizeP.enc_w=%d,gResizeP.enc_h=%d\n", gResizeP.enc_w, gResizeP.enc_h);
							PRINTF("gResizeP.dst_h=%d,gResizeP.dst_h=%d=caption w=%d,h=%d\n", gResizeP.dst_w, gResizeP.dst_h, capture_width, capture_height);

							if(gResizeP.dbstatus == 1) {
								DEBUG(DL_DEBUG, "gResizeP.enc_w=%d,gResizeP.enc_h=%d\n", gResizeP.enc_w, gResizeP.enc_h);
								CalcReaolution(gResizeP.enc_w, gResizeP.enc_h, capture_width, capture_height, 1, resizemode);

							}

							if(lock_status) {
								DEBUG(DL_DEBUG, "gResizeP.enc_w=%d,gResizeP.enc_h=%d\n", gResizeP.enc_w, gResizeP.enc_h);
								CalcReaolution(gResizeP.enc_w, gResizeP.enc_h, capture_width, capture_height, 1, resizemode);

							}

							if(source_size >= LOCK_BEGIN && source_size < LOCK_MAX) {
								PRINTF("gResizeP.enc_w=%d,gResizeP.enc_h=%d\n", gResizeP.enc_w, gResizeP.enc_h);
								calcLockResolution(source_size);
								CalcReaolution(gResizeP.enc_w, gResizeP.enc_h, capture_width, capture_height, 1, resizemode);
								PRINTF("gResizeP.enc_w=%d,gResizeP.enc_h=%d\n", gResizeP.enc_w, gResizeP.enc_h);
								PRINTF("gResizeP.dst_h=%d,gResizeP.dst_h=%d=caption w=%d,h=%d\n", gResizeP.dst_w, gResizeP.dst_h, capture_width, capture_height);
							}

							invaild_flag = 0;
						}

					}

				}

				oldmode = v412_mode->mode;
				gblSetMode(v412_mode->mode);
					DEBUG(DL_DEBUG, "current mode = %d\n", gblGetMode());
				gsignal_ret = ioctl(g_gpio_fd, SIGNAL_STA_MODE, &op);
//				ioctl(g_gpio_fd, GPIO_S_AIC32_REG, 12);
				
				gsignal_hpv = op.HPV;
				gsignal_vsyncF = op.VsyncF;
				gsignal_RGB_YPRPR = op.RGB_YPRPR;
				DEBUG(DL_DEBUG, "ret=%d,op.HDMI_HDCP=%d,op->HPV=%d,****op->RGB_YPRPR=%d****,op->TMDS=%d,op->VsyncF=%d\n",
				      gsignal_ret, op.HDMI_HDCP, op.HPV, op.RGB_YPRPR, op.TMDS, op.VsyncF);
	//			PRINTF("..............HDPV=%d,vsyncF=%d,RGB_YPRPR=%d..........\n",
	//					gsignal_hpv,gsignal_vsyncF,gsignal_RGB_YPRPR);
				
			} else {
				DEBUG(DL_ERROR, "[Capture_getVideoMode] Failed on Error No=%d\n", ret);
			}

#endif
		} else {
			gblCaptureunLock();
			DEBUG(DL_ERROR, "capture handle is NULL!!!\n");
		}

		//	gblCaptureunLock();

		if(v412_mode->adv7401_mode) {
			SendICMPmessage();
		}

		usleep(500000);
	}

cleanup:
	DEBUG(DL_DEBUG, "[detectThrFxn] cleanup XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
	Rendezvous_force(envp->hRendezvousInit);
	/* Meet up with other threads before cleaning up */
	Rendezvous_meet(envp->hRendezvousCleanup);

	if(v412_mode) {
		free(v412_mode);
		v412_mode = NULL;
	}

	// cleanICMP();
	DEBUG(DL_DEBUG, "[detectThrFxn] cleanup XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX end\n");
	return status;
}



//绿屏调整
/*获取当前的模拟还是数字*/
int app_get_digital_flag(int mode)
{
	int digital = -1;
	dmai_v4l2_control ctrl;

	if(mode < 0 || mode >= APP_VIDEO_MAX_COUNT) {
		return -1;
	}

	Capture_Handle hCapture = NULL;
	hCapture = gblGetCapture();

	if(hCapture != NULL) {
		memset(&ctrl, 0, sizeof(ctrl));
		get_digital_status(hCapture, &ctrl);
		digital = ctrl.value;
	}

	return digital;
}

void change_green_screen_adjust(int mode)
{
	if(mode < 0 || mode >= APP_VIDEO_MAX_COUNT) {
		return -1;
	}

	int ret = 0;
	int value = -1;
	int digital = 0;
	digital = app_get_digital_flag(mode);
	dmai_v4l2_control ctrl;
	ret = app_get_green_adjust_value(digital, mode, &value);

	if(ret < 0) {
		return ;
	}

	if(gblGetCapture() != NULL) {
		get_colour_revise(gblGetCapture(), &ctrl);
		DEBUG(DL_DEBUG, "[change_green_screen_adjust] i will set the colour revise ,mode = %d,value = %d,digital =%d\n", mode, value, digital);
		DEBUG(DL_DEBUG, "get the colour revise ,ctrl.value = %d\n", ctrl.value);
		//	if(ctrl.value != value)
		{
			ctrl.value = value;
			set_colour_revise(gblGetCapture(), &ctrl);
		}
	}

	return ;
}

static void change_screen_adjust(int mode)
{
	if(mode < 0 || mode >= APP_VIDEO_MAX_COUNT) {
		return -1;
	}

	int ret = 0;
	int value = -1;
	int digital = 0;
	digital = app_get_digital_flag(mode);
	//if(digital) {
	ret = app_get_screen_adjust_value(digital, mode, &value);
	DEBUG(DL_DEBUG, "[change_screen_adjust] i will set the colour adjust ,mode = %d,value = %d,digital =%d\n", mode, value, digital);

	if(ret < 0) {
		return ;
	}

	SetGPIOColorStatus(g_gpio_fd, value);
	//}
	return ;
}

/*green adjust*/
int GreenScreenAjust(void)
{
	int ret = 0 ;
	dmai_v4l2_control ctrl;
	gblCaptureLock();

	if(gblGetCapture() == NULL) {
		gblCaptureunLock();
		return -1;
	}

	ret = get_colour_revise(gblGetCapture(), &ctrl);
	DEBUG(DL_DEBUG, "before ctrl.value = %d  \n", ctrl.value);

	if(ctrl.value) {
		ctrl.value = 0;
	} else {
		ctrl.value = 1;
	}

	DEBUG(DL_DEBUG, "after ctrl.value = %d\n", ctrl.value);
	ret = set_colour_revise(gblGetCapture(), &ctrl);

	//save file
	int mode =  gblGetMode();
	int digital = app_get_digital_flag(mode);
	gblCaptureunLock();

	app_set_green_adjust_value(digital, mode, ctrl.value);
	DEBUG(DL_DEBUG, "[GreenScreenAjust] i will set the colour revise ,mode = %d,value = %d,digital =%d\n", mode, ctrl.value, digital);
	return ret;
}

static int ScreenAjust(void)
{
	int ret = -1;
	int value = -1;
	int digital = 1;
	int mode =  gblGetMode();

	if(mode < 0 || mode >= APP_VIDEO_MAX_COUNT) {
		return ret;
	}

	digital = app_get_digital_flag(mode);
	ret = app_get_screen_adjust_value(digital, mode, &value);

	if(ret < 0) {
		return ret;
	}

	if(value) {
		SetGPIOColorStatus(g_gpio_fd, 0);
		ret = app_set_screen_adjust_value(digital, mode, 0);
	} else {
		SetGPIOColorStatus(g_gpio_fd, 1);
		ret = app_set_screen_adjust_value(digital, mode, 1);
	}

	DEBUG(DL_DEBUG, "[ScreenAjust]  set the colour adjust ,mode = %d,value = %d,digital =%d\n", mode, value, digital);
	return ret;
}

static int g_i2c_error = 0;
static int capture_set_i2c_error(int flag)
{
	g_i2c_error = flag;
	return 0;
}
static int capture_get_i2c_error()
{
	return g_i2c_error;
}

int getRGB_YPRPR(void)
{
	return gsignal_RGB_YPRPR;
}
