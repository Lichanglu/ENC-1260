
#ifndef __COMMON__H
#define __COMMON__H

#include "middle_control.h"
/*DEBUG_LEVEL*/
//#define ENABLE_DEUBG
/*SDI board*/
#ifdef CL4000_SDI
#define 	BOX_VER					"6.4.6T"
#define 	MAXBITRATE				20000000
#define 	LIMITFRAMES				60
#define	BOARD_TYPE				"CL4000-SDI"
#define	CL4000_DVI_SDI
#endif
#ifdef CL4000_DVI
#define 	BOX_VER					"4.4.6"
#define 	MAXBITRATE				20000000
#define 	LIMITFRAMES				60
#define	BOARD_TYPE				"CL4000-DVI"
#define	CL4000_DVI_SDI
#define	COLOR_STATUS
#endif

#ifdef DSS_ENC_1200
#define	BOX_VER					"1.0.1"
#define 	MAXBITRATE				20000000
#define 	MULTIPLE 				200 * 1000
#define 	LIMITFRAMES				60
#define	BOARD_TYPE				"DSS-ENC-1200"
#define	DSS_ENC_1100_1200
#define  DSS_ENC_1100_1200_DEBUG
//config file
#define CONFIG_NAME				"config.dat"
#define PROTOCOL_NAME				"append.ini"
#define REMOTE_NAME				"remote.ini"
#define HV_TABLE_D_NAME			"digital.ini"
#define HV_TABLE_A_NAME			"analog.ini"
#define IFRAMES_NAME				"interval.ini"
#define DTYPECONFIG_NAME        		"dtypeconfig.ini"
#define INPUTTYPEFILE  				"inputtype.cfg"
#define LOWBIT_PARAM            		"lowbitparam.ini"
#define DHCPCONFIG_FILE				"dhcpfile.ini"
#define VIDEOENCODE_FILE			"videoencode.ini"
#define ADDTEXT_FILE				"text.ini"

#endif

#ifdef DSS_ENC_1100
#define 	BOX_VER					"2.1.4"
#define 	MAXBITRATE				10000000
#define 	MULTIPLE 				100 * 1024
#define 	LIMITFRAMES				30
#define	BOARD_TYPE				"DSS-ENC-1100"
#define	DSS_ENC_1100_1200
#define  DSS_ENC_1100_1200_DEBUG

//config file
#define CONFIG_NAME				"config.dat"
#define PROTOCOL_NAME				"append.ini"
#define REMOTE_NAME				"remote.ini"
#define HV_TABLE_D_NAME			"digital.ini"
#define HV_TABLE_A_NAME			"analog.ini"
#define IFRAMES_NAME				"interval.ini"
#define DTYPECONFIG_NAME        		"dtypeconfig.ini"
#define INPUTTYPEFILE  				"inputtype.cfg"
#define LOWBIT_PARAM            		"lowbitparam.ini"
#define DHCPCONFIG_FILE				"dhcpfile.ini"
#define VIDEOENCODE_FILE			"videoencode.ini"
#define ADDTEXT_FILE				"text.ini"
#endif

#ifdef DSS_ENC_1260
#define	BOX_VER					"1.1.2t"
#define 	MAXBITRATE				20000000
#define 	MULTIPLE 				200 * 1000
#define 	LIMITFRAMES				60
#define	BOARD_TYPE				"DSS-ENC-1260"
#define	DSS_ENC_1100_1200
#define  DSS_ENC_1100_1200_DEBUG

//config file
#define CONFIG_NAME				"config_1260.dat"
#define PROTOCOL_NAME				"append_1260.ini"
#define REMOTE_NAME				"remote_1260.ini"
#define HV_TABLE_D_NAME			"digital_1260.ini"
#define HV_TABLE_A_NAME			"analog_1260.ini"
#define IFRAMES_NAME				"interval_1260.ini"
#define DTYPECONFIG_NAME        		"dtypeconfig_1260.ini"
#define INPUTTYPEFILE  				"inputtype_1260.cfg"
#define LOWBIT_PARAM            		"lowbitparam_1260.ini"
#define DHCPCONFIG_FILE				"dhcpfile_1260.ini"
#define VIDEOENCODE_FILE			"videoencode_1260.ini"
#define ADDTEXT_FILE				"text_1260.ini"

