#ifndef __SYSPARAM__H
#define __SYSPARAM__H

#include "common.h"

#include "middle_control.h"

#define 	H264_ENC_PARAM_FLAG			0x55AA55AA
#define     MAX_FAR_CTRL_NUM		    6
#define MODE_MAX                33 //video max mode 
#define     MAX_FAR_CTRL_NUM		    6

#define 	LOW_VIDEO							0
#define 	HIGH_VIDEO						1
#define 	MAX_VAL						2

#define     MAX_MODE_VALUE			    50
#define     RUN_LED_GPIO				1
typedef int 			SOCKET;
typedef int 			DSPFD;
typedef unsigned short 	WORD ;
typedef unsigned int   	DWORD ;
typedef unsigned char  	BYTE;


/*视频/音频/系统参数表*/
typedef struct __SAVEPARAM {
	SYSPARAMS sysPara;                 				//standard param table
	VideoParam videoPara[PORT_NUM];				 	//video param table
	AudioParam audioPara[PORT_NUM];					//audio param table
} SavePTable;

typedef unsigned int UINT32;
typedef unsigned short UINT16;
typedef unsigned char UINT8;

/*设置视频编码库表*/
typedef struct __VENC_PARAMS_ {
	UINT32 uflag;  		/*video encode params available flag
						   flag:0x55AA55AA for param availability
						   flag:0x0  for param not availability
						   低码流时，flag: 0x55555555 宽高发生变化
						             flag: 0xAAAAAAAA 码率发生变化
						 */
	UINT32 req_I;       /*强制I frames*/
	UINT32 framerate;	/*control frame rate*/
	UINT32 quality;		/*quality*/
	UINT32 width;		/*encoder lib width*/
	UINT32 height;		/*encoder lib height*/
	UINT16 bitrate;		/*bit rate*/
	UINT16 cbr;	  		/*1:bitrate / 0:quality*/
} VENC_params;

/*设置音频参数编码库表*/
typedef struct __AENC_PARAMS__ {
	UINT32 uflag;  		/* audio encode params available flag
						   flag:0x55AA55AA for param availability
						   flag:0x0  for param not availability
						 */
	UINT32 samplerate;		/*audio encode sample rate*/
	UINT32 bitrate;			/*audio encode bit rate*/
	UINT32 channel;			/*audio channels*/
	UINT32 samplebit;		/*sample bit*/
	UINT8  lvolume;   		/*left volume*/
	UINT8  rvolume;			/*right volume*/
	UINT16 inputmode;		/*0: mic input 1:line input*/
} AENC_params;

/*设置编码库参数表*/
typedef struct __VAENC_PARAM__ {
	VENC_params venc;
	VENC_params lowvenc;
	AENC_params aenc;
} VAENC_params, *pVAENC_Params;

/*Resize Param table*/
typedef struct __RESIZE_PARAM__ {
	unsigned int dbstatus;
	unsigned int flag ;
	unsigned int re_enc ; // receate encode lbi
	int black;
	int enc_w;			//source width
	int enc_h;			//source height
	int dst_w;			//dest  width
	int dst_h;			//dest height
	int x;
	int y;

	int nFrame;		//帧率
	int nBitrate;   //带宽
	int nTemp;		//保留
} ResizeParam;

/*低码率参数*/
typedef struct strLOW_BITRATE {
	int nWidth;		//宽
	int nHeight;		//高
	int nFrame;		//帧率
	int nBitrate;   //带宽
	int nTemp;		//保留
} LOW_BITRATE;

/*偏移量*/
typedef struct __HV__ {
	short	hporch;
	short	vporch;
} HV;
typedef struct __HV_Table__ {
	HV	analog[MAX_MODE_VALUE];
	HV	digital[MAX_MODE_VALUE];
} HVTable;

#include <pthread.h>

typedef pthread_mutex_t Pthread_t;
#define Lock(m_mutex) pthread_mutex_lock(&m_mutex);
#define unLock(m_mutex) pthread_mutex_unlock(&m_mutex);
#define Pthread_init(m_mutex)	pthread_mutex_init(&m_mutex,NULL);


/*绿屏校正保存*/
typedef struct __GREENSCREEN__ {
	Pthread_t m_protect;
	short green[MAX_MODE_VALUE];		//绿屏
} GreenScreen;

/*绿屏校正表*/
typedef struct __GREENTABLE__ {
	GreenScreen tbl;
	int (*CopyTable)(GreenScreen *Old, GreenScreen *New);
	int (*initTable)(GreenScreen *gs);
	int (*readTable)(char *filename, GreenScreen *gs);
	int (*saveTable)(char *filename, GreenScreen *Old, GreenScreen *New);
} GreenTable;
/*线程互斥定义*/
typedef struct __PthreadLock__ {
	pthread_mutex_t net_send_m;
	pthread_mutex_t sys_m;
	pthread_mutex_t save_flush_m;
	pthread_mutex_t send_m;
	pthread_mutex_t send_audio_m;
	pthread_mutex_t audio_video_m;
	pthread_mutex_t save_sys_m;
#ifdef CL4000_DVI_SDI
	pthread_mutex_t rtp_send_m;
#else
	pthread_mutex_t save_dtype_m;
#endif

} PLock;

