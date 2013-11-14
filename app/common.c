#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <linux/watchdog.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include "common.h"
#include "sysparam.h"
#include "kernel_common.h"
#include "mid_timer.h"


#define MAXINTERFACES   16

static gblCommonData gblData 		= GBL_COMMON_DATA;
static unsigned int s_connect_start = 0;
static unsigned int s_osd_start 	= 1;
static unsigned int s_mute_status   = 0;
static int watchdogFD = 0;
static int 	DHCPflag = 0;
static int g_far_ctrl_index = 0;
extern PLock	gSetP_m;


/*初始化互斥变量*/
void InitgblCommonMutex(void)
{
	pthread_mutex_init(&gblData.hcapture_mutex, NULL);
	pthread_mutex_init(&gblData.mode_mutex, NULL);
	pthread_mutex_init(&gblData.encodeIdx_mutex, NULL);
	pthread_mutex_init(&gblData.h264_pixel_mutex, NULL);
	pthread_mutex_init(&gblData.encode_mutex, NULL);
	pthread_mutex_init(&gblData.process, NULL);

}

/*销毁通用互斥变量*/
void DestorygblCommonMutex(void)
{
	pthread_mutex_destroy(&gblData.hcapture_mutex);
	pthread_mutex_destroy(&gblData.mode_mutex);
	pthread_mutex_destroy(&gblData.encodeIdx_mutex);
	pthread_mutex_destroy(&gblData.h264_pixel_mutex);
	pthread_mutex_destroy(&gblData.encode_mutex);
	pthread_mutex_destroy(&gblData.process);
}

/*编码和VP口创建互斥*/
void gblEncodeLcok(void)
{
	pthread_mutex_lock(&gblData.encode_mutex);
}

/*取消编码和VP口创建互斥*/
void gblEncodeunLcok(void)
{
	pthread_mutex_unlock(&gblData.encode_mutex);
}

/*获取采集设备句柄*/
Capture_Handle gblGetCapture(void)
{
	Capture_Handle hcapture;
	hcapture = gblData.hcapture;
	return hcapture;
}

/*设置采集设备句柄*/
void gblSetCapture(Capture_Handle hcapture)
{
	gblData.hcapture = hcapture;
}

/*锁*/
void gblCaptureLock(void)
{
	pthread_mutex_lock(&gblData.hcapture_mutex);
}

/*解锁*/
void gblCaptureunLock(void)
{
	pthread_mutex_unlock(&gblData.hcapture_mutex);
}
/*锁*/
void gblprocessLock(void)
{
	pthread_mutex_lock(&gblData.process);
}

/*解锁*/
void gblprocessunLock(void)
{
	pthread_mutex_unlock(&gblData.process);
}
/*获取输入的模式*/
int gblGetMode(void)
{
	int	 mode;
	pthread_mutex_lock(&gblData.mode_mutex);
	mode = gblData.mode;
	pthread_mutex_unlock(&gblData.mode_mutex);
	return mode;
}

/*设置输入的模式*/
int gblSetMode(int mode)
{
	pthread_mutex_lock(&gblData.mode_mutex);
	gblData.mode = mode;
	pthread_mutex_unlock(&gblData.mode_mutex);
	return mode;
}

void gblSetIDX(int encodeIdx, int writefile)
{
	int ret = 0 ;
	char buf[10];

	pthread_mutex_lock(&gblData.encodeIdx_mutex);

#ifdef DSS_ENC_1100
	encodeIdx = AD9880_INDEX;
#endif

#ifdef CL4000_SDI
	encodeIdx = SDI_INDEX;
#elif  CL4000_DVI
	encodeIdx = AD9880_INDEX;
#endif



#if (defined(DSS_ENC_1200) || defined(DSS_ENC_1260))

	/*write the cfg 文件*/ //modify by zm ,just change need save file

	if(writefile && (gblData.encodeIdx != encodeIdx)) {
		if(encodeIdx == AD9880_INDEX) {
			strcpy(buf, "DVI");

		} else if(encodeIdx == SDI_INDEX) {
			strcpy(buf, "SDI");

		} else {
			strcpy(buf, "AUTO");
		}

		DEBUG(DL_DEBUG, "write input mode interface [%s]\n", buf);
		ret = ConfigSetKey(INPUTTYPEFILE, "InputMode", "interface", buf);

		if(ret < 0) {
			DEBUG(DL_ERROR, "write input mode error interface = %s\n", buf);
			pthread_mutex_unlock(&gblData.encodeIdx_mutex);
			return ;
		}
	}

#endif

	gblData.encodeIdx = encodeIdx;
	pthread_mutex_unlock(&gblData.encodeIdx_mutex);
}