#endif



#ifdef CL4000_DVI_SDI
#define CONFIG_NAME				"sysparam.ini"
#define LOWBIT_PARAM            		"lowbitparam.ini"
#define IP_PARAM					"ipconfig.ini"
#define HV_TABLE_D_NAME			"digital.ini"
#define HV_TABLE_A_NAME			"analog.ini"
#define REMOTE_NAME				"remote.ini"
#define IFRAMES_NAME				"interval.ini"
#endif



#ifdef TEST_MODE
#define CODE_COND "TEST_MODE"
#else
#define CODE_COND "VERSION_MODE"
#endif


#ifdef NEW_ENCODE_LIB_HP
#define CODE_TYPE "HP"
#else
#define CODE_TYPE ""
#endif

#ifdef ENABLE_DEUBG
#define DEUBG ""
#else
#define DEUBG ""
#endif
#define OPEN_WATCHDOG

#define 	PORT_NUM			4
#define 	PORT_ONE			0
#define 	PORT_TWO			1
#define LED_FREQ_TIMES			4
#define STATUS_LED_GPIO			7
#define 	ENCODE_TYPE					6	//ENC-MOD
#define 	H264_HEADER_LEN 			0x40
#define     START          1
#define     STOP           0
/*offset*/
#define MAX_STDNUM_OFFSET        (8)
/* AD9880 idx*/
#define AD9880_INDEX			 (0)
/* ADV7401 idx*/
#define ADV7401_INDEX			 (AD9880_INDEX+MAX_STDNUM_OFFSET)
/* SDI idx*/
#define SDI_INDEX				 (ADV7401_INDEX+MAX_STDNUM_OFFSET)
/* AUTO index*/
#define AUTO_INDEX				 (-1)

/*h264_pixel*/
#define VIDEO_INVALID					0
#define VIDEO_1080_PIXEL				1
#define VIDEO_720_PIXEL				2
#define VIDEO_VGA_PIXEL				3
#define VIDEO_576P_PIXEL				4
#define VIDEO_480P_PIXEL				5

#define VIDEO_NO_SOURCE				8

#define VIDEO_576I_PIXEL				30
#define VIDEO_480I_PIXEL				29
#define VIDEO_1080P60_PIXEL			28
#define VIDEO_1080P50_PIXEL			27
#define VIDEO_1080P30_PIXEL			26
#define VIDEO_1080P25_PIXEL			25
#define VIDEO_1080I30_PIXEL				24
#define VIDEO_1080I25_PIXEL				23
#define VIDEO_720P60_PIXEL				22
#define VIDEO_720P50_PIXEL				21
#define VGA_PIXEL_START				0

/*����*/
#define PORT_COM1		0
#define PORT_COM2		1
#define PORT_COM3		2

/*ICMP ����*/
#define	MAX_DUP_CHK		(8 * 128)
#define	DEFDATALEN		(64 - 8)	/* Ĭ�����ݳ���*/
#define	MAXPACKET		(200)		/* ������С */
#define	A(bit)			rcvd_tbl[(bit)>>3]	/* identify byte in array */
#define	B(bit)			(1 << ((bit) & 0x07))	/* identify bit in byte */
#define	SET(bit)		(A(bit) |= B(bit))
#define	CLR(bit)		(A(bit) &= (~(B(bit))))
#define	TST(bit)		(A(bit) & B(bit))

#define ADTS 					0x53544441
#define AAC						0x43414130
#define PCM						0x4d435030

#define MIC_INPUT				1
#define LINE_INPUT				0

#define MAX_CLIENT				6
#define MAX_PACKET				14600