/*线程互斥变量初始化*/
extern int initMutexPthread(void);
/*线程互斥变量初始化*/
extern int DestroyMutexPthread(void);
/*初始化参数表结构体*/
extern void InitSysParams();
/*初始化视频编码参数*/
extern int InitVideoEncParams(VideoParam *vparam);
/*初始化低码率视频编码参数*/
extern int InitLowRateParams(VideoParam *vparam);
/*初始化音频编码参数*/
extern int InitAudioEncParams(AudioParam *aparam);
/*设置编码的高*/
extern int SetH264Width(int width);
/*设置编码的高*/
extern int SetH264Height(int height);
/*获取编码宽*/
extern unsigned int H264Width(void);
/*获取编码高*/
extern unsigned int H264Height(void);
/*从配置文件中读取参数*/
extern int ReadEncodeParamTable(char *config_file, SavePTable *tblParam);
/*从配置文件中读取低码率参数*/
extern int ReadLowbitParamTable(char *config_file, SavePTable *tblParam);
/*从配置文件中读取IP信息*/
extern int ReadIPParamTable(char *config_file, SavePTable *tblParam);
/*保存参数到文件中*/
extern int SaveParamsToFlash();
/*保存低码率参数到文件中*/
extern int SaveLowRateParamsToFlash();
/**********************************************************************
* Function Name:  ConfigGetKeys
* Function scriptor:  Get All Key
* Access table:   NULL
* Modify Table:	  NULL
* Input param：   void *CFG_file File    void *section
* Output Param:    char *keys[]   : save key name
* Return:  		  key count   error: < 0
***********************************************************************/
extern int  ConfigGetKeys(void *CFG_file, void *section, char *keys[]);
/**********************************************************************
* Function Name:  ConfigSetKey
* Function scriptor:  Set Key Value
* Access table:   NULL
* Modify Table:	  NULL
* Input param：   void *CFG_file File   void *section   void *key
					void *buf
* Output Param:   void *buf  key Value
* Return:  		  0 -- OK  no 0-- FAIL
***********************************************************************/
extern int  ConfigSetKey(void *CFG_file, void *section, void *key, void *buf);
/*
## 将MAC地址的字符串转换成字符数组中
##
## 如:  src = "00:11:22:33:44:55" --->
##		dst[0] = 0x00 dst[1] = 0x11 dst[2] = 0x22;
##
*/
extern int SplitMacAddr(char *src, unsigned char *dst, int num);

/*打开 GPIO 端口*/
extern int open_gpio_port();

/*get audio param*/
extern void GetAudioParam(int socket, int dsp, unsigned char *data, int len);
/*get video param*/
extern void GetVideoParam(int socket, int dsp, unsigned char *data, int len);
/*set video param table*/
extern int SetVideoParam(int dsp, unsigned char *data, int len);
/*set audio param table*/
extern int SetAudioParam(int dsp, unsigned char *data, int len);
/*SetSysParams*/
extern int SetSysParams(SYSPARAMS *pold, SYSPARAMS *pnew);
/*获得文本长度*/
extern int GetTextLen(char *Data);
extern int GetSizeLen(char *Data);
/*Force I frame */
extern int ForceIframe(int start);
/*update program*/
extern int DoUpdateProgram(int sSocket, unsigned char *data, int len);
//*********************************************************************
//设置和获取静音参数
//*********************************************************************
extern int SetMuteParams(unsigned char *data, int len);
/*Print current avaliable users*/
extern void PrintClientNum();
//*********************************************************************
//读取边框值，初始化以及调节边框
//*********************************************************************
/*read Hportch Vportch Table*/
extern int ReadHVTable(const HVTable *pp, int digital);
extern int SaveHVTable(HVTable *pOld, HVTable *pNew, int digital);
/*compare beyond HV table*/
extern int BeyondHVTable(int mode, int digital, short hporch, short vporch, HVTable *pp);

/*initial hporch and vporch value*/
extern  int InitHVTable(HVTable *pp);

/*save remote control index*/
extern int SaveRemoteCtrlIndex(int sock, int index, int length);
extern int SaveLocalRemoteCtrlIndex(void);
/*调节图像的边框*/
extern int RevisePictureFrame(unsigned char *data, int len);

/*Read Remote Protocol*/
extern int ReadRemoteCtrlIndex(char *config_file, int *index);

extern int  ConfigGetKey(void *CFG_file, void *section, void *key, void *buf);
/*get control frame rate value*/
extern unsigned int GetCtrlFrame();
/*get control frame low rate value*/
extern unsigned int GetCtrlLowRateFrame();
/*读 I 帧间隔*/
extern int ReadIframeInterval(char *config_file);


#endif //__SYSPARAM__H