/*获取索引接口*/
int gblGetIDX(void)
{
	int	 encodeIdx;
	pthread_mutex_lock(&gblData.encodeIdx_mutex);
	encodeIdx = gblData.encodeIdx;
	pthread_mutex_unlock(&gblData.encodeIdx_mutex);
	return encodeIdx;
}

void gblLoadIDX(void)
{
	char buf[10];
	int ret = 0, encodeIdx = AUTO_INDEX;
#if (defined(DSS_ENC_1200) || defined(DSS_ENC_1260))
	ret = ConfigGetKey(INPUTTYPEFILE, "InputMode", "interface", buf);

	if(ret < 0) {
		DEBUG(DL_ERROR, "%s failed !!\n", __FILE__);
	}

	if(strcmp(buf, "SDI") == 0) {
		encodeIdx = SDI_INDEX;
	} else if(strcmp(buf, "DVI") == 0) {
		encodeIdx = AD9880_INDEX;
	} else {
		encodeIdx = AUTO_INDEX;
	}

#endif
	pthread_mutex_lock(&gblData.encodeIdx_mutex);

#ifdef CL4000_SDI
	encodeIdx = SDI_INDEX;
#elif  CL4000_DVI
	encodeIdx = AD9880_INDEX;
#elif DSS_ENC_1100
	encodeIdx = AD9880_INDEX;
#else
	gblData.encodeIdx = encodeIdx;
#endif

	pthread_mutex_unlock(&gblData.encodeIdx_mutex);
	return ;
}


/*设置编码的分辨率*/
void gblSetH264pixel(int h264_pixel)
{
	pthread_mutex_lock(&gblData.h264_pixel_mutex);
	gblData.h264_pixel = h264_pixel;
	pthread_mutex_unlock(&gblData.h264_pixel_mutex);
}


/*获取编码的分辨率*/
int gblGetH264pixel(void)
{
	int h264_pixel;
	pthread_mutex_lock(&gblData.h264_pixel_mutex);
	h264_pixel = gblData.h264_pixel;
	pthread_mutex_unlock(&gblData.h264_pixel_mutex);
	return (h264_pixel);
}

/*设置RTC的时间*/
void setRtcTime(int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond)
{
	if(nSecond < 1) {
		nSecond  = 10;
	}

	int fd = 0;
	struct rtc_time rtc_tm;

	if(fd == 0)		{
		//DEBUG(DL_ERROR,"YYH:Enter into setRtcTime open \n");
		fd = open(RTC_DEV_NAME, O_RDWR);
		//DEBUG(DL_ERROR,"YYH:Leave out setRtcTime open \n");
	}

	if(fd == -1) 	{
		DEBUG(DL_ERROR, "Cannot open RTC device due to following reason.\n");
		perror("/dev/rtc");
		fd = 0;
		return;
	}

	rtc_tm.tm_mday = nDay;
	rtc_tm.tm_mon = nMonth - 1;
	rtc_tm.tm_year = nYear - 1900;
	rtc_tm.tm_hour = nHour;
	rtc_tm.tm_min = nMinute;
	rtc_tm.tm_sec = nSecond;
	//更新操作系统时间
	//普通服务器上可以使用   date   -s   '2005-12-12   22:22:22'
	//arm用date   -s   121222222005.22   MMDDhhmmYYYY.ss
	char setStr[256] = {0};
//	sprintf(setStr, "date -s %04d%02d%02d", nYear, nMonth, nDay);
//	system(setStr);
//	sprintf(setStr, "date -s %02d:%02d:%02d", nHour, nMinute, nSecond);
//	system(setStr);

	sprintf(setStr, "date -s \"%02d:%02d:%02d %04d%02d%02d\"", nHour, nMinute, nSecond, nYear, nMonth, nDay);
	system(setStr);
//	sprintf(setStr, "date -s ", nHour, nMinute, nSecond);
//	system(setStr);
	
	
	//取得星期几
	time_t timep;
	struct tm *p;
	time(&timep);
	p = localtime(&timep);
	rtc_tm.tm_wday = p->tm_wday;
	int retval = ioctl(fd, RTC_SET_TIME, &rtc_tm);

	if(retval == -1) 	{
		fprintf(stderr, "Cannot do ioctl() due to following reason.\n");
		perror("ioctl");
		close(fd);
		return;
	}

	close(fd);

}

/*得到系统时间*/
void getSysTime(DATE_TIME_INFO *dtinfo)
{
	long ts;
	struct tm *ptm = NULL;

	ts = time(NULL);
	ptm = localtime((const time_t *)&ts);
	dtinfo->year = ptm->tm_year + 1900;
	dtinfo->month = ptm->tm_mon + 1;
	dtinfo->mday = ptm->tm_mday;
	dtinfo->hours = ptm->tm_hour;
	dtinfo->min = ptm->tm_min;
	dtinfo->sec = ptm->tm_sec;

}