#define PORT_LISTEN_ONE			3100

#define AVIIF_KEYFRAME				0x00000010



#define WATCH_DOG_DEV		    "/dev/watchdog"

/* The levels of initialization */
#define LOGSINITIALIZED         0x1
#define DISPLAYTHREADCREATED    0x2
#define CAPTURETHREADCREATED    0x4
#define WRITERTHREADCREATED     0x8
#define VIDEOTHREADCREATED      0x10
#define AUDIOTHREADCREATED      0x20
#define DETECTTHREADCREATED		0x40
#define WEBLISTENCREATED		0x80
#define VIDEOLOWRATETHREAD			0x100
#define RESIZELOWRATETHREAD			0x200
#define WRITELOWRATETHREAD			0x400


#define H264_MAX_ENCODE_WIDTH		(1920)
#define H264_MAX_ENCODE_HIGHT		(1080)

#define LOWRATE_MAX_WIDTH			(704)
#define LOWRATE_MAX_HEIGHT			(576)

#define LOWRATE_MIN_WIDTH           (352)
#define LOWRATE_MIN_HEIGHT          (288)

#define INIT_LOWBIT_PARAM	{STOP,STOP,704,576,30};

#define GBL_COMMON_DATA  {NULL,0,-1,-1}

#define INITWIDTH		1920
#define INITHEIGHT		1080

#define 	AppMain		main

#ifdef ENABLE_DEUBG
#include <stdio.h>
typedef enum {
    DL_NONE, //=0
    DL_ERROR,  //=1
    DL_WARNING,  //=2
    DL_FLOW,	//=3
    DL_DEBUG,	//=4
    DL_ALL, 	//all
} EDebugLevle;

#define DEBUG_LEVEL   	(DL_ALL)	// // TODO: modify DEBUG Level