/*设置侦听端口开始*/
int gblSetPortStart(void)
{
	s_connect_start = 1;
	return (s_connect_start);
}
/*设置侦听端口关闭*/
int gblSetPortStop(void)
{
	s_connect_start = 0;
	return (s_connect_start);
}

/*获得侦听端口*/
int gblGetPortStart(void)
{
	return (s_connect_start);
}

/*设置开启OSD*/
int gblSetStartOSD(void)
{
	s_osd_start = 1;
	return (s_osd_start);
}


/*设置关闭OSD*/
int gblSetStopOSD(void)
{
	s_osd_start = 0;
	return (s_osd_start);
}


/*获得OSD开始状态*/
int gblGetOSDStatus(void)
{
	return (s_osd_start);
}


/*设置静音*/
int gblSetMute(int status)
{
	s_mute_status = status;
	return (s_mute_status);
}


/*获得静音状态状态*/
int gblGetMuteStatus(void)
{
	return (s_mute_status);
}

/*获取远遥的协议index*/
int gblGetRemoteIndex(void)
{
	return g_far_ctrl_index;
}

/*获取远遥的协议index*/
int gblSetRemoteIndex(int index)
{
	g_far_ctrl_index = index;
	return 0;
}

int app_test_dog()
{
	PRINTF("the watch dog is not write for last 4 second\n");
	return 0;
}
/*喂狗*/
int writeWatchDog(void)
{
//#ifndef DSS_ENC_1100_1200_DEBUG
	int dummy;
	//mid_timer_create(2, 1, (mid_func_t)app_test_dog);

	//PRINTF("\n");
	if(ioctl(watchdogFD, WDIOC_KEEPALIVE, &dummy) != 0) {
		DEBUG(DL_ERROR, "writeWatchDog failed strerror(errno) = %s\n", strerror(errno));
		return -1;
	}

//#endif
	return 0;
}

/*初始化看门狗*/
int initWatchDog(void)
{

//#ifndef DSS_ENC_1100_1200_DEBUG
	watchdogFD = open(WATCH_DOG_DEV, O_WRONLY);

	if(watchdogFD == -1) {
		DEBUG(DL_ERROR, "open watch dog failed strerror(errno) = %s\n", strerror(errno));
		return -1;
	}

//#endif
	return 0;
}

/*关闭看门狗*/
int closeWatchDog(void)
{
//#ifndef DSS_ENC_1100_1200_DEBUG

	if(watchdogFD) {
		close(watchdogFD);
		watchdogFD = 0;
	}

//#endif
	return 0;
}

#if 1
/*get time */
//modify by zhangmin  ,getcurrenttime is 64 bit must
unsigned long long getCurrentTime(void)
{
	struct timeval tv;
	struct timezone tz;
	unsigned long long ultime;

	gettimeofday(&tv , &tz);
	ultime = (unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;
	//ultime = 0xfffffffffffffff;
//	printf("ultime=%lld==%u\n",ultime,tv.tv_sec);
	return ultime;
}
#else
unsigned long getCurrentTime(void)
{
	struct timeval tv;
	struct timezone tz;
	unsigned long ultime;

	gettimeofday(&tv , &tz);
	ultime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	return (ultime);
}
#endif

//获取毫秒级别
unsigned int get_run_time(void)
{
	unsigned int msec;
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	msec = tp.tv_sec;
	msec = msec * 1000 + tp.tv_nsec / 1000000;
	//msec +=0x5520000;
	//msec +=0x2D82D82 -1000*120;
	//printf("mid_clock ultime=%x==%x\n",msec,tp.tv_sec);
	return msec;
}

void getInputMode(char *buf)
{
	int mode = gblGetMode();

	switch(mode) {
		case APP_VGA_640X480X60:
			strcpy(buf, "640x480@60");
			break;

		case APP_VGA_640X480X72:
			strcpy(buf, "640x480@72");
			break;

		case APP_VGA_640X480X75:
			strcpy(buf, "640x480@75");
			break;

		case APP_VGA_640X480X85:
			strcpy(buf, "640x480@85");
			break;

		case APP_VGA_800X600X60:
			strcpy(buf, "800x600@60");
			break;

		case APP_VGA_800X600X72:
			strcpy(buf, "800x600@72");
			break;

		case APP_VGA_800X600X75:
			strcpy(buf, "800x600@75");
			break;

		case APP_VGA_800X600X85:
			strcpy(buf, "800x600@85");
			break;

		case APP_VGA_1024X768X60:
			strcpy(buf, "1024x768@60");
			break;

		case APP_VGA_1024X768X72:
			strcpy(buf, "1024x768@72");
			break;

		case APP_VGA_1024X768X75:
			strcpy(buf, "1024x768@75");
			break;

		case APP_VGA_1024X768X85:
			strcpy(buf, "1024x768@85");
			break;

		case APP_VGA_1280X768X60:
			strcpy(buf, "1280x768@60");
			break;

		case APP_VGA_1280X800X60:
			strcpy(buf, "1280x800@60");
			break;

		case APP_VGA_1280X960X60:
			strcpy(buf, "1280x960@60");
			break;

		case APP_VGA_1440X900X60:
			strcpy(buf, "1440x900@60");
			break;

		case APP_VGA_1400X1050X60:
			strcpy(buf, "1400x1050@60");
			break;

		case APP_VGA_1280X1024X60:
			strcpy(buf, "1280x1024@60");
			break;

		case APP_VGA_1280X1024X75:
			strcpy(buf, "1280x1024@75");
			break;

		case APP_VGA_1600X1200X60:
			strcpy(buf, "1600x1200@60");
			break;

		case APP_VGA_1280X720VX60:
			strcpy(buf, "1280x720V@60");
			break;

		case APP_VIDEO_1280X720X50P:
			strcpy(buf, "1280x720P@50");
			break;

		case APP_VIDEO_1280X720X60P:
			strcpy(buf, "1280x720P@60");
			break;

		case APP_VIDEO_1920X1080X50I:
			strcpy(buf, "1920x1080I@50");
			break;

		case APP_VIDEO_1920X1080X60I:
			strcpy(buf, "1920x1080I@60");
			break;

		case APP_VIDEO_1920X1080X25P:
			strcpy(buf, "1920x1080P@25");
			break;

		case APP_VIDEO_1920X1080X30P:
			strcpy(buf, "1920x1080P@30");
			break;

		case APP_VIDEO_1920X1080X50P:
			strcpy(buf, "1920x1080P@50");
			break;

		case APP_VIDEO_1920X1080X60P:
			strcpy(buf, "1920x1080P@60");
			break;

		case APP_VIDEO_720X480X60I:
			strcpy(buf, "720x480I@60");
			break;

		case APP_VIDEO_720X576X50I:
			strcpy(buf, "720x576I@50");
			break;

		case APP_VIDEO_720X480X60P:
			strcpy(buf, "720x480P@60");
			break;

		case APP_VIDEO_720X576X50P:
			strcpy(buf, "720x576P@50");
			break;

		case APP_VGA_1366X768X60:
			strcpy(buf, "1366x768@60");
			break;

		default:
			strcpy(buf, "invaild ");
			break;
	}
}


int saveDHCPTofile(void)
{
	char temp[512];
	char filename[512];
	int ret ;

	strcpy(filename, DHCPCONFIG_FILE);
	pthread_mutex_lock(&gSetP_m.save_sys_m);
	/*index*/
	sprintf(temp, "%d", DHCPflag);
	ret =  ConfigSetKey(filename, "dhcp", "value", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "set dhcp value failed\n");
		goto EXIT;
	}

EXIT:
	pthread_mutex_unlock(&gSetP_m.save_sys_m);
	return 0;

}

#if 0
int readDHCPValue(char *config_file, int *retflag)
{
	char 			temp[16];
	int 			ret  = -1 ;
	//	unsigned int 	value;

	DEBUG(DL_DEBUG, "read config = %s \n", config_file);
	ret =  ConfigGetKey(config_file, "dhcp", "value", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get dhcp flag failed\n");
		*retflag = 0;
		return 0;
	}

	*retflag = atoi(temp);

	return 0;
}


void setDHCPFlag(int data)
{
	DHCPflag = data;
}
int webgetDHCPFlag(int data, int *out)
{

	*out =	DHCPflag;
	return 0;
}

int websetDHCPFlag(int data, int *out)
{
	if(data < 0 || data > 1) {
		DHCPflag = 0;
		*out = DHCPflag;
		saveDHCPTofile();
		return -1;
	}

	DHCPflag = data;
	*out = DHCPflag;
	saveDHCPTofile();
#if 1

	if(data == 1) {
		sleep(3);
		system("reboot -f");
	}

#endif

	return 0;
}
#endif

/*
##
## send date to socket runtime
##
##
*/
int WriteData(int s, void *pBuf, int nSize)
{
	int nWriteLen = 0;
	int nRet = 0;
	int nCount = 0;

	SendPthreadLock();

	while(nWriteLen < nSize) {

		nRet = send(s, (char *)pBuf + nWriteLen, nSize - nWriteLen, 0);

		if(nRet < 0) {
			if((errno == ENOBUFS) && (nCount < 10)) {
				DEBUG(DL_ERROR, "network buffer have been full!\n");
				usleep(10000);
				nCount++;
				continue;
			}

			SendPthreadunLock();
			return nRet;
		} else if(nRet == 0) {
			DEBUG(DL_ERROR, "Send Net Data Error nRet= %d\n", nRet);
			continue;
		}

		nWriteLen += nRet;
		nCount = 0;
	}

	SendPthreadunLock();
	return nWriteLen;
}