#if 1		//��־�ļ�
#define DEBUG(x,fmt,arg...) \
	do {\
		if ((x)<DEBUG_LEVEL){\
			fprintf(stderr, fmt, ##arg);\
		}\
	}while (0)
#else
#include <syslog.h>
#define DEBUG(x,fmt,arg...) \
	do {\
		if ((x)<DEBUG_LEVEL){ \
			syslog(LOG_INFO, "%s",__func__);\
			syslog(LOG_INFO,fmt, ##arg);\
		}\
	}while (0)
#endif
#else
#define DEBUG(x,fmt,arg...) do {}while(0)
#endif

#define GPIO_0			0
#define GPIO_1			1
#define GPIO_2			2
#define GPIO_3			3
#define GPIO_4			4
#define GPIO_5			5
#define GPIO_6			6
#define GPIO_7			7
#define GPIO_8			8
#define GPIO_9			9

#define GPIO_25			25
#define GPIO_26			26


#define GPIO_INPUT		0
#define GPIO_OUTPUT		1
#define GPIO_LEVEL		0
#define GPIO_HIGH		1

#define KERNELFILENAME		"uImage"
#define FPGAFILENAME		"fpga.bin"
#define GPIO_0			0
#define GPIO_1			1
#define GPIO_2			2
#define GPIO_3			3
#define GPIO_4			4
#define GPIO_5			5
#define GPIO_6			6
#define GPIO_7			7
#define GPIO_8			8
#define GPIO_9			9

#define GPIO_25			25
#define GPIO_26			26


#define GPIO_INPUT		0
#define GPIO_OUTPUT		1
#define GPIO_LEVEL		0
#define GPIO_HIGH		1

#define KERNELFILENAME		"uImage"
#define FPGAFILENAME		"fpga.bin"

#include <ti/sdo/dmai/Capture.h>
#include <pthread.h>

#define RTC_DEV_NAME "/dev/rtc"

#define FPGA_UPDATE				0x55AA
#define FPGA_NO_UPDATE			0xAA55

#define IIS_FPGA_UPDATE()		(FPGA_UPDATE == g_FPGA_update)
#define SET_FPGA_UPDATE(update)		(g_FPGA_update = update)

#define MULT_STOP		(0)
#define MULT_START		(1)

//#define WEB_SET_MULT_START(protocol)		(s_MultAddr[protocol].nStatus = MULT_START)
//#define WEB_SET_MULT_STOP(protocol)		(s_MultAddr[protocol].nStatus = MULT_STOP)

/*****************************************************************/

typedef struct gblCommonData {
	Capture_Handle  	hcapture;
	int  				mode;			//��Ƶģʽ
	int  				encodeIdx;		//�ӿ����� -1 Ϊ�Զ�
	int	 				h264_pixel; 	//����ͼ���С

	pthread_mutex_t     hcapture_mutex;
	pthread_mutex_t     mode_mutex;
	pthread_mutex_t     encodeIdx_mutex;
	pthread_mutex_t     h264_pixel_mutex;
	pthread_mutex_t		encode_mutex;
	pthread_mutex_t		process;
} gblCommonData;

typedef struct __CTime__ {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
} CTime;


/*����ģ����ں���*/
Int ModuleEncode(Int argc, Char *argv[]);
/*��ȡ�����ӿ�*/
extern int gblGetIDX(void);
extern void gblSetIDX(int encodeIdx, int writefile);
/*��ȡ�����ģʽ*/
extern int gblGetMode(void);
/*���������ģʽ*/
extern int gblSetMode(int mode);
/*�����VP�ڴ�������*/
extern void gblEncodeLcok(void);
/*ȡ�������VP�ڴ�������*/
extern void gblEncodeunLcok(void);

/*��ȡ�ɼ��豸���*/
extern Capture_Handle gblGetCapture(void);
/*����H264�ķֱ���*/
extern void gblSetH264pixel(int h264_pixel);
/*��ȡ����ķֱ���*/
extern int gblGetH264pixel(void);
/*���òɼ��豸���*/
extern void gblSetCapture(Capture_Handle hcapture);
/*��*/
extern void gblCaptureLock(void);
/*����*/
extern void gblCaptureunLock(void);
/*��*/
extern void gblprocessLock(void);
/*����*/
extern void gblprocessunLock(void);

/*��ʼ���������*/
extern void InitgblCommonMutex(void);
/*����ͨ�û������*/
extern void DestorygblCommonMutex(void);
/*����RTC��ʱ��*/
extern void setRtcTime(int nYear, int nMonth, int nDay, int nHour, int nMinute, int nSecond);
/*�õ�ϵͳʱ��*/
extern void getSysTime(DATE_TIME_INFO *dtinfo);

/*���������˿ڿ�ʼ*/
extern int gblSetPortStart(void);

/*��������˿�*/
extern int gblGetPortStart(void);
/*���OSD��ʼ״̬*/
extern int gblGetOSDStatus(void);
/*���ÿ���OSD*/
extern int gblSetStartOSD(void);
/*���ùر�OSD*/
extern int gblSetStopOSD(void);

/*��ȡԶң��Э��index*/
extern int gblGetRemoteIndex(void);
/*��ȡԶң��Э��index*/
extern int gblSetRemoteIndex(int index);
extern int set_gpio_bit(int bit);
extern int clear_gpio_bit(int bit);
extern int get_gpio_bit(int bit);
extern int gblSetMute(int status);
extern int gblGetMuteStatus(void);
/*get time */
extern unsigned long long getCurrentTime(void);
/*get system run time*/
unsigned int get_run_time(void);
/*ι��*/
extern int writeWatchDog(void);
/*��ʼ�����Ź�*/
extern int initWatchDog(void);
/*�رտ��Ź�*/
extern int closeWatchDog(void);

/***************************LCD**************************************/

/*open lcd module communication ttyS*/
extern void OpenLCDCom();
/*Thread LCD*/
extern void LCDModuleTask(void *param);
/*********************************************************************/

extern int WriteData(int s, void *pBuf, int nSize);

#endif //__COMMON__H


